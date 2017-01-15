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

#include "Global.h"

#include "Program.h"
#include "SystemInterface.h"
#include "ExecutionUnit.h"
#include "slre.h"
#include <string>

using namespace m8r;

Global::Global(Program* program)
    : ObjectFactory(program)
    , _arguments(program)
    , _base64(program)
    , _gpio(program)
    , _iterator(program)
    , _currentTime(currentTime)
    , _delay(delay)
    , _print(print)
    , _printf(printf)
    , _println(println)
{
    addObject(program, ROMSTR("currentTime"), &_currentTime);
    addObject(program, ROMSTR("delay"), &_delay);
    addObject(program, ROMSTR("print"), &_print);
    addObject(program, ROMSTR("printf"), &_printf);
    addObject(program, ROMSTR("println"), &_println);

    addObject(program, ROMSTR("arguments"), &_arguments);
    addObject(program, ROMSTR("Iterator"), &_iterator);
    
    addValue(program, ROMSTR("Base64"), Value(_base64.objectId()));
    addValue(program, ROMSTR("GPIO"), Value(_gpio.objectId()));
}

CallReturnValue Global::currentTime(ExecutionUnit* eu, Value thisValue, uint32_t nparams)
{
    uint64_t t = SystemInterface::shared()->currentMicroseconds();
    eu->stack().push(Float(static_cast<Float::value_type>(t), -6));
    return CallReturnValue(CallReturnValue::Type::ReturnCount, 1);
}

CallReturnValue Global::delay(ExecutionUnit* eu, Value thisValue, uint32_t nparams)
{
    uint32_t ms = eu->stack().top().toIntValue(eu);
    return CallReturnValue(CallReturnValue::Type::MsDelay, ms);
}

CallReturnValue Global::print(ExecutionUnit* eu, Value thisValue, uint32_t nparams)
{
    for (int32_t i = 1 - nparams; i <= 0; ++i) {
        SystemInterface::shared()->printf(eu->stack().top(i).toStringValue(eu).c_str());
    }
    return CallReturnValue(CallReturnValue::Type::ReturnCount, 0);
}

CallReturnValue Global::printf(ExecutionUnit* eu, Value thisValue, uint32_t nparams)
{
    static const char* formatRegex = "(%)([\\d]*)(.?)([\\d]*)([c|s|d|i|x|X|u|f|e|E|g|G|p])";
    
    int32_t nextParam = 1 - nparams;

    const String& format = eu->stack().top(nextParam++).toStringValue(eu);
    int size = static_cast<int>(format.size());
    const char* s = format.c_str();
    struct slre_cap caps[5];
    memset(caps, 0, sizeof(caps));
    while (true) {
        int next = slre_match(formatRegex, s, size, caps, 5, 0);
        if (nextParam > 0 || next == SLRE_NO_MATCH) {
            // Print the remainder of the string
            SystemInterface::shared()->printf(s);
            return CallReturnValue(CallReturnValue::Type::ReturnCount, 0);
        }
        if (next < 0) {
            return CallReturnValue(CallReturnValue::Type::Error);
        }
        
        // Output anything from s to the '%'
        assert(caps[0].len == 1);
        if (s != caps[0].ptr) {
            String str(s, static_cast<int32_t>(caps[0].ptr - s));
            SystemInterface::shared()->printf(str.c_str());
        }
        
        // FIXME: handle the leading number(s) in the format
        assert(caps[4].len == 1);
        Value value = eu->stack().top(nextParam++);
        char formatChar = *(caps[4].ptr);
        switch (formatChar) {
            case 'c':
                SystemInterface::shared()->printf("%c", value.toIntValue(eu));
                break;
            case 's':
                SystemInterface::shared()->printf("%s", value.toStringValue(eu).c_str());
                break;
            case 'd':
            case 'i':
                SystemInterface::shared()->printf("%d", value.toIntValue(eu));
                break;
            case 'x':
            case 'X':
                SystemInterface::shared()->printf((formatChar == 'x') ? "%x" : "%X", static_cast<uint32_t>(value.toIntValue(eu)));
                break;
            case 'u':
                SystemInterface::shared()->printf("%u", static_cast<uint32_t>(value.toIntValue(eu)));
                break;
            case 'f':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
                // FIXME: Implement
                break;
            case 'p':
                SystemInterface::shared()->printf("%p", *(reinterpret_cast<void**>(&value)));
                break;
            default: return CallReturnValue(CallReturnValue::Type::Error);
        }
        
        s += next;
    }
}

CallReturnValue Global::println(ExecutionUnit* eu, Value thisValue, uint32_t nparams)
{
    for (int32_t i = 1 - nparams; i <= 0; ++i) {
        SystemInterface::shared()->printf(eu->stack().top(i).toStringValue(eu).c_str());
    }
    SystemInterface::shared()->printf("\n");

    return CallReturnValue(CallReturnValue::Type::ReturnCount, 0);
}
