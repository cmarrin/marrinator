/*-------------------------------------------------------------------------
This source file is a part of m8rscript

For the latest info, see http://www.marrin.org/

Copyright (c) 2016, Chris Marrin
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

    - Redistributions of source code must retain the above copyright notice, 
      this list of conditions and the following disclaimer.
      
    - Redistributions in binary form must reproduce the above copyright 
      notice, this list of conditions and the following disclaimer in the 
      documentation and/or other materials provided with the distribution.
      
    - Neither the name of the <ORGANIZATION> nor the names of its 
      contributors may be used to endorse or promote products derived from 
      this software without specific prior written permission.
      
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
-------------------------------------------------------------------------*/

#include "Parser.h"

#include "ParseEngine.h"
#include "ExecutionUnit.h"
#include "SystemInterface.h"
#include <limits>

using namespace m8r;

uint32_t Parser::_nextLabelId = 1;

Parser::Parser(SystemInterface* system)
    : _scanner(this)
    , _program(new Program(system))
    , _system(system)
{
    _currentFunction = _program;
}

void Parser::parse(m8r::Stream* istream)
{
    _scanner.setStream(istream);
    ParseEngine p(this);
    while(1) {
        if (!p.statement()) {
            break;
        }
    }
    programEnd();
}

void Parser::printError(const char* s)
{
    ++_nerrors;
    if (_system) {
        _system->printf(ROMSTR("Error: %s on line %d\n"), s, _scanner.lineno());
    }
}

Label Parser::label()
{
    Label label;
    label.label = static_cast<int32_t>(_currentFunction->code()->size());
    label.uniqueID = _nextLabelId++;
    return label;
}

void Parser::addMatchedJump(Op op, Label& label)
{
    assert(op == Op::JMP || op == Op::JF || op == Op::JF);
    label.matchedAddr = static_cast<int32_t>(_currentFunction->code()->size());
    emitCodeRRR(op);
}

void Parser::matchJump(Label& label)
{
    int32_t jumpAddr = static_cast<int32_t>(_currentFunction->code()->size()) - label.matchedAddr - 1;
    if (jumpAddr < -32767 || jumpAddr > 32767) {
        printError("JUMP ADDRESS TOO BIG TO EXIT LOOP. CODE WILL NOT WORK!\n");
        return;
    }
    _currentFunction->setCodeAtIndex(label.matchedAddr + 1, ExecutionUnit::byteFromInt(jumpAddr, 1));
    _currentFunction->setCodeAtIndex(label.matchedAddr + 2, ExecutionUnit::byteFromInt(jumpAddr, 0));
}

void Parser::jumpToLabel(Op op, Label& label)
{
    assert(op == Op::JMP || op == Op::JF || op == Op::JT);
    int32_t jumpAddr = label.label - static_cast<int32_t>(_currentFunction->code()->size()) - 1;
    emitCodeRSN(op, (op == Op::JMP) ? 0 : _currentFunction->currentReg(), jumpAddr);
    _currentFunction->popReg();
}

void Parser::emitCodeRRR(Op op, uint32_t ra, uint32_t rb, uint32_t rc)
{
    addCode((static_cast<uint32_t>(op) << 26) | ((ra & 0xff) << 18) | ((rb & 0x1ff) << 9) | (rc & 0x1ff));
}

void Parser::emitCodeRUN(Op op, uint32_t ra, uint32_t n)
{
    addCode((static_cast<uint32_t>(op) << 26) | ((ra & 0xff) << 18) | (n & 0x3ffff));
}

void Parser::emitCodeRSN(Op op, uint32_t ra, int32_t n)
{
    addCode((static_cast<uint32_t>(op) << 26) | ((ra & 0xff) << 18) | (n & 0x3ffff));
}

void Parser::addCode(uint32_t c)
{
    if (_deferred) {
        assert(_deferredCodeBlocks.size() > 0);
        _deferredCode.push_back(c);
    } else {
        _currentFunction->addCode(c);
    }
}

void Parser::emit(StringLiteral::Raw s)
{
    ConstantId id = _currentFunction->addConstant(StringLiteral(s));
    emitCodeRRR(Op::MOVE, _currentFunction->pushReg(), id.raw() + MaxRegister);
}

void Parser::emit(uint32_t value)
{
    ConstantId id = _currentFunction->addConstant(value);
    emitCodeRRR(Op::MOVE, _currentFunction->pushReg(), id.raw() + MaxRegister);
}

void Parser::emit(Float value)
{
    ConstantId id = _currentFunction->addConstant(value);
    emitCodeRRR(Op::MOVE, _currentFunction->pushReg(), id.raw() + MaxRegister);
}

void Parser::emitId(const Atom& atom, IdType type)
{
    if (type == IdType::MightBeLocal || type == IdType::MustBeLocal) {
        int32_t index = _currentFunction->localIndex(atom);
        if (index < 0 && type == IdType::MustBeLocal) {
            String s = "nonexistent variable '";
            s += _program->stringFromAtom(atom);
            s += "'";
            printError(s.c_str());
        }
        if (index >= 0) {
            emitCodeRRR(Op::LOADL, _currentFunction->pushReg(), index);
            return;
        }
    }
    
    ConstantId id = _currentFunction->addConstant(atom);
    emitCodeRRR((type == IdType::NotLocal) ? Op::MOVE : Op::LOADREFK, _currentFunction->pushReg(), id.raw() + MaxRegister);
}

void Parser::emit(Op value)
{
    emitCodeRRR(value);
}

void Parser::emit(ObjectId function)
{
    ConstantId id = _currentFunction->addConstant(function);
    emitCodeRRR(Op::MOVE, _currentFunction->pushReg(), id.raw() + MaxRegister);
}

void Parser::emitMove()
{
    uint32_t src = _currentFunction->popReg();
    uint32_t dst = _currentFunction->popReg();
    emitCodeRRR(Op::MOVE, dst, src);
}

void Parser::emitBinOp(Op op, bool sto)
{
    uint32_t src = _currentFunction->popReg();
    uint32_t dst = _currentFunction->popReg();
    emitCodeRRR(op, dst, dst, src);
}

void Parser::addNamedFunction(ObjectId functionId, const Atom& name)
{
    assert(name);
    int32_t index = _currentFunction->addProperty(name);
    assert(index >= 0);
    _currentFunction->setProperty(nullptr, index, functionId);
}

void Parser::emitWithCount(Op value, uint32_t nparams)
{
    assert(nparams < 256);
    assert(value == Op::CALL || value == Op::NEW || value == Op::RET);
    
    emitCodeRUN(value, (value == Op::RET) ? 0 : _currentFunction->popReg(), nparams);
}

void Parser::emitDeferred()
{
    assert(!_deferred);
    assert(_deferredCodeBlocks.size() > 0);
    for (size_t i = _deferredCodeBlocks.back(); i < _deferredCode.size(); ++i) {
        _currentFunction->addCode(_deferredCode[i]);
    }
    _deferredCode.resize(_deferredCodeBlocks.back());
    _deferredCodeBlocks.pop_back();
}

void Parser::functionAddParam(const Atom& atom)
{
    if (_currentFunction->addLocal(atom) < 0) {
        m8r::String s = "param '";
        s += _program->stringFromAtom(atom);
        s += "' already exists";
        printError(s.c_str());
    }
}

void Parser::functionStart()
{
    _functions.push_back(_currentFunction);
    _currentFunction = new Function();
    ObjectId functionId = _program->addObject(_currentFunction);
    _currentFunction->setObjectId(functionId);
}

void Parser::functionParamsEnd()
{
    _currentFunction->markParamEnd();
}

ObjectId Parser::functionEnd()
{
    assert(_currentFunction && _functions.size());
    Function* function = _currentFunction;
    _currentFunction = _functions.back();
    _functions.pop_back();
    ObjectId functionId = _program->addObject(function);
    return functionId;
}

void Parser::programEnd()
{
    emit(Op::END);
}
