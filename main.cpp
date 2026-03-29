#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "typechecker.h"
#include "ir.h"

using namespace std;

// ================================================================
//  TERMINAL COLOURS  (ANSI escape codes)
// ================================================================
#define COL_RESET   "\033[0m"
#define COL_BOLD    "\033[1m"
#define COL_RED     "\033[31m"
#define COL_GREEN   "\033[32m"
#define COL_YELLOW  "\033[33m"
#define COL_BLUE    "\033[34m"
#define COL_MAGENTA "\033[35m"
#define COL_CYAN    "\033[36m"
#define COL_WHITE   "\033[37m"

// ── Banner ────────────────────────────────────────────────────────
void printBanner() {
    cout << COL_CYAN << COL_BOLD;
    cout << "\n";
    cout << "  ==================================================\n";
    cout << "  |    HINDI PROGRAMMING LANGUAGE COMPILER        |\n";
    cout << "  |    Lexer -> Parser -> TypeChecker -> IR       |\n";
    cout << "  ==================================================\n";
    cout << COL_RESET << "\n";
}

// ── Section header ────────────────────────────────────────────────
void section(const string& title) {
    cout << "\n" << COL_BOLD << COL_BLUE;
    cout << "\n--------------------------------------------------\n";
    cout << "  " << title << "\n";
    cout << "--------------------------------------------------\n";
    cout << COL_RESET;
}

// ── Read file ─────────────────────────────────────────────────────
string readFile(const string& path) {
    ifstream file(path);
    if (!file) {
        cerr << COL_RED << "[Error] Cannot open file: " << path
             << COL_RESET << "\n";
        exit(1);
    }
    stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// ── Write IR to file ──────────────────────────────────────────────
void writeIRFile(const vector<IRInstruction>& ir, const string& path) {
    ofstream out(path);
    out << "; ================================================\n";
    out << ";  Hindi Compiler - Generated Three-Address IR\n";
    out << "; ================================================\n\n";
    for (const auto& instr : ir)
        out << instr.toString() << "\n";
    out << "\n; -- END OF IR --\n";
    cout << COL_GREEN << "  IR written to: " << path << COL_RESET << "\n";
}

// ================================================================
//  MAIN - Compiler Pipeline
// ================================================================
int main(int argc, char* argv[]) {
    system("chcp 65001 > nul");
    printBanner();

    string inputFile  = (argc >= 2) ? argv[1] : "input.hin";
    string irOutFile  = (argc >= 3) ? argv[2] : "output.ir";

    cout << COL_MAGENTA << "  Source file : " << inputFile  << COL_RESET << "\n";
    cout << COL_MAGENTA << "  IR output   : " << irOutFile  << COL_RESET << "\n";

    // ── Read source ──────────────────────────────────────────────
    string source = readFile(inputFile);

    // ================================================================
    //  PHASE 1 - LEXICAL ANALYSIS
    // ================================================================
    section("PHASE 1 - Lexical Analysis  (Lexer)");

    Lexer lexer(source);
    vector<Token> tokens = lexer.tokenize();

    cout << "\n  Token Stream:\n";
    cout << "  " << string(54, '-') << "\n";
    cout << "  " << left
         << setw(5)  << "Line"
         << setw(6)  << "Col"
         << setw(22) << "Type"
         << setw(20) << "Value"
         << "\n";
    cout << "  " << string(54, '-') << "\n";

    for (const auto& tok : tokens) {
        if (tok.type == TokenType::END_OF_FILE) continue;
        string color = COL_WHITE;
        if (tok.type == TokenType::KW_IF    || tok.type == TokenType::KW_ELSE  ||
            tok.type == TokenType::KW_WHILE || tok.type == TokenType::KW_FOR   ||
            tok.type == TokenType::KW_FUNC  || tok.type == TokenType::KW_RETURN)
            color = COL_YELLOW;
        else if (tok.type == TokenType::KW_INT   || tok.type == TokenType::KW_FLOAT ||
                 tok.type == TokenType::KW_BOOL  || tok.type == TokenType::KW_VOID)
            color = COL_CYAN;
        else if (tok.type == TokenType::INTEGER_LITERAL ||
                 tok.type == TokenType::FLOAT_LITERAL)
            color = COL_GREEN;
        else if (tok.type == TokenType::KW_PRINT)
            color = COL_MAGENTA;

        cout << color << "  "
             << setw(5)  << tok.line
             << setw(6)  << tok.col
             << setw(22) << tokenTypeName(tok.type)
             << setw(20) << tok.value
             << COL_RESET << "\n";
    }

    cout << "\n  " << COL_GREEN << "Total tokens: " << tokens.size()
         << COL_RESET << "\n";

    if (lexer.errorCount() > 0) {
        cout << COL_RED << "  Lexer errors: " << lexer.errorCount()
             << " - aborting.\n" << COL_RESET;
        return 1;
    }

    // ================================================================
    //  PHASE 2 - SYNTAX ANALYSIS  (Parsing -> AST)
    // ================================================================
    section("PHASE 2 - Syntax Analysis  (Parser -> AST)");

    Parser parser(tokens);
    auto program = parser.parse();

    cout << "\n  Abstract Syntax Tree:\n\n";
    program->print();

    if (parser.errorCount() > 0) {
        cout << COL_RED << "\n  Parse errors: " << parser.errorCount()
             << " - aborting.\n" << COL_RESET;
        return 1;
    }
    cout << COL_GREEN << "\n  Parse successful - AST built.\n" << COL_RESET;

    // ================================================================
    //  PHASE 3 - SEMANTIC ANALYSIS  (Type Checking)
    // ================================================================
    section("PHASE 3 - Semantic Analysis  (Type Checker)");

    TypeChecker tc;
    tc.check(program.get());

    if (tc.errorCount() > 0) {
        cout << COL_RED << "\n  Type errors: " << tc.errorCount()
             << " - aborting.\n" << COL_RESET;
        return 1;
    }
    if (tc.warningCount() > 0)
        cout << COL_YELLOW << "  Warnings: " << tc.warningCount()
             << COL_RESET << "\n";

    cout << COL_GREEN << "  Type checking passed - all types valid.\n" << COL_RESET;

    // ================================================================
    //  PHASE 4 - IR GENERATION
    // ================================================================
    section("PHASE 4 - IR Generation  (Three-Address Code)");

    IRGenerator irGen;
    auto rawIR = irGen.generate(program.get());

    cout << "\n  Raw IR:\n";
    for (const auto& instr : rawIR)
        cout << COL_WHITE << instr.toString() << COL_RESET << "\n";

    // ================================================================
    //  PHASE 5 - IR OPTIMISATION
    // ================================================================
    section("PHASE 5 - Optimisation Passes");

    cout << "\n  Pass 1: Constant Folding + Propagation\n";
    auto foldedIR = irGen.optimise(rawIR);

    cout << "  Pass 2: Dead Code Elimination\n";
    auto optimisedIR = irGen.eliminateDeadCode(foldedIR);

    cout << "\n  Optimised IR:\n";
    for (const auto& instr : optimisedIR)
        cout << COL_GREEN << instr.toString() << COL_RESET << "\n";

    int removed = (int)rawIR.size() - (int)optimisedIR.size();
    cout << "\n  " << COL_CYAN
         << "Instructions before: " << rawIR.size()
         << "  After: " << optimisedIR.size()
         << "  Removed: " << removed
         << COL_RESET << "\n";

    // ================================================================
    //  PHASE 6 - OUTPUT
    // ================================================================
    section("PHASE 6 - Output");
    writeIRFile(optimisedIR, irOutFile);

    // ── Final summary ────────────────────────────────────────────
    cout << "\n" << COL_BOLD << COL_GREEN;
    cout << "  ==============================\n";
    cout << "   Compilation Successful!\n";
    cout << "  ==============================\n";
    cout << COL_RESET;

    return 0;
}

// g++ -std=c++17 main.cpp lexer.cpp parser.cpp typechecker.cpp ir.cpp -o compiler //