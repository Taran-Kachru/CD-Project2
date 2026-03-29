#include "language.h"
#include "lexer.h"
#include <cctype>
#include <iostream>

const unordered_map<string, TokenType> Lexer::KEYWORDS = {
    {KW_INT_WORD,      TokenType::KW_INT},
    {KW_FLOAT_WORD,    TokenType::KW_FLOAT},
    {KW_BOOL_WORD,     TokenType::KW_BOOL},
    {KW_VOID_WORD,     TokenType::KW_VOID},
    {KW_IF_WORD,       TokenType::KW_IF},
    {KW_ELSE_WORD,     TokenType::KW_ELSE},
    {KW_WHILE_WORD,    TokenType::KW_WHILE},
    {KW_FOR_WORD,      TokenType::KW_FOR},
    {KW_BREAK_WORD,    TokenType::KW_BREAK},
    {KW_CONTINUE_WORD, TokenType::KW_CONTINUE},
    {KW_FUNC_WORD,     TokenType::KW_FUNC},
    {KW_RETURN_WORD,   TokenType::KW_RETURN},
    {KW_PRINT_WORD,    TokenType::KW_PRINT},
    {KW_INPUT_WORD,    TokenType::KW_INPUT},
    {KW_TRUE_WORD,     TokenType::KW_TRUE},
    {KW_FALSE_WORD,    TokenType::KW_FALSE},
};

// ── Constructor ──────────────────────────────────────────────────
Lexer::Lexer(const string& source)
    : src(source), pos(0), line(1), col(1), errors(0) {}

// ── Character helpers ────────────────────────────────────────────
char Lexer::current() const {
    return isAtEnd() ? '\0' : src[pos];
}
char Lexer::peek(int offset) const {
    size_t p = pos + offset;
    return (p < src.size()) ? src[p] : '\0';
}
char Lexer::advance() {
    char c = src[pos++];
    if (c == '\n') { line++; col = 1; } else col++;
    return c;
}
bool Lexer::isAtEnd() const { return pos >= src.size(); }

// ── Skip whitespace and # comments ──────────────────────────────
void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = current();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else if (c == '#') {
            while (!isAtEnd() && current() != '\n') advance();
        } else {
            break;
        }
    }
}

// ── Number: integer or float ─────────────────────────────────────
Token Lexer::readNumber() {
    int startLine = line, startCol = col;
    string num;
    bool isFloat = false;

    while (!isAtEnd() && isdigit(current())) num += advance();

    if (!isAtEnd() && current() == '.' && isdigit(peek())) {
        isFloat = true;
        num += advance();                        // consume '.'
        while (!isAtEnd() && isdigit(current())) num += advance();
    }

    TokenType tt = isFloat ? TokenType::FLOAT_LITERAL : TokenType::INTEGER_LITERAL;
    return Token(tt, num, startLine, startCol);
}

// ── Identifier or keyword ────────────────────────────────────────
Token Lexer::readIdentifierOrKeyword() {
    int startLine = line, startCol = col;
    string word;

    while (!isAtEnd() && (isalnum(current()) || current() == '_'))
        word += advance();

    auto it = KEYWORDS.find(word);
    if (it != KEYWORDS.end())
        return Token(it->second, word, startLine, startCol);

    if (word == KW_TRUE_WORD || word == KW_FALSE_WORD)
        return Token(TokenType::BOOL_LITERAL, word, startLine, startCol);

    return Token(TokenType::IDENTIFIER, word, startLine, startCol);
}

// ── String literal ───────────────────────────────────────────────
Token Lexer::readString() {
    int startLine = line, startCol = col;
    advance();  // consume opening "
    string str;

    while (!isAtEnd() && current() != '"') {
        if (current() == '\\') {
            advance();
            switch (current()) {
                case 'n':  str += '\n'; break;
                case 't':  str += '\t'; break;
                case '"':  str += '"';  break;
                case '\\': str += '\\'; break;
                default:   str += current();
            }
        } else {
            str += current();
        }
        advance();
    }

    if (isAtEnd()) {
        cerr << "[Lexer Error] Line " << startLine
             << ": Unterminated string literal\n";
        errors++;
    } else {
        advance(); // consume closing "
    }

    return Token(TokenType::STRING_LITERAL, str, startLine, startCol);
}

// ── Operators and delimiters ─────────────────────────────────────
Token Lexer::readOperator() {
    int startLine = line, startCol = col;
    char c = advance();

    switch (c) {
        case '+':
            if (!isAtEnd() && current() == '=') { advance(); return Token(TokenType::OP_PLUS_EQ,  "+=", startLine, startCol); }
            return Token(TokenType::OP_PLUS,  "+", startLine, startCol);
        case '-':
            if (!isAtEnd() && current() == '=') { advance(); return Token(TokenType::OP_MINUS_EQ, "-=", startLine, startCol); }
            return Token(TokenType::OP_MINUS, "-", startLine, startCol);
        case '*':
            if (!isAtEnd() && current() == '=') { advance(); return Token(TokenType::OP_MUL_EQ,   "*=", startLine, startCol); }
            return Token(TokenType::OP_MUL,   "*", startLine, startCol);
        case '/':
            if (!isAtEnd() && current() == '=') { advance(); return Token(TokenType::OP_DIV_EQ,   "/=", startLine, startCol); }
            return Token(TokenType::OP_DIV,   "/", startLine, startCol);
        case '%': return Token(TokenType::OP_MOD,  "%", startLine, startCol);
        case '=':
            if (!isAtEnd() && current() == '=') { advance(); return Token(TokenType::OP_EQ,  "==", startLine, startCol); }
            return Token(TokenType::OP_ASSIGN, "=", startLine, startCol);
        case '!':
            if (!isAtEnd() && current() == '=') { advance(); return Token(TokenType::OP_NEQ, "!=", startLine, startCol); }
            return Token(TokenType::OP_NOT,    "!",  startLine, startCol);
        case '<':
            if (!isAtEnd() && current() == '=') { advance(); return Token(TokenType::OP_LTE, "<=", startLine, startCol); }
            return Token(TokenType::OP_LT,     "<",  startLine, startCol);
        case '>':
            if (!isAtEnd() && current() == '=') { advance(); return Token(TokenType::OP_GTE, ">=", startLine, startCol); }
            return Token(TokenType::OP_GT,     ">",  startLine, startCol);
        case '&':
            if (!isAtEnd() && current() == '&') { advance(); return Token(TokenType::OP_AND, "&&", startLine, startCol); }
            break;
        case '|':
            if (!isAtEnd() && current() == '|') { advance(); return Token(TokenType::OP_OR,  "||", startLine, startCol); }
            break;
        case '(': return Token(TokenType::LPAREN,    "(", startLine, startCol);
        case ')': return Token(TokenType::RPAREN,    ")", startLine, startCol);
        case '{': return Token(TokenType::LBRACE,    "{", startLine, startCol);
        case '}': return Token(TokenType::RBRACE,    "}", startLine, startCol);
        case '[': return Token(TokenType::LBRACKET,  "[", startLine, startCol);
        case ']': return Token(TokenType::RBRACKET,  "]", startLine, startCol);
        case ';': return Token(TokenType::SEMICOLON, ";", startLine, startCol);
        case ',': return Token(TokenType::COMMA,     ",", startLine, startCol);
        case ':': return Token(TokenType::COLON,     ":", startLine, startCol);
        case '.': return Token(TokenType::DOT,       ".", startLine, startCol);
        default:  break;
    }

    cerr << "[Lexer Error] Line " << startLine
         << ": Unknown character '" << c << "'\n";
    errors++;
    return Token(TokenType::UNKNOWN, string(1, c), startLine, startCol);
}

// ── Main tokenise loop ───────────────────────────────────────────
vector<Token> Lexer::tokenize() {
    vector<Token> tokens;

    while (true) {
        skipWhitespaceAndComments();
        if (isAtEnd()) break;

        char c = current();

        if (isdigit(c))              tokens.push_back(readNumber());
        else if (isalpha(c) || c == '_') tokens.push_back(readIdentifierOrKeyword());
        else if (c == '"')           tokens.push_back(readString());
        else                         tokens.push_back(readOperator());
    }

    tokens.emplace_back(TokenType::END_OF_FILE, "EOF", line, col);
    return tokens;
}