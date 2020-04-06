/*-------------------------------------------------------------------------
    This source file is a part of m8rscript
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2019, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/

#include "MString.h"

#include "MStream.h"
#include "Scanner.h"
#include "slre.h"

using namespace m8r;

m8r::String& m8r::String::erase(uint16_t pos, uint16_t len)
{
    if (pos >= _size - 1) {
        return *this;
    }
    if (pos + len >= _size) {
        len = _size - pos - 1;
    }
    memmove(_data.get() + pos, _data.get() + pos + len, _size - pos - len);
    _size -= len;
    return *this;
}

m8r::String m8r::String::slice(int32_t start, int32_t end) const
{
    int32_t sz = static_cast<int32_t>(size());
    if (start < 0) {
        start = sz + start;
    }
    if (end < 0) {
        end = sz + end;
    }
    if (end > sz) {
        end = sz;
    }
    if (start >= end) {
        return String();
    }
    return String(_data.get() + start, end - start);
}

m8r::String m8r::String::trim() const
{
    if (_size < 2 || !_data.valid()) {
        return String();
    }
    uint16_t l = _size - 1;
    char* s = _data.get();
    while (isspace(s[l - 1])) {
        --l;
    }
    while (*s && isspace(*s)) {
        ++s;
        --l;
    }
    return String(s, static_cast<int32_t>(l));
}

Vector<m8r::String> m8r::String::split(const m8r::String& separator, bool skipEmpty) const
{
    Vector<String> array;
    char* p = _data.get();
    while (1) {
        char* n = strstr(p, separator.c_str());
        if (!n || n - p != 0 || !skipEmpty) {
            array.push_back(String(p, static_cast<int32_t>(n ? (n - p) : -1)));
        }
        if (!n) {
            break;
        }
        p = n ? (n + separator.size()) : nullptr;
    }
    return array;
}

m8r::String m8r::String::join(const Vector<m8r::String>& array, const m8r::String& separator)
{
    String s;
    bool first = true;
    for (auto it : array) {
        if (first) {
            first = false;
        } else {
            s += separator;
        }
        s += it;
    }
    return s;
}

m8r::String m8r::String::join(const Vector<char>& array)
{
    String s;
    s.ensureCapacity(array.size());
    for (auto it : array) {
        s += it;
    }
    return s;
}
void m8r::String::doEnsureCapacity(uint16_t size)
{
    _capacity = _capacity ? _capacity * 2 : 1;
    if (_capacity < size) {
        _capacity = size;
    }
    Mad<char> newData = Mad<char>::create(_capacity);
    assert(newData.valid());
    if (_data.valid()) {
        if (newData.valid()) {
            memcpy(newData.get(), _data.get(), _size);
        } else {
            _capacity = 0;
            _size = 1;
        }
        _data.destroy();
    }
    _data = newData;
}

static int32_t intToString(Float::decompose_type x, char* str, int16_t dp, uint8_t decimalDigits)
{
    // Adjust x and dp for decimalDigits
    if (dp > decimalDigits) {
        int16_t exp = dp - decimalDigits;
        while (exp-- > 1) {
            x /= 10;
        }
        
        // We've tossed all but one digit. Round and then toss it
        x = (x + 5) / 10;
        dp = decimalDigits;
    }
    
    int32_t i = 0;
    bool haveDP = false;
    
    while (x) {
        str[i++] = (x % 10) + '0';
        x /= 10;
        if (--dp == 0) {
            str[i++] = '.';
            haveDP = true;
        }
    }
    
    if (dp > 0) {
        while (dp--) {
            str[i++] = '0';
        }
        str[i++] = '.';
        haveDP = true;
    }
    assert(i > 0);
    if (str[i-1] == '.') {
        str[i++] = '0';
    }
    
    std::reverse(str, str + i);
    str[i] = '\0';

    if (haveDP) {
        i--;
        while (str[i] == '0') {
            str[i--] = '\0';
        }
        if (str[i] == '.') {
            str[i--] = '\0';
        }
        i++;
    }

    return i;
}

static bool toString(char* buf, Float::decompose_type value, int16_t& exp, uint8_t decimalDigits)
{
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        exp = 0;
        return true;
    }
    
    if (!exp) {
        intToString(value, buf, 0, decimalDigits);
        return true;
    }

    // See how many digits we have
    Float::decompose_type v = value;
    int digits = 0;
    for ( ; v > 0; ++digits, v /= 10) ;
    v = value;
    int32_t dp;
    if (exp + digits > Float::MaxDigits || -exp > Float::MaxDigits) {
        // Scientific notation
        dp = digits - 1;
        exp += dp;
    } else {
        dp = -exp;
        exp = 0;
    }
    
    int32_t i = intToString(value, buf, dp, decimalDigits);
    
    if (exp) {
        buf[i++] = 'e';
        if (exp < 0) {
            buf[i++] = '-';
            exp = -exp;
        }
        intToString(exp, buf + i, 0, decimalDigits);
    }
    
    return true;
}

m8r::String::String(Float value, uint8_t decimalDigits)
{
    //          sign    digits  dp      'e'     dp      exp     '\0'
    char buf[   1 +     16 +    1 +     1 +     1 +     3 +     1];
    int16_t exp;
    Float::decompose_type mantissa;
    value.decompose(mantissa, exp);
    if (mantissa < 0) {
        buf[0] = '-';
        mantissa = - mantissa;
        ::toString(buf + 1, mantissa, exp, decimalDigits);
    } else {
        ::toString(buf, mantissa, exp, decimalDigits);
    }
    *this = String(buf);
}

m8r::String::String(uint32_t value)
{
    char buf[12];
    int16_t exp = 0;
    ::toString(buf, value, exp, 0);
    *this = String(buf);
}

m8r::String::String(int32_t value)
{
    String s;
    if (value < 0) {
        *this = String('-') + String(static_cast<uint32_t>(-value));
    } else {
        *this = String(static_cast<uint32_t>(value));
    }
}

m8r::String::String(void* value)
{
    // Convert to a uint32_t. This will truncate the pointer on Mac
    *this = String::format("0x%08x", static_cast<uint32_t>(reinterpret_cast<intptr_t>(value)));
}

bool m8r::String::toFloat(Float& f, const char* s, bool allowWhitespace)
{
    StringStream stream(s);
    Scanner scanner(&stream);
    bool neg = false;
    Scanner::TokenType type;
      Token token = scanner.getToken(type, allowWhitespace);
    if (token == Token::Minus) {
        neg = true;
        token = scanner.getToken(type, allowWhitespace);
    }
    if (token == Token::Float || token == Token::Integer) {
        f = (token == Token::Float) ? Float(type.number) : Float(type.integer, 0);
        if (neg) {
            f = -f;
        }
        return true;
    }
    return false;
}

bool m8r::String::toInt(int32_t& i, const char* s, bool allowWhitespace)
{
    StringStream stream(s);
    Scanner scanner(&stream);
    bool neg = false;
    Scanner::TokenType type;
      Token token = scanner.getToken(type, allowWhitespace);
    if (token == Token::Minus) {
        neg = true;
        token = scanner.getToken(type, allowWhitespace);
    }
    if (token == Token::Integer && type.integer <= std::numeric_limits<int32_t>::max()) {
        i = type.integer;
        if (neg) {
            i = -i;
        }
        return true;
    }
    return false;
}

bool m8r::String::toUInt(uint32_t& u, const char* s, bool allowWhitespace)
{
    StringStream stream(s);
    Scanner scanner(&stream);
    Scanner::TokenType type;
      Token token = scanner.getToken(type, allowWhitespace);
    if (token == Token::Integer) {
        u = type.integer;
        return true;
    }
    return false;
}

m8r::String m8r::String::prettySize(uint32_t size, uint8_t decimalDigits)
{
    m8r::String s;
    
    if (size < 1000) {
        return String(size) + ' ';
    } else if (size < 1000000) {
        return String(Float(static_cast<int32_t>(size)) / Float(1000), decimalDigits) + " K";
    } else if (size < 1000000000) {
        return String(Float(static_cast<int32_t>(size)) / Float(1000000), decimalDigits) + " M";
    } else {
        return String(Float(static_cast<int32_t>(size)) / Float(1000000000), decimalDigits) + " G";
    }
}

static inline char nibbleToHexChar(uint8_t b, bool upperCase)
{
    char base = upperCase ? 'A' : 'a';
    return (b >= 10) ? (b - 10 + base) : (b + '0');
}

static void toInteger(char* result, bool zeroFill, int32_t width, char type, uint32_t number)
{
    bool sign = false;
    if (type == 'i' || type == 'd') {
        int32_t signedNumber = static_cast<int32_t>(number);
        if (signedNumber < 0) {
            sign = true;
            number = static_cast<uint32_t>(-signedNumber);
        }
    }
    
    bool hex = type == 'x' || type == 'X';
    
    char buf[20];
    buf[19] = '\0';
    char* p = buf + 19;
    if (number == 0) {
        *--p = '0';
        --width;
    } else {
        while (number) {
            if (hex) {
                *--p = nibbleToHexChar(number & 0x0f, type == 'X');
                number /= 16;
            } else {
                *--p = (number % 10) + '0';
                number /= 10;
            }
            --width;
        }
    }
    
    while (width-- > 0) {
        *--p = zeroFill ? '0' : ' ';
        if (p == buf) {
            break;
        }
    }
    
    if (sign) {
        *--p = '-';
    }
    strcpy(result, p);
}

m8r::String m8r::String::fformat(const char* fmt, std::function<void(FormatType, String&)> func)
{
    if (!fmt || fmt[0] == '\0') {
        return String();
    }
    
    String resultString;
    String tmpString;
    
    static ROMString formatRegexROM = ROMSTR("(%)([\\d]*)(.?)([\\d]*)([c|s|d|i|x|X|u|f|e|E|g|G|p])");
        
    uint16_t formatRegexSize = ROMstrlen(formatRegexROM) + 1;
    Mad<char> formatRegex = Mad<char>::create(formatRegexSize);
    ROMmemcpy(formatRegex.get(), formatRegexROM, formatRegexSize);
    
    int size = static_cast<int>(strlen(fmt));
    const char* start = fmt;
    const char* s = start;
    while (true) {
        struct slre_cap caps[5];
        memset(caps, 0, sizeof(caps));
        int next = slre_match(formatRegex.get(), s, size - static_cast<int>(s - start), caps, 5, 0);
        if (next == SLRE_NO_MATCH) {
            // Print the remainder of the string
            resultString += s;
            formatRegex.destroy();
            return resultString;
        }
        if (next < 0) {
            formatRegex.destroy();
            return String();
        }
        
        // Output anything from s to the '%'
        assert(caps[0].len == 1);
        if (s != caps[0].ptr) {
            resultString += String(s, static_cast<int32_t>(caps[0].ptr - s));
        }
        
        // FIXME: handle the leading number(s) in the format
        assert(caps[4].len == 1);
        
        int32_t width = 0;
        bool zeroFill = false;
        if (caps[1].len) {
            String::toInt(width, caps[1].ptr);
            if (caps[1].ptr[0] == '0') {
                zeroFill = true;
            }
        }
        
        char formatChar = *(caps[4].ptr);
        
        switch (formatChar) {
            case 'c': {
                func(FormatType::Int, tmpString);
                uint8_t uc = static_cast<char>(tmpString.toUInt());
                char escapeChar = '\0';
                switch(uc) {
                    case 0x07: escapeChar = 'a'; break;
                    case 0x08: escapeChar = 'b'; break;
                    case 0x09: escapeChar = 't'; break;
                    case 0x0a: escapeChar = 'n'; break;
                    case 0x0b: escapeChar = 'v'; break;
                    case 0x0c: escapeChar = 'f'; break;
                    case 0x0d: escapeChar = 'r'; break;
                    case 0x1b: escapeChar = 'e'; break;
                }
                if (escapeChar) {
                    resultString += '\\';
                    resultString += escapeChar;
                } else if (uc < ' ') {
                    resultString += "\\x";
                    resultString += nibbleToHexChar(uc >> 4, false);
                    resultString += nibbleToHexChar(uc & 0x0f, false);
                } else {
                    resultString += static_cast<char>(uc);
                }
                break;
            }
            case 's':
                func(FormatType::String, tmpString);
                resultString += tmpString;
                break;
            case 'd':
            case 'i':
            case 'x':
            case 'X':
            case 'u': {
                func(FormatType::Int, tmpString);
                char numberBuf[20];
                toInteger(numberBuf, zeroFill, width, formatChar, tmpString.toUInt());
                resultString += numberBuf;
                break;
            }
            case 'f':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
                func(FormatType::Float, tmpString);
                resultString += tmpString;
                break;
            case 'p': {
                func(FormatType::Ptr, tmpString);
                resultString += tmpString;
                break;
            }
            default:
                formatRegex.destroy();
                return String();
        }
        
        s += next;
    }
}

m8r::String m8r::String::vformat(const char* fmt, va_list args)
{
    String s = fformat(fmt, [&args](String::FormatType type, String& s) {
        switch(type) {
            case String::FormatType::Int:
                s = String(static_cast<int32_t>(va_arg(args, int)));
                return;
            case String::FormatType::String:
                s = String(va_arg(args, const char*));
                return;
            case String::FormatType::Float:
                // TODO: Implement
                //s = String::toString(Float(va_arg(args, double)));
                return;
            case String::FormatType::Ptr: {
                s = String(va_arg(args, void*));
                return;
            default:
                s = "UNKNOWN";
                return;
            }
        }
    });
    return s;
}

m8r::String m8r::String::vformat(ROMString romfmt, va_list args)
{
    return vformat(String(romfmt).c_str(), args);
}
