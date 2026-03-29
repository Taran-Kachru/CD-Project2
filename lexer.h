#pragma once
#include "token.h"
#include <vector>
#include <string>

// ================================================================
//  LEXER  (Lexical Analyser)
//
//  Phase 1 of the Hindi compiler pipeline.
//  Converts raw source text into a flat stream of Token objects.
//
//  Features:
//    • Hindi keyword recognition (ASCII transliteration)
//    • Integer and float literal scanning
//    • String literal scanning  (" ... ")
//    • Multi-character operators: ==  !=  <=  >=  &&  ||  +=  -=
//    • Single-line comments     ( # to end of line )
//    • Line + column tracking   (for precise error messages)
// ================================================================
class Lexer {
public:
    explicit Lexer(const string& source);

    // Tokenise the entire source; last token is always EOF
    vector<Token> tokenize();

    // Returns how many lexer-level errors were found
    int errorCount() const { return errors; }

private:
    string  src;
    size_t  pos;
    int     line, col, errors;

    char   current()            const;
    char   peek(int offset = 1) const;
    char   advance();
    bool   isAtEnd()            const;

    void   skipWhitespaceAndComments();
    Token  readNumber();
    Token  readIdentifierOrKeyword();
    Token  readString();
    Token  readOperator();

    // Hindi keyword → TokenType map
    static const unordered_map<string, TokenType> KEYWORDS;
};