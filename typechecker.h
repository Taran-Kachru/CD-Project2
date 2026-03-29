#pragma once
#include "ast.h"
#include <unordered_map>
#include <vector>
#include <string>

// ================================================================
//  TYPE CHECKER  (Semantic Analysis — Phase 3)
//
//  Walks the AST and enforces:
//    1. Variables declared before use
//    2. Type consistency in binary operations
//    3. Return type matches function signature
//    4. Function calls match declared parameter count/types
//    5. Scope management (nested blocks)
//
//  Types used internally:
//    "int"    ← sankhya
//    "float"  ← dasha
//    "bool"   ← satya
//    "string" ← string literals
//    "void"   ← shunya
//    "error"  ← propagated on type mismatch (avoids cascading errors)
// ================================================================

struct SymbolInfo {
    string type;
    int    declaredLine;
    bool   isFunction;
};

class TypeChecker {
public:
    TypeChecker();

    // Entry point — pass the root program node
    void check(ProgramNode* program);

    int errorCount()   const { return errors; }
    int warningCount() const { return warnings; }

private:
    // ── Scope stack ───────────────────────────────────────
    vector<unordered_map<string, SymbolInfo>> scopes;
    string currentFunctionReturnType;
    int    errors;
    int    warnings;

    void  enterScope();
    void  exitScope();
    void  declare(const string& name, const string& type,
                  int line, bool isFunc = false);
    SymbolInfo* lookup(const string& name);

    // ── Visitors ──────────────────────────────────────────
    void   checkStmt(ASTNode* node);
    string inferType(ASTNode* node);   // returns type string

    void   checkVarDecl(VarDeclNode*   node);
    void   checkAssign (AssignNode*    node);
    void   checkIf     (IfNode*        node);
    void   checkWhile  (WhileNode*     node);
    void   checkFor    (ForNode*       node);
    void   checkReturn (ReturnNode*    node);
    void   checkPrint  (PrintNode*     node);
    void   checkBlock  (BlockNode*     node);
    void   checkFuncDecl(FuncDeclNode* node);

    string inferBinaryOp (BinaryOpNode*  node);
    string inferUnaryOp  (UnaryOpNode*   node);
    string inferFuncCall (FuncCallNode*  node);
    string inferIdentifier(IdentifierNode* node);

    // ── Type utilities ────────────────────────────────────
    string  mapKeywordToType(const string& kw);
    bool    isNumeric(const string& t)   const;
    bool    typesCompatible(const string& a, const string& b) const;
    string  promoteTypes(const string& a, const string& b) const;

    void    typeError  (int line, const string& msg);
    void    typeWarning(int line, const string& msg);
};