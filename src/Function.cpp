/*-------------------------------------------------------------------------
    This source file is a part of m8rscript
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2019, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/

#include "Function.h"

#include "ExecutionUnit.h"

using namespace m8r;

Function::Function()
{
}

CallReturnValue Function::callProperty(ExecutionUnit* eu, Atom prop, uint32_t nparams)
{
    if (prop == Atom(SA::call)) {
        if (nparams < 1) {
            return CallReturnValue(CallReturnValue::Error::WrongNumberOfParams);
        }
        
        // Remove the first element and use it as the this pointer
        Value self = eu->stack().top(1 - nparams);
        eu->stack().remove(1 - nparams);
        nparams--;
    
        return call(eu, self, nparams, false);
    }
    return CallReturnValue(CallReturnValue::Error::PropertyDoesNotExist);
}

CallReturnValue Function::call(ExecutionUnit* eu, Value thisValue, uint32_t nparams, bool ctor)
{
    eu->startFunction(Mad<Object>(this), thisValue.asObject(), nparams);
    return CallReturnValue(CallReturnValue::Type::FunctionStart);
}

int32_t Function::addLocal(const Atom& atom)
{
    for (auto name : _locals) {
        if (name == atom) {
            return -1;
        }
    }
    _locals.push_back(atom);
    return static_cast<int32_t>(_locals.size()) - 1;
}

int32_t Function::localIndex(const Atom& name) const
{
    for (int32_t i = 0; i < static_cast<int32_t>(_locals.size()); ++i) {
        if (_locals[i] == name) {
            return i;
        }
    }
    return -1;
}

uint32_t Function::addUpValue(uint32_t index, uint16_t frame, Atom name)
{
    assert(_upValues.size() < std::numeric_limits<uint16_t>::max());
    UpValueEntry entry(index, frame, name);
    
    for (uint32_t i = 0; i < _upValues.size(); ++i) {
        if (_upValues[i] == entry) {
            return i;
        }
            
    }
    _upValues.push_back(entry);
    return static_cast<uint32_t>(_upValues.size()) - 1;
}

uint32_t Function::upValueStackIndex(ExecutionUnit* eu, uint32_t index) const
{
    return eu->upValueStackIndex(_upValues[index]._index, _upValues[index]._frame);
}

bool Function::loadUpValue(ExecutionUnit* eu, uint32_t index, Value& value) const
{
    assert(index < _upValues.size());
    value = eu->stack().at(upValueStackIndex(eu, index));
    return true;
}

bool Function::storeUpValue(ExecutionUnit* eu, uint32_t index, const Value& value)
{
    return false;
}
