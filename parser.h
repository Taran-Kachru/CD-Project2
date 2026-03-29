#pragma once
#include "token.h"
#include "ast.h"
#include <vector>
#include <memory>

// ================================================================
//  PARSER  (Syntax Analysis)
//
//  Phase 2 of the Hindi compiler pipeline.
//  Takes a flat token stream → builds a typed AST.
//
//  Grammar style: Recursive Descent
//
//  Operator precedence (low → high):
//    1.  || (or)
//    2.  && (and)
//    3.  == !=
//    4.  < > <= >=
//    5.  + -
//    6.  * / %
//    7.  unary  ! -
//    8.  primary  ( literals, identifiers, grouped exprs )
// ================================================================
class Parser {
public:
    explicit Parser(vector<Token> tokens);

    unique_ptr<ProgramNode> parse();
    int errorCount() const { return errors; }

private:
    vector<Token> tokens;
    size_t        pos;
    int           errors;

    // ── Token stream helpers ──────────────────────────────
    Token&  current();
    Token&  peek(int offset = 1);
    Token   advance();
    bool    check(TokenType t)  const;
    bool    match(TokenType t);
    Token   expect(TokenType t, const string& msg);
    bool    isAtEnd() const;

    // ── Statement parsers ─────────────────────────────────
    NodePtr parseStatement();
    NodePtr parseVarDecl(const string& typeName);
    NodePtr parseIfStmt();
    NodePtr parseWhileStmt();
    NodePtr parseForStmt();
    NodePtr parseReturnStmt();
    NodePtr parsePrintStmt();
    NodePtr parseFuncDecl();
    NodePtr parseBlock();
    NodePtr parseExprStatement();

    // ── Expression parsers (precedence climbing) ──────────
    NodePtr parseExpression();
    NodePtr parseOr();
    NodePtr parseAnd();
    NodePtr parseEquality();
    NodePtr parseComparison();
    NodePtr parseAddSub();
    NodePtr parseMulDiv();
    NodePtr parseUnary();
    NodePtr parsePrimary();

    // ── Helpers ───────────────────────────────────────────
    string  typeNameFromToken(TokenType t);
    bool    isTypeKeyword(TokenType t) const;
    void    syncError();    // panic-mode error recovery
};