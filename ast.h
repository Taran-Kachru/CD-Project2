#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>
using namespace std;

// ================================================================
//  ABSTRACT SYNTAX TREE  (AST)
//
//  Phase 2 output — produced by the Parser, consumed by the
//  Type Checker and IR Generator.
//
//  Node hierarchy:
//
//    ASTNode  (base)
//    +-- Expressions
//    |   +-- NumberLiteralNode   (42, 3.14)
//    |   +-- BoolLiteralNode     (sach / jhooth)
//    |   +-- StringLiteralNode   ("hello")
//    |   +-- IdentifierNode      (variable name)
//    |   +-- BinaryOpNode        (a + b, x == y, …)
//    |   +-- UnaryOpNode         (!flag, -x)
//    |   \-- FuncCallNode        (kaam foo(a, b))
//    \-- Statements
//        +-- VarDeclNode         (sankhya x = 5)
//        +-- AssignNode          (x = expr)
//        +-- PrintNode           (dikhao expr)
//        +-- IfNode              (agar / nahi_to)
//        +-- WhileNode           (jabtak)
//        +-- ForNode             (baar_baar)
//        +-- ReturnNode          (wapas)
//        +-- BlockNode           ({ ... })
//        +-- FuncDeclNode        (kaam name(...) { ... })
//        \-- ProgramNode         (root)
// ================================================================

// ── Base node ────────────────────────────────────────────────────
struct ASTNode {
    int line = 0;
    virtual ~ASTNode() = default;
    virtual string nodeType() const = 0;
    // Pretty-print with indentation prefix
    virtual void print(const string& prefix = "", bool isLast = true) const = 0;
};

using NodePtr = unique_ptr<ASTNode>;

// ================================================================
//  EXPRESSION NODES
// ================================================================

struct NumberLiteralNode : ASTNode {
    string value;
    bool   isFloat;
    NumberLiteralNode(string v, bool f, int l)
        : value(move(v)), isFloat(f) { line = l; }
    string nodeType() const override { return isFloat ? "FloatLiteral" : "IntLiteral"; }
    void print(const string& prefix, bool isLast) const override {
        cout << prefix << (isLast ? "--- " : "+-- ")
             << nodeType() << " [" << value << "]\n";
    }
};

struct BoolLiteralNode : ASTNode {
    bool value;
    BoolLiteralNode(bool v, int l) : value(v) { line = l; }
    string nodeType() const override { return "BoolLiteral"; }
    void print(const string& prefix, bool isLast) const override {
        cout << prefix << (isLast ? "--- " : "+-- ")
             << "BoolLiteral [" << (value ? "sach" : "jhooth") << "]\n";
    }
};

struct StringLiteralNode : ASTNode {
    string value;
    StringLiteralNode(string v, int l) : value(move(v)) { line = l; }
    string nodeType() const override { return "StringLiteral"; }
    void print(const string& prefix, bool isLast) const override {
        cout << prefix << (isLast ? "--- " : "+-- ")
             << "StringLiteral [\"" << value << "\"]\n";
    }
};

struct IdentifierNode : ASTNode {
    string name;
    IdentifierNode(string n, int l) : name(move(n)) { line = l; }
    string nodeType() const override { return "Identifier"; }
    void print(const string& prefix, bool isLast) const override {
        cout << prefix << (isLast ? "--- " : "+-- ")
             << "Identifier [" << name << "]\n";
    }
};

struct BinaryOpNode : ASTNode {
    string   op;
    NodePtr  left;
    NodePtr  right;
    BinaryOpNode(string o, NodePtr l, NodePtr r, int ln)
        : op(move(o)), left(move(l)), right(move(r)) { line = ln; }
    string nodeType() const override { return "BinaryOp"; }
    void print(const string& prefix, bool isLast) const override {
        cout << prefix << (isLast ? "--- " : "+-- ")
             << "BinaryOp [" << op << "]\n";
        string child = prefix + (isLast ? "    " : "|   ");
        left ->print(child, false);
        right->print(child, true);
    }
};

struct UnaryOpNode : ASTNode {
    string  op;
    NodePtr operand;
    UnaryOpNode(string o, NodePtr operand, int l)
        : op(move(o)), operand(move(operand)) { line = l; }
    string nodeType() const override { return "UnaryOp"; }
    void print(const string& prefix, bool isLast) const override {
        cout << prefix << (isLast ? "--- " : "+-- ")
             << "UnaryOp [" << op << "]\n";
        operand->print(prefix + (isLast ? "    " : "|   "), true);
    }
};

struct FuncCallNode : ASTNode {
    string          name;
    vector<NodePtr> args;
    FuncCallNode(string n, vector<NodePtr> a, int l)
        : name(move(n)), args(move(a)) { line = l; }
    string nodeType() const override { return "FuncCall"; }
    void print(const string& prefix, bool isLast) const override {
        cout << prefix << (isLast ? "--- " : "+-- ")
             << "FuncCall [" << name << "]\n";
        string child = prefix + (isLast ? "    " : "|   ");
        for (size_t i = 0; i < args.size(); i++)
            args[i]->print(child, i == args.size() - 1);
    }
};

// ================================================================
//  STATEMENT NODES
// ================================================================

struct VarDeclNode : ASTNode {
    string  typeName;   // sankhya / dasha / satya
    string  varName;
    NodePtr initializer;
    VarDeclNode(string t, string n, NodePtr init, int l)
        : typeName(move(t)), varName(move(n)), initializer(move(init)) { line = l; }
    string nodeType() const override { return "VarDecl"; }
    void print(const string& prefix, bool isLast) const override {
        cout << prefix << (isLast ? "--- " : "+-- ")
             << "VarDecl [" << typeName << " " << varName << "]\n";
        if (initializer)
            initializer->print(prefix + (isLast ? "    " : "|   "), true);
    }
};

struct AssignNode : ASTNode {
    string  varName;
    string  op;       // = += -= *= /=
    NodePtr value;
    AssignNode(string n, string o, NodePtr v, int l)
        : varName(move(n)), op(move(o)), value(move(v)) { line = l; }
    string nodeType() const override { return "Assign"; }
    void print(const string& prefix, bool isLast) const override {
        cout << prefix << (isLast ? "--- " : "+-- ")
             << "Assign [" << varName << " " << op << "]\n";
        value->print(prefix + (isLast ? "    " : "|   "), true);
    }
};

struct PrintNode : ASTNode {
    NodePtr expr;
    PrintNode(NodePtr e, int l) : expr(move(e)) { line = l; }
    string nodeType() const override { return "Print"; }
    void print(const string& prefix, bool isLast) const override {
        cout << prefix << (isLast ? "--- " : "+-- ") << "Print\n";
        expr->print(prefix + (isLast ? "    " : "|   "), true);
    }
};

struct BlockNode : ASTNode {
    vector<NodePtr> stmts;
    explicit BlockNode(int l) { line = l; }
    string nodeType() const override { return "Block"; }
    void print(const string& prefix, bool isLast) const override {
        cout << prefix << (isLast ? "--- " : "+-- ") << "Block\n";
        string child = prefix + (isLast ? "    " : "|   ");
        for (size_t i = 0; i < stmts.size(); i++)
            stmts[i]->print(child, i == stmts.size() - 1);
    }
};

struct IfNode : ASTNode {
    NodePtr condition;
    NodePtr thenBranch;
    NodePtr elseBranch;   // may be null
    IfNode(NodePtr cond, NodePtr thenB, NodePtr elseB, int l)
        : condition(move(cond)), thenBranch(move(thenB)), elseBranch(move(elseB)) { line = l; }
    string nodeType() const override { return "If"; }
    void print(const string& prefix, bool isLast) const override {
        cout << prefix << (isLast ? "--- " : "+-- ") << "If\n";
        string child = prefix + (isLast ? "    " : "|   ");
        condition ->print(child, !elseBranch && !thenBranch);
        thenBranch->print(child, !elseBranch);
        if (elseBranch) elseBranch->print(child, true);
    }
};

struct WhileNode : ASTNode {
    NodePtr condition;
    NodePtr body;
    WhileNode(NodePtr cond, NodePtr b, int l)
        : condition(move(cond)), body(move(b)) { line = l; }
    string nodeType() const override { return "While"; }
    void print(const string& prefix, bool isLast) const override {
        cout << prefix << (isLast ? "--- " : "+-- ") << "While\n";
        string child = prefix + (isLast ? "    " : "|   ");
        condition->print(child, false);
        body     ->print(child, true);
    }
};

struct ForNode : ASTNode {
    NodePtr init;
    NodePtr condition;
    NodePtr update;
    NodePtr body;
    ForNode(NodePtr i, NodePtr c, NodePtr u, NodePtr b, int l)
        : init(move(i)), condition(move(c)), update(move(u)), body(move(b)) { line = l; }
    string nodeType() const override { return "For"; }
    void print(const string& prefix, bool isLast) const override {
        cout << prefix << (isLast ? "--- " : "+-- ") << "For\n";
        string child = prefix + (isLast ? "    " : "|   ");
        if (init)      init     ->print(child, false);
        if (condition) condition->print(child, false);
        if (update)    update   ->print(child, false);
        if (body)      body     ->print(child, true);
    }
};

struct ReturnNode : ASTNode {
    NodePtr value;
    ReturnNode(NodePtr v, int l) : value(move(v)) { line = l; }
    string nodeType() const override { return "Return"; }
    void print(const string& prefix, bool isLast) const override {
        cout << prefix << (isLast ? "--- " : "+-- ") << "Return\n";
        if (value) value->print(prefix + (isLast ? "    " : "|   "), true);
    }
};

struct FuncDeclNode : ASTNode {
    string          returnType;
    string          name;
    vector<pair<string,string>> params;  // (type, name)
    NodePtr         body;
    FuncDeclNode(string rt, string n,
                 vector<pair<string,string>> p, NodePtr b, int l)
        : returnType(move(rt)), name(move(n)),
          params(move(p)), body(move(b)) { line = l; }
    string nodeType() const override { return "FuncDecl"; }
    void print(const string& prefix, bool isLast) const override {
        cout << prefix << (isLast ? "--- " : "+-- ")
             << "FuncDecl [" << returnType << " " << name << "(";
        for (size_t i = 0; i < params.size(); i++) {
            cout << params[i].first << " " << params[i].second;
            if (i + 1 < params.size()) cout << ", ";
        }
        cout << ")]\n";
        if (body) body->print(prefix + (isLast ? "    " : "|   "), true);
    }
};

struct ProgramNode : ASTNode {
    vector<NodePtr> declarations;
    string nodeType() const override { return "Program"; }
    void print(const string& prefix = "", bool isLast = true) const override {
        cout << "Program\n";
        for (size_t i = 0; i < declarations.size(); i++)
            declarations[i]->print("", i == declarations.size() - 1);
    }
};