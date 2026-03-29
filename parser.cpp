#include "language.h"
#include "parser.h"
#include <iostream>
#include <stdexcept>

// ── Constructor ──────────────────────────────────────────────────
Parser::Parser(vector<Token> toks)
    : tokens(move(toks)), pos(0), errors(0) {}

// ── Token stream helpers ─────────────────────────────────────────
Token& Parser::current() { return tokens[pos]; }

Token& Parser::peek(int offset) {
    size_t p = pos + offset;
    return p < tokens.size() ? tokens[p] : tokens.back();
}

Token Parser::advance() {
    Token t = tokens[pos];
    if (pos + 1 < tokens.size()) pos++;
    return t;
}

bool Parser::check(TokenType t) const { return tokens[pos].type == t; }

bool Parser::match(TokenType t) {
    if (check(t)) { advance(); return true; }
    return false;
}

Token Parser::expect(TokenType t, const string& msg) {
    if (check(t)) return advance();
    cerr << "[Parse Error] Line " << current().line
         << ": Expected " << msg
         << " but got '" << current().value << "'\n";
    errors++;
    return current();
}

bool Parser::isAtEnd() const {
    return tokens[pos].type == TokenType::END_OF_FILE;
}

bool Parser::isTypeKeyword(TokenType t) const {
    return t == TokenType::KW_INT   ||
           t == TokenType::KW_FLOAT ||
           t == TokenType::KW_BOOL  ||
           t == TokenType::KW_VOID;
}

string Parser::typeNameFromToken(TokenType t) {
    switch (t) {
        case TokenType::KW_INT:   return KW_INT_WORD;
        case TokenType::KW_FLOAT: return KW_FLOAT_WORD;
        case TokenType::KW_BOOL:  return KW_BOOL_WORD;
        case TokenType::KW_VOID:  return KW_VOID_WORD;
        default:                  return "unknown";
    }
}

// panic-mode: skip to next semicolon or brace to recover
void Parser::syncError() {
    while (!isAtEnd()) {
        if (check(TokenType::SEMICOLON)) { advance(); return; }
        if (check(TokenType::RBRACE))    return;
        advance();
    }
}

// ── Root ─────────────────────────────────────────────────────────
unique_ptr<ProgramNode> Parser::parse() {
    auto program = make_unique<ProgramNode>();
    while (!isAtEnd()) {
        try {
            program->declarations.push_back(parseStatement());
        } catch (...) {
            errors++;
            syncError();
        }
    }
    return program;
}

// ── Statement dispatch ────────────────────────────────────────────
NodePtr Parser::parseStatement() {
    TokenType t = current().type;

    if (isTypeKeyword(t))           return parseVarDecl(typeNameFromToken(advance().type));
    if (t == TokenType::KW_IF)      return parseIfStmt();
    if (t == TokenType::KW_WHILE)   return parseWhileStmt();
    if (t == TokenType::KW_FOR)     return parseForStmt();
    if (t == TokenType::KW_RETURN)  return parseReturnStmt();
    if (t == TokenType::KW_PRINT)   return parsePrintStmt();
    if (t == TokenType::KW_FUNC)    return parseFuncDecl();
    if (t == TokenType::LBRACE)     return parseBlock();

    return parseExprStatement();
}

// ── Variable declaration: sankhya x = expr ; ─────────────────────
NodePtr Parser::parseVarDecl(const string& typeName) {
    int ln = current().line;
    string varName = expect(TokenType::IDENTIFIER, "variable name").value;

    NodePtr init = nullptr;
    if (match(TokenType::OP_ASSIGN))
        init = parseExpression();

    expect(TokenType::SEMICOLON, "';' after variable declaration");
    return make_unique<VarDeclNode>(typeName, varName, move(init), ln);
}

// ── If statement ─────────────────────────────────────────────────
NodePtr Parser::parseIfStmt() {
    int ln = current().line;
    advance(); // consume 'agar'
    expect(TokenType::LPAREN, "'(' after agar");
    NodePtr cond = parseExpression();
    expect(TokenType::RPAREN, "')' after condition");
    NodePtr thenB = parseBlock();
    NodePtr elseB = nullptr;
    if (match(TokenType::KW_ELSE))
        elseB = parseBlock();
    return make_unique<IfNode>(move(cond), move(thenB), move(elseB), ln);
}

// ── While statement ──────────────────────────────────────────────
NodePtr Parser::parseWhileStmt() {
    int ln = current().line;
    advance(); // consume 'jabtak'
    expect(TokenType::LPAREN, "'(' after jabtak");
    NodePtr cond = parseExpression();
    expect(TokenType::RPAREN, "')' after condition");
    NodePtr body = parseBlock();
    return make_unique<WhileNode>(move(cond), move(body), ln);
}

// ── For statement ────────────────────────────────────────────────
NodePtr Parser::parseForStmt() {
    int ln = current().line;
    advance(); // consume 'baar_baar'
    expect(TokenType::LPAREN, "'(' after baar_baar");

    NodePtr init = nullptr;
    if (!check(TokenType::SEMICOLON))
        init = parseStatement();
    else
        advance();

    NodePtr cond = nullptr;
    if (!check(TokenType::SEMICOLON))
        cond = parseExpression();
    expect(TokenType::SEMICOLON, "';' in for loop");

    NodePtr update = nullptr;
    if (!check(TokenType::RPAREN)) {
        if (check(TokenType::IDENTIFIER)) {
            string name = current().value;
            TokenType next = peek().type;
            if (next == TokenType::OP_ASSIGN   || next == TokenType::OP_PLUS_EQ  ||
                next == TokenType::OP_MINUS_EQ || next == TokenType::OP_MUL_EQ   ||
                next == TokenType::OP_DIV_EQ) {
                int ln2 = current().line;
                advance();
                string op = current().value; advance();
                NodePtr val = parseExpression();
                update = make_unique<AssignNode>(name, op, move(val), ln2);
            } else {
                update = parseExpression();
            }
        } else {
            update = parseExpression();
        }
    }
    expect(TokenType::RPAREN, "')' after for clauses");

    NodePtr body = parseBlock();
    return make_unique<ForNode>(move(init), move(cond), move(update), move(body), ln);
}

// ── Return statement ─────────────────────────────────────────────
NodePtr Parser::parseReturnStmt() {
    int ln = current().line;
    advance(); // consume 'wapas'
    NodePtr val = nullptr;
    if (!check(TokenType::SEMICOLON))
        val = parseExpression();
    expect(TokenType::SEMICOLON, "';' after wapas");
    return make_unique<ReturnNode>(move(val), ln);
}

// ── Print statement: dikhao ( expr ) ; ───────────────────────────
NodePtr Parser::parsePrintStmt() {
    int ln = current().line;
    advance(); // consume 'dikhao'
    expect(TokenType::LPAREN, "'(' after dikhao");
    NodePtr expr = parseExpression();
    expect(TokenType::RPAREN, "')' after dikhao expression");
    expect(TokenType::SEMICOLON, "';' after dikhao");
    return make_unique<PrintNode>(move(expr), ln);
}

// ── Function declaration ─────────────────────────────────────────
NodePtr Parser::parseFuncDecl() {
    int ln = current().line;
    advance(); // consume 'kaam'

    string retType = typeNameFromToken(current().type);
    if (isTypeKeyword(current().type)) advance();
    else retType = "shunya";

    string name = expect(TokenType::IDENTIFIER, "function name").value;
    expect(TokenType::LPAREN, "'(' after function name");

    vector<pair<string,string>> params;
    while (!check(TokenType::RPAREN) && !isAtEnd()) {
        string pType = typeNameFromToken(current().type);
        if (!isTypeKeyword(current().type)) {
            cerr << "[Parse Error] Line " << current().line
                 << ": Expected parameter type\n";
            errors++;
            break;
        }
        advance();
        string pName = expect(TokenType::IDENTIFIER, "parameter name").value;
        params.push_back({pType, pName});
        if (!check(TokenType::RPAREN)) expect(TokenType::COMMA, "','");
    }
    expect(TokenType::RPAREN, "')'");
    NodePtr body = parseBlock();
    return make_unique<FuncDeclNode>(retType, name, move(params), move(body), ln);
}

// ── Block  { stmt* } ─────────────────────────────────────────────
NodePtr Parser::parseBlock() {
    int ln = current().line;
    expect(TokenType::LBRACE, "'{'");
    auto block = make_unique<BlockNode>(ln);
    while (!check(TokenType::RBRACE) && !isAtEnd())
        block->stmts.push_back(parseStatement());
    expect(TokenType::RBRACE, "'}'");
    return block;
}

// ── Expression statement ─────────────────────────────────────────
NodePtr Parser::parseExprStatement() {
    int ln = current().line;
    // Assignment: identifier (= += -= *= /=) expr ;
    if (check(TokenType::IDENTIFIER)) {
        string name = current().value;
        TokenType next = peek().type;
        if (next == TokenType::OP_ASSIGN   || next == TokenType::OP_PLUS_EQ  ||
            next == TokenType::OP_MINUS_EQ || next == TokenType::OP_MUL_EQ   ||
            next == TokenType::OP_DIV_EQ) {
            advance();                    // consume identifier
            string op = current().value;
            advance();                    // consume operator
            NodePtr val = parseExpression();
            expect(TokenType::SEMICOLON, "';'");
            return make_unique<AssignNode>(name, op, move(val), ln);
        }
    }
    NodePtr expr = parseExpression();
    expect(TokenType::SEMICOLON, "';' after expression");
    return expr;
}

// ================================================================
//  EXPRESSION PARSERS  (precedence climbing)
// ================================================================

NodePtr Parser::parseExpression() { return parseOr(); }

NodePtr Parser::parseOr() {
    NodePtr left = parseAnd();
    while (check(TokenType::OP_OR)) {
        int ln = current().line;
        string op = advance().value;
        left = make_unique<BinaryOpNode>(op, move(left), parseAnd(), ln);
    }
    return left;
}

NodePtr Parser::parseAnd() {
    NodePtr left = parseEquality();
    while (check(TokenType::OP_AND)) {
        int ln = current().line;
        string op = advance().value;
        left = make_unique<BinaryOpNode>(op, move(left), parseEquality(), ln);
    }
    return left;
}

NodePtr Parser::parseEquality() {
    NodePtr left = parseComparison();
    while (check(TokenType::OP_EQ) || check(TokenType::OP_NEQ)) {
        int ln = current().line;
        string op = advance().value;
        left = make_unique<BinaryOpNode>(op, move(left), parseComparison(), ln);
    }
    return left;
}

NodePtr Parser::parseComparison() {
    NodePtr left = parseAddSub();
    while (check(TokenType::OP_LT)  || check(TokenType::OP_GT) ||
           check(TokenType::OP_LTE) || check(TokenType::OP_GTE)) {
        int ln = current().line;
        string op = advance().value;
        left = make_unique<BinaryOpNode>(op, move(left), parseAddSub(), ln);
    }
    return left;
}

NodePtr Parser::parseAddSub() {
    NodePtr left = parseMulDiv();
    while (check(TokenType::OP_PLUS) || check(TokenType::OP_MINUS)) {
        int ln = current().line;
        string op = advance().value;
        left = make_unique<BinaryOpNode>(op, move(left), parseMulDiv(), ln);
    }
    return left;
}

NodePtr Parser::parseMulDiv() {
    NodePtr left = parseUnary();
    while (check(TokenType::OP_MUL) || check(TokenType::OP_DIV) || check(TokenType::OP_MOD)) {
        int ln = current().line;
        string op = advance().value;
        left = make_unique<BinaryOpNode>(op, move(left), parseUnary(), ln);
    }
    return left;
}

NodePtr Parser::parseUnary() {
    if (check(TokenType::OP_NOT) || check(TokenType::OP_MINUS)) {
        int ln = current().line;
        string op = advance().value;
        return make_unique<UnaryOpNode>(op, parseUnary(), ln);
    }
    return parsePrimary();
}

NodePtr Parser::parsePrimary() {
    int ln = current().line;

    if (check(TokenType::INTEGER_LITERAL)) {
        string v = advance().value;
        return make_unique<NumberLiteralNode>(v, false, ln);
    }
    if (check(TokenType::FLOAT_LITERAL)) {
        string v = advance().value;
        return make_unique<NumberLiteralNode>(v, true, ln);
    }
    if (check(TokenType::STRING_LITERAL)) {
        string v = advance().value;
        return make_unique<StringLiteralNode>(v, ln);
    }
    if (check(TokenType::KW_TRUE))  { advance(); return make_unique<BoolLiteralNode>(true,  ln); }
    if (check(TokenType::KW_FALSE)) { advance(); return make_unique<BoolLiteralNode>(false, ln); }

    if (check(TokenType::IDENTIFIER)) {
        string name = advance().value;
        // Function call: name ( args )
        if (check(TokenType::LPAREN)) {
            advance();
            vector<NodePtr> args;
            while (!check(TokenType::RPAREN) && !isAtEnd()) {
                args.push_back(parseExpression());
                if (!check(TokenType::RPAREN)) expect(TokenType::COMMA, "','");
            }
            expect(TokenType::RPAREN, "')'");
            return make_unique<FuncCallNode>(name, move(args), ln);
        }
        return make_unique<IdentifierNode>(name, ln);
    }

    if (check(TokenType::LPAREN)) {
        advance();
        NodePtr expr = parseExpression();
        expect(TokenType::RPAREN, "')'");
        return expr;
    }

    cerr << "[Parse Error] Line " << ln
         << ": Unexpected token '" << current().value << "'\n";
    errors++;
    advance();
    return make_unique<NumberLiteralNode>("0", false, ln);
}