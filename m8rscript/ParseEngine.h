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

#include "Atom.h"
#include "Containers.h"
#include "Parser.h"

namespace m8r {

class Function;
class Value;

//////////////////////////////////////////////////////////////////////////////
//
//  Class: ParseEngine
//
//  
//
//////////////////////////////////////////////////////////////////////////////

class ParseEngine  {
public:
  	ParseEngine(Parser* parser);
  	
  	~ParseEngine()
  	{
    }
  
    bool statement();

private:
    struct OpInfo {
        static const uint8_t LeftAssoc = 0;
        static const uint8_t RightAssoc = 1;
        uint8_t prec : 6;
        uint8_t assoc : 1;
        uint8_t sto : 1;
        Op op;
    };
        
    bool expect(Token token);
    bool expect(Token token, bool expected);
    
    Token getToken()
    {
        if (_currentToken == Token::None) {
            _currentToken = _parser->getToken(_currentTokenValue);
        }
        return _currentToken;
    }
    
    const Scanner::TokenType& getTokenValue()
    {
        if (_currentToken == Token::None) {
            _currentToken = _parser->getToken(_currentTokenValue);
        }
        return _currentTokenValue;
    }
    
    void retireToken() { _currentToken = Token::None; }

    bool functionDeclaration();
    bool compoundStatement();
    bool selectionStatement();
    bool switchStatement();
    bool iterationStatement();
    bool jumpStatement();
    uint32_t variableDeclarationList();
    bool variableDeclaration();
    
    bool arithmeticPrimary();
    bool expression(uint8_t minPrec = 1);
    
    bool leftHandSideExpression();
    bool primaryExpression();
    
    ObjectId function();
    uint32_t argumentList();
    void forLoopCondAndIt();
    void forIteration();
    bool propertyAssignment();
    bool propertyName();
    void formalParameterList();
    
    Parser* _parser;
    Token _currentToken = Token::None;
    Scanner::TokenType _currentTokenValue;
    
    std::vector<std::vector<Label>> _breakStack;
    std::vector<std::vector<Label>> _continueStack;

    struct CompareTokens
    {
        int operator()(const Token& lhs, const Token& rhs) const { return static_cast<int>(lhs) - static_cast<int>(rhs); }
    };

    static Map<Token, OpInfo, CompareTokens> _opInfo;
};

}
