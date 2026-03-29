#include "ir.h"
#include <iostream>
#include <set>
#include <sstream>

// ── IRInstruction::toString ──────────────────────────────────────
string IRInstruction::toString() const {
    if (opcode == "LABEL")     return opcode + " " + result + ":";
    if (opcode == "GOTO")      return "    GOTO " + op1;
    if (opcode == "IF_GOTO")   return "    IF " + op1 + " GOTO " + op2;
    if (opcode == "PRINT")     return "    PRINT " + op1;
    if (opcode == "RETURN")    return "    RETURN " + op1;
    if (opcode == "FUNC_BEGIN")return "\nFUNC_BEGIN " + result;
    if (opcode == "FUNC_END")  return "FUNC_END " + result;
    if (opcode == "PARAM")     return "    PARAM " + op1;
    if (opcode == "CALL")      return "    " + result + " = CALL " + op1 + " " + op2;
    if (opcode == "NOT")       return "    " + result + " = NOT " + op1;
    if (opcode == "NEG")       return "    " + result + " = NEG " + op1;
    if (!op2.empty())
        return "    " + result + " = " + op1 + " " + opcode + " " + op2;
    if (!op1.empty())
        return "    " + result + " = " + op1;
    return "    " + result;
}

// ── Constructor ──────────────────────────────────────────────────
IRGenerator::IRGenerator() : tempCount(0), labelCount(0) {}

string IRGenerator::newTemp()  { return "t" + to_string(tempCount++); }
string IRGenerator::newLabel() { return "L" + to_string(labelCount++); }

void IRGenerator::emit(const IRInstruction& instr) {
    code.push_back(instr);
}

// ── Entry point ───────────────────────────────────────────────────
vector<IRInstruction> IRGenerator::generate(ProgramNode* program) {
    code.clear();
    tempCount = labelCount = 0;
    for (auto& decl : program->declarations)
        genStmt(decl.get());
    return code;
}

// ── Statement dispatcher ──────────────────────────────────────────
void IRGenerator::genStmt(ASTNode* node) {
    if (auto* n = dynamic_cast<VarDeclNode*>  (node)) { genVarDecl(n);  return; }
    if (auto* n = dynamic_cast<AssignNode*>   (node)) { genAssign(n);   return; }
    if (auto* n = dynamic_cast<IfNode*>       (node)) { genIf(n);       return; }
    if (auto* n = dynamic_cast<WhileNode*>    (node)) { genWhile(n);    return; }
    if (auto* n = dynamic_cast<ForNode*>      (node)) { genFor(n);      return; }
    if (auto* n = dynamic_cast<ReturnNode*>   (node)) { genReturn(n);   return; }
    if (auto* n = dynamic_cast<PrintNode*>    (node)) { genPrint(n);    return; }
    if (auto* n = dynamic_cast<BlockNode*>    (node)) { genBlock(n);    return; }
    if (auto* n = dynamic_cast<FuncDeclNode*> (node)) { genFuncDecl(n); return; }
    genExpr(node); // expression statement
}

// ── VarDecl ───────────────────────────────────────────────────────
void IRGenerator::genVarDecl(VarDeclNode* node) {
    if (node->initializer) {
        string val = genExpr(node->initializer.get());
        emit({"ASSIGN", node->varName, val});
        constTable[node->varName] = val;
    } else {
        emit({"DECLARE", node->varName, "0"});
        constTable[node->varName] = "0";
    }
}

// ── Assign ────────────────────────────────────────────────────────
void IRGenerator::genAssign(AssignNode* node) {
    string rhs = genExpr(node->value.get());

    if (node->op == "=") {
        emit({"ASSIGN", node->varName, rhs});
        constTable[node->varName] = rhs;
        return;
    }

    // Compound assignment: x += y  →  x = x + y
    string opcode;
    if      (node->op == "+=") opcode = "+";
    else if (node->op == "-=") opcode = "-";
    else if (node->op == "*=") opcode = "*";
    else if (node->op == "/=") opcode = "/";

    string tmp = newTemp();
    emit({opcode, tmp, node->varName, rhs});
    emit({"ASSIGN", node->varName, tmp});
    constTable.erase(node->varName);
}

// ── If  ──────────────────────────────────────────────────────────
void IRGenerator::genIf(IfNode* node) {
    string cond   = genExpr(node->condition.get());
    string elseL  = newLabel();
    string endL   = newLabel();

    emit({"IF_GOTO", "", cond, elseL});   // if cond is false goto else
    genStmt(node->thenBranch.get());
    emit({"GOTO",   "",  endL});
    emit({"LABEL",  elseL});
    if (node->elseBranch) genStmt(node->elseBranch.get());
    emit({"LABEL",  endL});
}

// ── While ─────────────────────────────────────────────────────────
void IRGenerator::genWhile(WhileNode* node) {
    string startL = newLabel();
    string endL   = newLabel();

    emit({"LABEL",   startL});
    string cond = genExpr(node->condition.get());
    emit({"IF_GOTO", "", cond, endL});
    genStmt(node->body.get());
    emit({"GOTO",    "", startL});
    emit({"LABEL",   endL});
}

// ── For ───────────────────────────────────────────────────────────
void IRGenerator::genFor(ForNode* node) {
    string startL = newLabel();
    string endL   = newLabel();

    if (node->init)      genStmt(node->init.get());
    emit({"LABEL",   startL});
    if (node->condition) {
        string cond = genExpr(node->condition.get());
        emit({"IF_GOTO", "", cond, endL});
    }
    if (node->body)      genStmt(node->body.get());
    if (node->update)    genExpr(node->update.get());
    emit({"GOTO",    "", startL});
    emit({"LABEL",   endL});
}

// ── Return ────────────────────────────────────────────────────────
void IRGenerator::genReturn(ReturnNode* node) {
    string val = node->value ? genExpr(node->value.get()) : "0";
    emit({"RETURN", "", val});
}

// ── Print ─────────────────────────────────────────────────────────
void IRGenerator::genPrint(PrintNode* node) {
    string val = genExpr(node->expr.get());
    emit({"PRINT", "", val});
}

// ── Block ─────────────────────────────────────────────────────────
void IRGenerator::genBlock(BlockNode* node) {
    for (auto& stmt : node->stmts) genStmt(stmt.get());
}

// ── FuncDecl ─────────────────────────────────────────────────────
void IRGenerator::genFuncDecl(FuncDeclNode* node) {
    emit({"FUNC_BEGIN", node->name});
    for (auto& [pType, pName] : node->params)
        emit({"PARAM", pName});
    if (node->body) genBlock(dynamic_cast<BlockNode*>(node->body.get()));
    emit({"FUNC_END", node->name});
}

// ================================================================
//  EXPRESSION CODE GENERATION
// ================================================================
string IRGenerator::genExpr(ASTNode* node) {
    if (auto* n = dynamic_cast<NumberLiteralNode*>(node)) return n->value;
    if (auto* n = dynamic_cast<StringLiteralNode*>(node)) return "\"" + n->value + "\"";
    if (auto* n = dynamic_cast<BoolLiteralNode*>  (node)) return n->value ? "1" : "0";
    if (auto* n = dynamic_cast<IdentifierNode*>   (node)) return n->name;
    if (auto* n = dynamic_cast<BinaryOpNode*>     (node)) return genBinaryOp(n);
    if (auto* n = dynamic_cast<UnaryOpNode*>      (node)) return genUnaryOp(n);
    if (auto* n = dynamic_cast<FuncCallNode*>     (node)) return genFuncCall(n);
    return "0";
}

string IRGenerator::genBinaryOp(BinaryOpNode* node) {
    string l = genExpr(node->left.get());
    string r = genExpr(node->right.get());
    string t = newTemp();
    emit({node->op, t, l, r});
    return t;
}

string IRGenerator::genUnaryOp(UnaryOpNode* node) {
    string operand = genExpr(node->operand.get());
    string t = newTemp();
    if (node->op == "!") emit({"NOT", t, operand});
    else                 emit({"NEG", t, operand});
    return t;
}

string IRGenerator::genFuncCall(FuncCallNode* node) {
    // Push args in reverse
    for (int i = (int)node->args.size() - 1; i >= 0; i--) {
        string arg = genExpr(node->args[i].get());
        emit({"PUSH_ARG", "", arg});
    }
    string t = newTemp();
    emit({"CALL", t, node->name, to_string(node->args.size())});
    return t;
}

// ================================================================
//  OPTIMISATION PASSES
// ================================================================

// Pass 1: Constant folding
//   t0 = 10 + 20   →   t0 = 30
vector<IRInstruction> IRGenerator::optimise(const vector<IRInstruction>& ir) {
    vector<IRInstruction> out;
    unordered_map<string, int> vals;

    auto isInt = [](const string& s) {
        if (s.empty()) return false;
        size_t i = (s[0] == '-') ? 1 : 0;
        for (; i < s.size(); i++) if (!isdigit(s[i])) return false;
        return true;
    };

    for (const auto& instr : ir) {
        // Constant propagation: look up known values
        string o1 = (vals.count(instr.op1)) ? to_string(vals[instr.op1]) : instr.op1;
        string o2 = (vals.count(instr.op2)) ? to_string(vals[instr.op2]) : instr.op2;

        // Fold arithmetic if both operands are literal ints
        if (isInt(o1) && isInt(o2)) {
            int a = stoi(o1), b = stoi(o2);
            int result = 0;
            bool folded = true;
            if      (instr.opcode == "+")  result = a + b;
            else if (instr.opcode == "-")  result = a - b;
            else if (instr.opcode == "*")  result = a * b;
            else if (instr.opcode == "/")  result = (b != 0) ? a / b : 0;
            else if (instr.opcode == "%")  result = (b != 0) ? a % b : 0;
            else folded = false;

            if (folded) {
                string r = to_string(result);
                out.push_back({"ASSIGN", instr.result, r});
                vals[instr.result] = result;
                continue;
            }
        }
        if (instr.opcode == "ASSIGN" && isInt(o1))
            vals[instr.result] = stoi(o1);
        out.push_back({instr.opcode, instr.result, o1, o2});
    }
    return out;
}

// Pass 2: Dead-code elimination
//   Remove any ASSIGN whose result is never used afterwards
vector<IRInstruction> IRGenerator::eliminateDeadCode(const vector<IRInstruction>& ir) {
    // Collect all operands that are actually read
    set<string> used;
    for (const auto& instr : ir) {
        if (!instr.op1.empty())    used.insert(instr.op1);
        if (!instr.op2.empty())    used.insert(instr.op2);
        if (instr.opcode == "PRINT"  || instr.opcode == "RETURN" ||
            instr.opcode == "IF_GOTO"|| instr.opcode == "PUSH_ARG")
            used.insert(instr.op1);
    }

    vector<IRInstruction> out;
    for (const auto& instr : ir) {
        // Drop a pure ASSIGN whose result nobody reads (and isn't a named variable)
        if (instr.opcode == "ASSIGN") {
            bool isTemp = (!instr.result.empty() && instr.result[0] == 't');
            if (isTemp && !used.count(instr.result)) continue; // dead
        }
        out.push_back(instr);
    }
    return out;
}