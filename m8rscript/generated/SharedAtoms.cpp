// This file is generated. Do not edit

#include "SharedAtoms.h"

#include "Defines.h"
#include <stdlib.h>

#define AtomEntry(a) _##a

static const char _Array[] ROMSTR_ATTR = "Array";
static const char _Base64[] ROMSTR_ATTR = "Base64";
static const char _BothEdges[] ROMSTR_ATTR = "BothEdges";
static const char _Connected[] ROMSTR_ATTR = "Connected";
static const char _Disconnected[] ROMSTR_ATTR = "Disconnected";
static const char _Error[] ROMSTR_ATTR = "Error";
static const char _FallingEdge[] ROMSTR_ATTR = "FallingEdge";
static const char _High[] ROMSTR_ATTR = "High";
static const char _GPIO[] ROMSTR_ATTR = "GPIO";
static const char _Input[] ROMSTR_ATTR = "Input";
static const char _InputPulldown[] ROMSTR_ATTR = "InputPulldown";
static const char _InputPullup[] ROMSTR_ATTR = "InputPullup";
static const char _IPAddr[] ROMSTR_ATTR = "IPAddr";
static const char _JSON[] ROMSTR_ATTR = "JSON";
static const char _Low[] ROMSTR_ATTR = "Low";
static const char _MaxConnections[] ROMSTR_ATTR = "MaxConnections";
static const char _None[] ROMSTR_ATTR = "None";
static const char _Object[] ROMSTR_ATTR = "Object";
static const char _Output[] ROMSTR_ATTR = "Output";
static const char _OutputOpenDrain[] ROMSTR_ATTR = "OutputOpenDrain";
static const char _PinMode[] ROMSTR_ATTR = "PinMode";
static const char _ReceivedData[] ROMSTR_ATTR = "ReceivedData";
static const char _Reconnected[] ROMSTR_ATTR = "Reconnected";
static const char _RisingEdge[] ROMSTR_ATTR = "RisingEdge";
static const char _SentData[] ROMSTR_ATTR = "SentData";
static const char _TCP[] ROMSTR_ATTR = "TCP";
static const char _Trigger[] ROMSTR_ATTR = "Trigger";
static const char _UDP[] ROMSTR_ATTR = "UDP";
static const char _a[] ROMSTR_ATTR = "a";
static const char _arguments[] ROMSTR_ATTR = "arguments";
static const char _b[] ROMSTR_ATTR = "b";
static const char _c[] ROMSTR_ATTR = "c";
static const char _call[] ROMSTR_ATTR = "call";
static const char _constructor[] ROMSTR_ATTR = "constructor";
static const char _currentTime[] ROMSTR_ATTR = "currentTime";
static const char _d[] ROMSTR_ATTR = "d";
static const char _decode[] ROMSTR_ATTR = "decode";
static const char _delay[] ROMSTR_ATTR = "delay";
static const char _digitalRead[] ROMSTR_ATTR = "digitalRead";
static const char _digitalWrite[] ROMSTR_ATTR = "digitalWrite";
static const char _disconnect[] ROMSTR_ATTR = "disconnect";
static const char _encode[] ROMSTR_ATTR = "encode";
static const char _end[] ROMSTR_ATTR = "end";
static const char _iterator[] ROMSTR_ATTR = "iterator";
static const char _length[] ROMSTR_ATTR = "length";
static const char _lookupHostname[] ROMSTR_ATTR = "lookupHostname";
static const char _meta[] ROMSTR_ATTR = "meta";
static const char _next[] ROMSTR_ATTR = "next";
static const char _onInterrupt[] ROMSTR_ATTR = "onInterrupt";
static const char _parse[] ROMSTR_ATTR = "parse";
static const char _print[] ROMSTR_ATTR = "print";
static const char _printf[] ROMSTR_ATTR = "printf";
static const char _println[] ROMSTR_ATTR = "println";
static const char _send[] ROMSTR_ATTR = "send";
static const char _setPinMode[] ROMSTR_ATTR = "setPinMode";
static const char _split[] ROMSTR_ATTR = "split";
static const char _stringify[] ROMSTR_ATTR = "stringify";
static const char _toFloat[] ROMSTR_ATTR = "toFloat";
static const char _toInt[] ROMSTR_ATTR = "toInt";
static const char _toUInt[] ROMSTR_ATTR = "toUInt";
static const char _trim[] ROMSTR_ATTR = "trim";
static const char _value[] ROMSTR_ATTR = "value";
static const char ___nativeObject[] ROMSTR_ATTR = "__nativeObject";
static const char ___this[] ROMSTR_ATTR = "this";
static const char ___typeName[] ROMSTR_ATTR = "__typeName";

const char* RODATA_ATTR sharedAtoms[] = {
    AtomEntry(Array),
    AtomEntry(Base64),
    AtomEntry(BothEdges),
    AtomEntry(Connected),
    AtomEntry(Disconnected),
    AtomEntry(Error),
    AtomEntry(FallingEdge),
    AtomEntry(GPIO),
    AtomEntry(High),
    AtomEntry(Input),
    AtomEntry(InputPulldown),
    AtomEntry(InputPullup),
    AtomEntry(IPAddr),
    AtomEntry(JSON),
    AtomEntry(Low),
    AtomEntry(MaxConnections),
    AtomEntry(None),
    AtomEntry(Object),
    AtomEntry(Output),
    AtomEntry(OutputOpenDrain),
    AtomEntry(PinMode),
    AtomEntry(ReceivedData),
    AtomEntry(Reconnected),
    AtomEntry(RisingEdge),
    AtomEntry(SentData),
    AtomEntry(TCP),
    AtomEntry(Trigger),
    AtomEntry(UDP),
    
    AtomEntry(a),
    AtomEntry(arguments),
    AtomEntry(b),
    AtomEntry(c),
    AtomEntry(call),
    AtomEntry(constructor),
    AtomEntry(currentTime),
    AtomEntry(d),
    AtomEntry(decode),
    AtomEntry(delay),
    AtomEntry(digitalRead),
    AtomEntry(digitalWrite),
    AtomEntry(disconnect),
    AtomEntry(encode),
    AtomEntry(end),
    AtomEntry(iterator),
    AtomEntry(length),
    AtomEntry(lookupHostname),
    AtomEntry(meta),
    AtomEntry(next),
    AtomEntry(onInterrupt),
    AtomEntry(parse),
    AtomEntry(print),
    AtomEntry(printf),
    AtomEntry(println),
    AtomEntry(send),
    AtomEntry(setPinMode),
    AtomEntry(split),
    AtomEntry(stringify),
    AtomEntry(toFloat),
    AtomEntry(toInt),
    AtomEntry(toUInt),
    AtomEntry(trim),
    AtomEntry(value),
    AtomEntry(__nativeObject),
    AtomEntry(__this),
    AtomEntry(__typeName),
};

const char* sharedAtom(enum SharedAtom id)
{
    return sharedAtoms[static_cast<uint32_t>(id)];
}
