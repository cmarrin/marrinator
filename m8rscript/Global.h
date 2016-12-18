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

#pragma once

#include "Object.h"

namespace m8r {

class SystemInterface;

class Serial : public Object {
public:
    Serial(Program*);

    virtual const char* typeName() const override { return "Serial"; }

    virtual const Value property(const Atom&) const override;

private:
    Atom _beginAtom;
    Atom _printAtom;
    Atom _printfAtom;
    
    static CallReturnValue begin(ExecutionUnit*, uint32_t nparams);
    static CallReturnValue print(ExecutionUnit*, uint32_t nparams);
    static CallReturnValue printf(ExecutionUnit*, uint32_t nparams);

    NativeFunction _begin;
    NativeFunction _print;
    NativeFunction _printf;
};

class Global : public Object {
public:
    Global(SystemInterface*, Program*);
    
    virtual ~Global();
    
    virtual const char* typeName() const override { return "Global"; }

    // Global has built-in properties. Handle those here
    virtual const Value property(const Atom&) const override;
    virtual bool setProperty(ExecutionUnit*, const Atom& prop, const Value&) override;
    virtual Atom propertyName(uint32_t index) const override;
    virtual size_t propertyCount() const override;
    
    SystemInterface* system() const { return _system; }

protected:
    virtual bool serialize(Stream*, Error&, Program*) const override
    {
        return true;
    }

    virtual bool deserialize(Stream*, Error&, Program*, const AtomTable&, const std::vector<char>&) override
    {
        return true;
    }

    uint64_t _startTime = 0;

    static constexpr size_t PropertyCount = 5; // Date, GPIO, Serial, Base64 and System
    
    enum class Property : uint8_t
    {
        None = 0,
        print = 0x01,
        System = 0x10, System_delay, 
        Date = 0x20, Date_now,
        GPIO = 0x30, GPIO_setPinMode, GPIO_digitalWrite, GPIO_PinMode, GPIO_Trigger,
                GPIO_PinMode_Output, GPIO_PinMode_OutputOpenDrain,
                    GPIO_PinMode_Input, GPIO_PinMode_InputPullup, GPIO_PinMode_InputPulldown,
                GPIO_Trigger_None, GPIO_Trigger_RisingEdge, GPIO_Trigger_FallingEdge, 
                    GPIO_Trigger_BothEdges, GPIO_Trigger_High, GPIO_Trigger_Low,
        Serial = 0x40, Serial_begin, Serial_print, Serial_printf,
        Base64 = 0x50, Base64_encode, Base64_decode,
    };
    
    SystemInterface* _system;

private:        
    Atom _DateAtom;
    Atom _GPIOAtom;
    Atom _SerialAtom;
    Atom _Base64Atom;
    Atom _SystemAtom;
    Atom _PinModeAtom;
    Atom _TriggerAtom;
    
    Atom _nowAtom;
    Atom _delayAtom;
    Atom _setPinModeAtom;
    Atom _digitalWriteAtom;
    Atom _digitalReadAtom;
    Atom _onInterruptAtom;
    Atom _encodeAtom;
    Atom _decodeAtom;
    Atom _OutputAtom;
    Atom _OutputOpenDrainAtom;
    Atom _InputAtom;
    Atom _InputPullupAtom;
    Atom _InputPulldownAtom;
    Atom _NoneAtom;
    Atom _RisingEdgeAtom;
    Atom _FallingEdgeAtom;
    Atom _BothEdgesAtom;
    Atom _LowAtom;
    Atom _HighAtom;
    
    Serial _serial;
};
    
}
