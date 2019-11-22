/*-------------------------------------------------------------------------
    This source file is a part of m8rscript
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2019, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/

#include "Value.h"

#include "ExecutionUnit.h"
#include "Object.h"

using namespace m8r;

m8r::String Value::toStringValue(ExecutionUnit* eu) const
{
    switch(type()) {
        case Type::None: return String("undefined");
        case Type::Function:
        case Type::Object: {
            Mad<Object> obj = asObject();
            return obj.valid() ? obj->toString(eu) : String("null");
        }
        case Type::Float: return String::toString(asFloatValue());
        case Type::Integer: return String::toString(asIntValue());
        case Type::String: {
            Mad<String> s = asString();
            return s.valid() ? *s : String("*BAD*");
        }
        case Type::StringLiteral: return eu->program()->stringFromStringLiteral(stringLiteralFromValue());
        case Type::Id: return String(eu->program()->stringFromAtom(atomFromValue()));
        case Type::Null: return String("null");
        case Type::NativeObject: return String("Native()"); // FIXME: Add formatted toString and show the address
        case Type::NativeFunction: return String("Callable()"); // FIXME: Add formatted toString and show the address
    }
}

Float Value::_toFloatValue(ExecutionUnit* eu) const
{
    switch(type()) {
        case Type::Function:
        case Type::Object: {
            Mad<Object> obj = asObject();
            Float f;
            if (obj.valid()) {
                String::toFloat(f, obj->toString(eu).c_str());
            }
            return f;
        }
        case Type::Float: return asFloatValue();
        case Type::Integer: return Float(int32FromValue(), 0);
        case Type::String: {
            const Mad<String> s = asString();
            if (!s.valid()) {
                return Float();
            }
            Float f;
            String::toFloat(f, s->c_str());
            return f;
        }
        case Type::StringLiteral: {
            const String& s = eu->program()->stringFromStringLiteral(stringLiteralFromValue());
            Float f;
            String::toFloat(f, s.c_str());
            return f;
        }
        case Type::Id:
        case Type::NativeObject:
        case Type::NativeFunction:
        case Type::Null:
            return Float();
        case Type::None:
            return Float::nan();
    }
}

Atom Value::_toIdValue(ExecutionUnit* eu) const
{
    switch(type()) {
        case Type::Function:
        case Type::Object: {
            Mad<Object> obj = asObject();
            return obj.valid() ? eu->program()->atomizeString(obj->toString(eu).c_str()) : Atom();
        }
        case Type::Integer:
        case Type::Float: return eu->program()->atomizeString(toStringValue(eu).c_str());
        case Type::String: {
            const Mad<String> s = asString();
            return s.valid() ? eu->program()->atomizeString(s->c_str()) : Atom();
        }
        case Type::StringLiteral: {
            const String& s = eu->program()->stringFromStringLiteral(stringLiteralFromValue());
            return eu->program()->atomizeString(s.c_str());
        }
        case Type::Id:
        case Type::NativeObject:
        case Type::NativeFunction:
        case Type::None:
        case Type::Null:
            return Atom();
    }
}

String Value::format(ExecutionUnit* eu, Value formatValue, uint32_t nparams)
{
    // thisValue is the format string
    String format = formatValue.toStringValue(eu);
    if (format.empty()) {
        return String();
    }
    
    int32_t nextParam = 1 - nparams;
    Value value;
    
    return String::fformat(format.c_str(), [eu, &nextParam](String::FormatType type) {
        switch(type) {
            case String::FormatType::Int:
                return Value(eu->stack().top(nextParam++).toIntValue(eu));
            case String::FormatType::String:
                return Value(Mad<String>::create(eu->stack().top(nextParam++).toStringValue(eu)));
            case String::FormatType::Float:
                return Value(eu->stack().top(nextParam++).toFloatValue(eu));
            case String::FormatType::Ptr: {
                // return a string showing <type>(value)
                Value val = eu->stack().top(nextParam++);
                Mad<String> s = Mad<String>::create();
                switch(val.type()) {
                    case Type::None:            *s = "UND()"; break;
                    case Type::Float:           *s = "FLT()"; break;
                    case Type::Object:          *s = "OBJ()"; break;
                    case Type::Function:        *s = "FUN()"; break;
                    case Type::Integer:         *s = "INT()"; break;
                    case Type::String:          *s = "STR()"; break;
                    case Type::StringLiteral:   *s = "LIT()"; break;
                    case Type::Id:              *s = "ID()";  break;
                    case Type::Null:            *s = "NUL()"; break;
                    case Type::NativeObject:    *s = "NOB()"; break;
                    case Type::NativeFunction:  *s = "NFU()"; break;
                }
                
                return Value(s);
            }
        }
    });
}

bool Value::isType(ExecutionUnit* eu, Atom atom)
{
    if (!isObject()) {
        return false;
    }
    Atom typeAtom = asObject()->typeName();
    return typeAtom == atom;
}

bool Value::isType(ExecutionUnit* eu, SA sa)
{
    return isType(eu, Atom(sa));
}

const Value Value::property(ExecutionUnit* eu, const Atom& prop) const
{
    switch(type()) {
        case Type::Function:
        case Type::Object: {
            Mad<Object> obj = asObject();
            return obj.valid() ? obj->property(prop) : Value();
        }
        case Type::Integer:
        case Type::Float: 
            // FIXME: Implement a Number object
            break;
        case Type::StringLiteral:
        case Type::String: {
            String s = toStringValue(eu);
            if (prop == Atom(SA::length)) {
                return Value(static_cast<int32_t>(s.size()));
            }
            break;
        }
        case Type::Id:
        case Type::NativeObject:
        case Type::NativeFunction:
        case Type::None:
        case Type::Null:
            break;
    }
    return Value();
}

bool Value::setProperty(ExecutionUnit* eu, const Atom& prop, const Value& value, Value::SetPropertyType type)
{
    // FIXME: Handle Integer, Float, String and StringLiteral
    Mad<Object> obj = asObject();
    return obj.valid() ? obj->setProperty(prop, value, type) : false;
}

const Value Value::element(ExecutionUnit* eu, const Value& elt) const
{
    if (isString()) {
        // This means String or StringLiteral
        int32_t index = elt.toIntValue(eu);
        const Mad<String> s = asString();
        if (s.valid()) {
            if (s->size() > index && index >= 0) {
                return Value(static_cast<int32_t>((*s)[index]));
            }
        } else {
            // Must be a string literal
            const char* s = eu->program()->stringFromStringLiteral(asStringLiteralValue());
            if (s) {
                size_t size = strlen(s);
                if (size > index && index >= 0) {
                    return Value(static_cast<int32_t>((s[index])));
                }
            }
        }
    } else {
        Mad<Object> obj = asObject();
        if (obj.valid()) {
            return obj->element(eu, elt);
        }
    }
    return Value();
}

bool Value::setElement(ExecutionUnit* eu, const Value& elt, const Value& value, bool append)
{
    // FIXME: Handle Integer, Float, String and StringLiteral
    Mad<Object> obj = asObject();
    return obj.valid() ? obj->setElement(eu, elt, value, append) : false;
}

CallReturnValue Value::call(ExecutionUnit* eu, Value thisValue, uint32_t nparams, bool ctor)
{
    // FIXME: Handle Integer, Float, String and StringLiteral
    if (isNativeFunction()) {
        return asNativeFunction()(eu, thisValue, nparams);
    }
    Mad<Object> obj = asObject();
    return obj.valid() ? obj->call(eu, thisValue, nparams, ctor) : CallReturnValue(CallReturnValue::Error::CannotCall);
}

CallReturnValue Value::callProperty(ExecutionUnit* eu, Atom prop, uint32_t nparams)
{
    switch(type()) {
        case Type::Function:
        case Type::Object: {
            Mad<Object> obj = asObject();
            return obj.valid() ? obj->callProperty(eu, prop, nparams) : CallReturnValue(CallReturnValue::Error::CannotCall);
        }
        case Type::Integer:
        case Type::Float: 
            // FIXME: Implement a Number object
            return CallReturnValue(CallReturnValue::Error::CannotCall);
        case Type::StringLiteral:
        case Type::String: {
            String s = toStringValue(eu);
            if (prop == Atom(SA::format)) {
                String s = Value::format(eu, eu->stack().top(1 - nparams), nparams - 1);
                eu->stack().push(Value(s));
                return CallReturnValue(CallReturnValue::Type::ReturnCount, 1);
            }
            if (prop == Atom(SA::trim)) {
                s = s.trim();
                eu->stack().push(Value(Mad<String>::create(s)));
                return CallReturnValue(CallReturnValue::Type::ReturnCount, 1);
            }
            if (prop == Atom(SA::split)) {
                String separator = (nparams > 0) ? eu->stack().top(1 - nparams).toStringValue(eu) : String(" ");
                bool skipEmpty = (nparams > 1) ? eu->stack().top(2 - nparams).toBoolValue(eu) : false;
                Vector<String> array = s.split(separator, skipEmpty);
                Mad<MaterObject> arrayObject = Mad<MaterObject>::create();
                arrayObject->setArray(true);
                arrayObject->resize(array.size());
                for (uint16_t i = 0; i < array.size(); ++i) {
                    (*arrayObject)[i] = Value(Mad<String>::create(array[i]));
                }
                
                eu->stack().push(Value(Mad<Object>(static_cast<Mad<Object>>(arrayObject))));
                return CallReturnValue(CallReturnValue::Type::ReturnCount, 1);
            }
            return CallReturnValue(CallReturnValue::Error::PropertyDoesNotExist);
        }
        case Type::Id:
        case Type::NativeObject:
        case Type::NativeFunction:
        case Type::None:
        case Type::Null:
            return CallReturnValue(CallReturnValue::Error::CannotCall);
    }
}

void Value::gcMark()
{
    Mad<String> string = asString();
    if (string.valid()) {
        string->setMarked(true);
        return;
    }
    
    Mad<Object> obj = asObject();
    if (obj.valid()) {
        obj->gcMark();
    }
}
