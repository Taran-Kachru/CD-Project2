#pragma once
#include <string>
using namespace std;

// ================================================================
//  LANGUAGE CONFIGURATION
//
//  This is the ONLY file you need to change to switch languages.
//
//  To switch to Devanagari Hindi:
//    Replace the values below with: संख्या, दशांश, सत्य, etc.
//
//  To switch to Tamil:
//    Replace with: எண், தசம், உண்மை, etc.
//
//  To switch to English:
//    Replace with: int, float, bool, etc.
//
//  The rest of the compiler never changes.
// ================================================================

// ── Data Types ───────────────────────────────────────────────────
const string KW_INT_WORD      = "sankhya";    // integer type
const string KW_FLOAT_WORD    = "dasha";      // float type
const string KW_BOOL_WORD     = "satya";      // boolean type
const string KW_VOID_WORD     = "shunya";     // void type

// ── Control Flow ─────────────────────────────────────────────────
const string KW_IF_WORD       = "agar";       // if
const string KW_ELSE_WORD     = "nahi_to";    // else
const string KW_WHILE_WORD    = "jabtak";     // while
const string KW_FOR_WORD      = "baar_baar";  // for
const string KW_BREAK_WORD    = "ruko";       // break
const string KW_CONTINUE_WORD = "aage_bado";  // continue

// ── Functions ────────────────────────────────────────────────────
const string KW_FUNC_WORD     = "kaam";       // function
const string KW_RETURN_WORD   = "wapas";      // return

// ── I/O ──────────────────────────────────────────────────────────
const string KW_PRINT_WORD    = "dikhao";     // print
const string KW_INPUT_WORD    = "lo";         // input

// ── Boolean Literals ─────────────────────────────────────────────
const string KW_TRUE_WORD     = "sach";       // true
const string KW_FALSE_WORD    = "jhooth";     // false