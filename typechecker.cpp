#include "language.h"
#include "typechecker.h"
#include <iostream>

// ── Constructor ──────────────────────────────────────────────────
TypeChecker::TypeChecker()
    : currentFunctionReturnType("void"), errors(0), warnings(0) {}

// ── Scope management ─────────────────────────────────────────────
void TypeChecker::enterScope() { scopes.push_back({}); }
void TypeChecker::exitScope()  { if (!scopes.empty()) scopes.pop_back(); }

void TypeChecker::declare(const string& name, const string& type,
                          int line, bool isFunc) {
    if (scopes.empty()) return;
    auto& top = scopes.back();
    if (top.count(name)) {
        typeError(line, "Variable '" + name + "' already declared in this scope");
        return;
    }
    top[name] = {type, line, isFunc};
}

SymbolInfo* TypeChecker::lookup(const string& name) {
    for (int i = (int)scopes.size() - 1; i >= 0; i--) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) return &it->second;
    }
    return nullptr;
}

// ── Error / warning reporting ─────────────────────────────────────
void TypeChecker::typeError(int line, const string& msg) {
    cerr << "[Type Error] Line " << line << ": " << msg << "\n";
    errors++;
}
void TypeChecker::typeWarning(int line, const string& msg) {
    cerr << "[Type Warning] Line " << line << ": " << msg << "\n";
    warnings++;
}

// ── Type utilities ────────────────────────────────────────────────
string TypeChecker::mapKeywordToType(const string& kw) {
    if (kw == KW_INT_WORD)   return "int";
    if (kw == KW_FLOAT_WORD) return "float";
    if (kw == KW_BOOL_WORD)  return "bool";
    if (kw == KW_VOID_WORD)  return "void";
    return kw;
}

bool TypeChecker::isNumeric(const string& t) const {
    return t == "int" || t == "float";
}

bool TypeChecker::typesCompatible(const string& a, const string& b) const {
    if (a == "error" || b == "error") return true; // suppress cascading
    if (a == b) return true;
    if (isNumeric(a) && isNumeric(b)) return true;  // int ↔ float ok
    return false;
}

string TypeChecker::promoteTypes(const string& a, const string& b) const {
    if (a == "float" || b == "float") return "float";
    return a;
}

// ── Entry point ───────────────────────────────────────────────────
void TypeChecker::check(ProgramNode* program) {
    enterScope();
    for (auto& decl : program->declarations)
        checkStmt(decl.get());
    exitScope();
}

// ── Statement dispatcher ──────────────────────────────────────────
void TypeChecker::checkStmt(ASTNode* node) {
    if (auto* n = dynamic_cast<VarDeclNode*>  (node)) { checkVarDecl(n); return; }
    if (auto* n = dynamic_cast<AssignNode*>   (node)) { checkAssign(n);  return; }
    if (auto* n = dynamic_cast<IfNode*>       (node)) { checkIf(n);      return; }
    if (auto* n = dynamic_cast<WhileNode*>    (node)) { checkWhile(n);   return; }
    if (auto* n = dynamic_cast<ForNode*>      (node)) { checkFor(n);     return; }
    if (auto* n = dynamic_cast<ReturnNode*>   (node)) { checkReturn(n);  return; }
    if (auto* n = dynamic_cast<PrintNode*>    (node)) { checkPrint(n);   return; }
    if (auto* n = dynamic_cast<BlockNode*>    (node)) { checkBlock(n);   return; }
    if (auto* n = dynamic_cast<FuncDeclNode*> (node)) { checkFuncDecl(n);return; }
    // expression statements — just infer type for side-effect check
    inferType(node);
}

// ── VarDecl ───────────────────────────────────────────────────────
void TypeChecker::checkVarDecl(VarDeclNode* node) {
    string declaredType = mapKeywordToType(node->typeName);
    declare(node->varName, declaredType, node->line);

    if (node->initializer) {
        string initType = inferType(node->initializer.get());
        if (!typesCompatible(declaredType, initType))
            typeError(node->line,
                "Cannot assign " + initType +
                " to variable '" + node->varName +
                "' of type " + declaredType);
    }
}

// ── Assign ────────────────────────────────────────────────────────
void TypeChecker::checkAssign(AssignNode* node) {
    SymbolInfo* sym = lookup(node->varName);
    if (!sym) {
        typeError(node->line, "Undeclared variable '" + node->varName + "'");
        return;
    }
    string rhsType = inferType(node->value.get());
    if (!typesCompatible(sym->type, rhsType))
        typeError(node->line,
            "Cannot assign " + rhsType +
            " to '" + node->varName + "' (type: " + sym->type + ")");
}

// ── If ────────────────────────────────────────────────────────────
void TypeChecker::checkIf(IfNode* node) {
    string condType = inferType(node->condition.get());
    if (condType != "bool" && condType != "int" && condType != "error")
        typeWarning(node->line,
            "Condition in 'agar' is type '" + condType + "', expected bool");
    checkStmt(node->thenBranch.get());
    if (node->elseBranch) checkStmt(node->elseBranch.get());
}

// ── While ─────────────────────────────────────────────────────────
void TypeChecker::checkWhile(WhileNode* node) {
    string condType = inferType(node->condition.get());
    if (condType != "bool" && condType != "int" && condType != "error")
        typeWarning(node->line,
            "Condition in 'jabtak' is type '" + condType + "', expected bool");
    checkStmt(node->body.get());
}

// ── For ───────────────────────────────────────────────────────────
void TypeChecker::checkFor(ForNode* node) {
    enterScope();
    if (node->init)      checkStmt(node->init.get());
    if (node->condition) inferType(node->condition.get());
    if (node->update)    inferType(node->update.get());
    if (node->body)      checkStmt(node->body.get());
    exitScope();
}

// ── Return ────────────────────────────────────────────────────────
void TypeChecker::checkReturn(ReturnNode* node) {
    string retType = node->value ? inferType(node->value.get()) : "void";
    string expected = mapKeywordToType(currentFunctionReturnType);
    if (!typesCompatible(expected, retType))
        typeError(node->line,
            "Return type mismatch: function expects " +
            expected + " but returning " + retType);
}

// ── Print ─────────────────────────────────────────────────────────
void TypeChecker::checkPrint(PrintNode* node) {
    inferType(node->expr.get()); // just check expression is valid
}

// ── Block ─────────────────────────────────────────────────────────
void TypeChecker::checkBlock(BlockNode* node) {
    enterScope();
    for (auto& stmt : node->stmts) checkStmt(stmt.get());
    exitScope();
}

// ── FuncDecl ─────────────────────────────────────────────────────
void TypeChecker::checkFuncDecl(FuncDeclNode* node) {
    string retType = mapKeywordToType(node->returnType);
    declare(node->name, retType, node->line, true);

    string prevReturn = currentFunctionReturnType;
    currentFunctionReturnType = retType;

    enterScope();
    for (auto& [pType, pName] : node->params)
        declare(pName, mapKeywordToType(pType), node->line);

    if (node->body) checkStmt(node->body.get());
    exitScope();

    currentFunctionReturnType = prevReturn;
}

// ================================================================
//  TYPE INFERENCE
// ================================================================
string TypeChecker::inferType(ASTNode* node) {
    if (!node) return "void";

    if (auto* n = dynamic_cast<NumberLiteralNode*>(node))
        return n->isFloat ? "float" : "int";

    if (dynamic_cast<BoolLiteralNode*>(node))   return "bool";
    if (dynamic_cast<StringLiteralNode*>(node)) return "string";

    if (auto* n = dynamic_cast<IdentifierNode*> (node)) return inferIdentifier(n);
    if (auto* n = dynamic_cast<BinaryOpNode*>   (node)) return inferBinaryOp(n);
    if (auto* n = dynamic_cast<UnaryOpNode*>    (node)) return inferUnaryOp(n);
    if (auto* n = dynamic_cast<FuncCallNode*>   (node)) return inferFuncCall(n);

    return "void";
}

string TypeChecker::inferIdentifier(IdentifierNode* node) {
    SymbolInfo* sym = lookup(node->name);
    if (!sym) {
        typeError(node->line, "Undeclared variable '" + node->name + "'");
        return "error";
    }
    return sym->type;
}

string TypeChecker::inferBinaryOp(BinaryOpNode* node) {
    string lt = inferType(node->left.get());
    string rt = inferType(node->right.get());

    // Comparison operators always return bool
    const string& op = node->op;
    if (op == "==" || op == "!=" || op == "<" ||
        op == ">"  || op == "<=" || op == ">=")
        return "bool";

    if (op == "&&" || op == "||") return "bool";

    if (!typesCompatible(lt, rt)) {
        typeError(node->line,
            "Type mismatch in '" + op +
            "': " + lt + " vs " + rt);
        return "error";
    }
    return promoteTypes(lt, rt);
}

string TypeChecker::inferUnaryOp(UnaryOpNode* node) {
    string t = inferType(node->operand.get());
    if (node->op == "!") return "bool";
    if (node->op == "-") {
        if (!isNumeric(t))
            typeError(node->line, "Unary '-' requires numeric type, got " + t);
        return t;
    }
    return t;
}

string TypeChecker::inferFuncCall(FuncCallNode* node) {
    SymbolInfo* sym = lookup(node->name);
    if (!sym) {
        typeError(node->line, "Undeclared function '" + node->name + "'");
        return "error";
    }
    return sym->type;
}