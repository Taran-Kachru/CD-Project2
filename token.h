#pragma once
#include <string>
#include <unordered_map>
using namespace std;

// ================================================================
//  TOKEN TYPES
//  Covers all terminal symbols produced by the Hindi Lexer.
// ================================================================
enum class TokenType {
    // Literals
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    BOOL_LITERAL,

    // Type Keywords (Hindi)
    KW_INT,       // sankhya   — integer
    KW_FLOAT,     // dasha     — float
    KW_BOOL,      // satya     — boolean
    KW_VOID,      // shunya    — void

    // Control Flow
    KW_IF,        // agar
    KW_ELSE,      // nahi_to
    KW_WHILE,     // jabtak
    KW_FOR,       // baar_baar
    KW_BREAK,     // ruko
    KW_CONTINUE,  // aage_bado

    // Functions
    KW_FUNC,      // kaam
    KW_RETURN,    // wapas

    // I/O
    KW_PRINT,     // dikhao
    KW_INPUT,     // lo

    // Boolean literals
    KW_TRUE,      // sach
    KW_FALSE,     // jhooth

    // Arithmetic Operators
    OP_PLUS, OP_MINUS, OP_MUL, OP_DIV, OP_MOD,

    // Comparison Operators
    OP_EQ, OP_NEQ, OP_LT, OP_GT, OP_LTE, OP_GTE,

    // Assignment
    OP_ASSIGN,

    // Logical Operators
    OP_AND, OP_OR, OP_NOT,

    // Compound Assignment
    OP_PLUS_EQ, OP_MINUS_EQ, OP_MUL_EQ, OP_DIV_EQ,

    // Delimiters
    LPAREN, RPAREN, LBRACE, RBRACE,
    LBRACKET, RBRACKET,
    SEMICOLON, COMMA, COLON, DOT,

    // Meta
    IDENTIFIER,
    END_OF_FILE,
    UNKNOWN
};

// ── Human-readable names (used in dumps & error messages) ───────
inline string tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::INTEGER_LITERAL: return "INTEGER";
        case TokenType::FLOAT_LITERAL:   return "FLOAT";
        case TokenType::STRING_LITERAL:  return "STRING";
        case TokenType::BOOL_LITERAL:    return "BOOL";
        case TokenType::KW_INT:          return "KW_INT";
        case TokenType::KW_FLOAT:        return "KW_FLOAT";
        case TokenType::KW_BOOL:         return "KW_BOOL";
        case TokenType::KW_VOID:         return "KW_VOID";
        case TokenType::KW_IF:           return "KW_IF";
        case TokenType::KW_ELSE:         return "KW_ELSE";
        case TokenType::KW_WHILE:        return "KW_WHILE";
        case TokenType::KW_FOR:          return "KW_FOR";
        case TokenType::KW_BREAK:        return "KW_BREAK";
        case TokenType::KW_CONTINUE:     return "KW_CONTINUE";
        case TokenType::KW_FUNC:         return "KW_FUNC";
        case TokenType::KW_RETURN:       return "KW_RETURN";
        case TokenType::KW_PRINT:        return "KW_PRINT";
        case TokenType::KW_INPUT:        return "KW_INPUT";
        case TokenType::KW_TRUE:         return "KW_TRUE";
        case TokenType::KW_FALSE:        return "KW_FALSE";
        case TokenType::OP_PLUS:         return "OP_PLUS";
        case TokenType::OP_MINUS:        return "OP_MINUS";
        case TokenType::OP_MUL:          return "OP_MUL";
        case TokenType::OP_DIV:          return "OP_DIV";
        case TokenType::OP_MOD:          return "OP_MOD";
        case TokenType::OP_EQ:           return "OP_EQ";
        case TokenType::OP_NEQ:          return "OP_NEQ";
        case TokenType::OP_LT:           return "OP_LT";
        case TokenType::OP_GT:           return "OP_GT";
        case TokenType::OP_LTE:          return "OP_LTE";
        case TokenType::OP_GTE:          return "OP_GTE";
        case TokenType::OP_ASSIGN:       return "OP_ASSIGN";
        case TokenType::OP_AND:          return "OP_AND";
        case TokenType::OP_OR:           return "OP_OR";
        case TokenType::OP_NOT:          return "OP_NOT";
        case TokenType::OP_PLUS_EQ:      return "OP_PLUS_EQ";
        case TokenType::OP_MINUS_EQ:     return "OP_MINUS_EQ";
        case TokenType::OP_MUL_EQ:       return "OP_MUL_EQ";
        case TokenType::OP_DIV_EQ:       return "OP_DIV_EQ";
        case TokenType::LPAREN:          return "LPAREN";
        case TokenType::RPAREN:          return "RPAREN";
        case TokenType::LBRACE:          return "LBRACE";
        case TokenType::RBRACE:          return "RBRACE";
        case TokenType::SEMICOLON:       return "SEMICOLON";
        case TokenType::COMMA:           return "COMMA";
        case TokenType::IDENTIFIER:      return "IDENTIFIER";
        case TokenType::END_OF_FILE:     return "EOF";
        default:                         return "UNKNOWN";
    }
}

// ── Token struct ─────────────────────────────────────────────────
struct Token {
    TokenType type;
    string    value;
    int       line;
    int       col;

    Token(TokenType t, string v, int l = 0, int c = 0)
        : type(t), value(move(v)), line(l), col(c) {}
};