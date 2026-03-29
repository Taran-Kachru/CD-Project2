#pragma once
#include "ast.h"
#include <vector>
#include <string>
#include <unordered_map>

// ================================================================
//  IR INSTRUCTION  (Three-Address Code style)
//
//  Every computation is broken into instructions of the form:
//       result = op1  operator  op2
//
//  Examples:
//       t0 = 10
//       t1 = 20
//       t2 = t0 + t1
//       PRINT t2
//       IF x > 0 GOTO L1
//       LABEL L1
//       GOTO L2
//       LABEL L2
// ================================================================
struct IRInstruction {
    string opcode;   // DECLARE, ASSIGN, ADD, SUB, MUL, DIV, MOD,
                     // EQ, NEQ, LT, GT, LTE, GTE, AND, OR, NOT,
                     // PRINT, IF_GOTO, GOTO, LABEL,
                     // CALL, RETURN, FUNC_BEGIN, FUNC_END, PARAM
    string result;
    string op1;
    string op2;

    IRInstruction(string oc, string r = "", string o1 = "", string o2 = "")
        : opcode(move(oc)), result(move(r)), op1(move(o1)), op2(move(o2)) {}

    // Pretty format for display
    string toString() const;
};

// ================================================================
//  IR GENERATOR
//
//  Walks the AST and emits a flat list of IRInstructions.
//  Generates:
//    • Temporaries:  t0, t1, t2, ...
//    • Labels:       L0, L1, L2, ...
// ================================================================
class IRGenerator {
public:
    IRGenerator();

    vector<IRInstruction> generate(ProgramNode* program);

    // Constant folding optimiser pass
    vector<IRInstruction> optimise(const vector<IRInstruction>& ir);

    // Dead-code elimination pass
    vector<IRInstruction> eliminateDeadCode(const vector<IRInstruction>& ir);

private:
    vector<IRInstruction>          code;
    unordered_map<string, string>  constTable;   // var → value for folding
    int                            tempCount;
    int                            labelCount;

    string newTemp();
    string newLabel();
    void   emit(const IRInstruction& instr);

    // ── Visitors ──────────────────────────────────────────
    string genExpr (ASTNode* node);   // returns operand name
    void   genStmt (ASTNode* node);

    void   genVarDecl  (VarDeclNode*   node);
    void   genAssign   (AssignNode*    node);
    void   genIf       (IfNode*        node);
    void   genWhile    (WhileNode*     node);
    void   genFor      (ForNode*       node);
    void   genReturn   (ReturnNode*    node);
    void   genPrint    (PrintNode*     node);
    void   genBlock    (BlockNode*     node);
    void   genFuncDecl (FuncDeclNode*  node);

    string genBinaryOp (BinaryOpNode*  node);
    string genUnaryOp  (UnaryOpNode*   node);
    string genFuncCall (FuncCallNode*  node);
};