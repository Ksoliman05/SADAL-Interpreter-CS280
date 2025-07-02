/* Implementation of Recursive-Descent Parser
   for the Simple Ada-Like (SADAL) Language
 * parser.cpp
 * Programming Assignment 2
 * Spring 2025
*/
#include <queue>
#include <vector>
#include <map>
#include <algorithm>
#include "parserInterp.h"
#include "val.h"

using namespace std;

// Global variables
string declaredProcName;
map<string, bool> defVar;
map<string, Token> SymTable;       // Tracks variable types
map<string, Value> TempsResults;   // Stores variable values
vector<string> *Ids_List;

using namespace std;

namespace Parser {
	bool pushed_back = false;
	LexItem pushed_token;

	static LexItem GetNextToken(istream& in, int& line) {
		if (pushed_back) {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem & t) {
		if (pushed_back) {
			abort();
		}
		pushed_back = true;
		pushed_token = t;	
	}

}

static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
	++error_count;
	cout << line << ": " << msg << endl;
}

// 3. ProcName ::= IDENT
bool ProcName(istream& in, int& line) {
    LexItem tok;
	tok = Parser::GetNextToken(in, line);
	if (tok != IDENT) {
		return false;
	}
	else return true;
}

//Prog ::= PROCEDURE ProcName IS ProcBody
bool Prog(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != PROCEDURE) {
        ParseError(line, "Incorrect compilation file.");
        return false;
    }


    tok = Parser::GetNextToken(in, line);
    if (tok != IDENT) {
        ParseError(line, "Missing Procedure Name.");
        return false;
    }
    declaredProcName = tok.GetLexeme();
    defVar[declaredProcName] = true;  

    // Check for IS keyword
    tok = Parser::GetNextToken(in, line);
    if (tok != IS) {
        ParseError(line, "Missing IS Keyword");
        return false;
    }

    bool procBodyStatus = ProcBody(in, line);
    if (!procBodyStatus) {
        ParseError(line, "Incorrect Procedure Definition.");
        ParseError(line, "Incorrect Procedure Body");
        return false;
    }

    // Print decl var if no errors
    if (error_count == 0) {
        cout << "Declared Variables:" << endl;
        bool first = true;
        for (const auto& var : defVar) {
            if (!first) cout << ", ";
            cout << var.first;
            first = false;
        }
        cout << endl << endl << "(DONE)" << endl;
    }
    
    return true;
}

// 2. ProcBody ::= DeclPart BEGIN StmtList END ProcName ;
bool ProcBody(istream& in, int& line) {
    LexItem tok;
    
    // 1. Check DeclPart
    if (!DeclPart(in, line)) {
        return false; 
    }
    
    // 2. Check BEGIN
    tok = Parser::GetNextToken(in, line);
    if (tok != BEGIN) {
        ParseError(line, "Missing BEGIN keyword for procedure body");
        return false;
    }
    
    // 3. Check StmtList
    if (!StmtList(in, line)) {
        //ParseError(line, "Syntactic error in statement list");
        return false;    
    }
    
    // 4. Check END
    tok = Parser::GetNextToken(in, line);
    if (tok != END) {
        ParseError(line, "Missing END keyword");
        return false;
    }
    
    tok = Parser::GetNextToken(in, line);
    if (tok != IDENT || tok.GetLexeme() != declaredProcName) {
        ParseError(line, "Procedure name mismatch in closing end identifier.");
        return false;
    }

    return true;
}


// DeclPart ::= DeclStmt { DeclStmt }
bool DeclPart(istream& in, int& line) {
    LexItem tok;
    
    if (!DeclStmt(in, line)) {
        ParseError(line, "Non-recognizable Declaration Part.");
        return false;
    }
    
    tok = Parser::GetNextToken(in, line);
    while (tok != BEGIN && tok!= END) {
        Parser::PushBackToken(tok);
        
        if (!DeclStmt(in, line)) {
            ParseError(line, "Invalid declaration.");
            return false;
        }
        
        tok = Parser::GetNextToken(in, line);
    }
    
    Parser::PushBackToken(tok);
    return true;
}

// 5. DeclStmt ::= IDENT {, IDENT } : [CONSTANT] Type [(Range)] [:= Expr] ;
// 5. DeclStmt ::= IDENT {, IDENT } : Type [:= Expr] ;
// 5. DeclStmt ::= IDENT {, IDENT } : Type [:= Expr] ;
bool DeclStmt(istream& in, int& line) {
    LexItem tok;
    vector<string> identifiers;
    Token varType;
    Value initVal;
    bool hasInit = false;

    // 1. Parse identifier list
    tok = Parser::GetNextToken(in, line);
    if (tok != IDENT) {
        ParseError(line, "Missing identifier in declaration");
        return false;
    }
    identifiers.push_back(tok.GetLexeme());

    // Parse additional identifiers separated by commas
    while (true) {
        tok = Parser::GetNextToken(in, line);
        
        if (tok == STRING) {
            ParseError(line, "Invalid name for an Identifier: " + tok.GetLexeme());
            return false;
        }

        if (tok != COMMA) {
            Parser::PushBackToken(tok);
            break;
        }

        tok = Parser::GetNextToken(in, line);
        if (tok != IDENT) {
            ParseError(line, "Missing identifier after comma");
            return false;
        }
        
        // Check for duplicate identifiers in this declaration
        if (find(identifiers.begin(), identifiers.end(), tok.GetLexeme()) != identifiers.end()) {
            ParseError(line, "Duplicate identifier in declaration: " + tok.GetLexeme());
            return false;
        }
        identifiers.push_back(tok.GetLexeme());
    }

    // 2. Check for colon
    tok = Parser::GetNextToken(in, line);
    if (tok != COLON) {
        ParseError(line, "Missing colon in declaration");
        return false;
    }

    // 3. Parse type (using the existing Type function)
    tok = Parser::GetNextToken(in, line);
    if (tok != INT && tok != FLOAT && tok != BOOL && tok != STRING && tok != CHAR) {
        ParseError(line, "Invalid type specification");
        return false;
    }
    varType = tok.GetToken();

    // 4. Check for initialization
    tok = Parser::GetNextToken(in, line);
    if (tok == ASSOP) {
        hasInit = true;
        if (!Expr(in, line, initVal)) {
            ParseError(line, "Invalid initialization expression");
            return false;
        }

        // Verify type compatibility
        bool typeError = false;
        switch(varType) {
            case INT:
                if (!initVal.IsInt()) typeError = true;
                break;
            case FLOAT:
                if (!initVal.IsReal()) typeError = true;
                break;
            case BOOL:
                if (!initVal.IsBool()) typeError = true;
                break;
            case STRING:
                if (!initVal.IsString()) typeError = true;
                break;
            case CHAR:
                if (!initVal.IsChar()) typeError = true;
                break;
            default:
                typeError = true;
        }

        if (typeError) {
            ParseError(line, "Type mismatch in initialization");
            return false;
        }
    } else {
        Parser::PushBackToken(tok);
    }

    // 5. Check for semicolon
    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line, "Missing semicolon at end of declaration");
        return false;
    }

    // 6. Register variables in symbol table and initialize if needed
    for (const auto& id : identifiers) {
        // Check for previous declaration
        if (SymTable.find(id) != SymTable.end()) {
            ParseError(line, "Variable redefinition: " + id);
            return false;
        }

        // Add to symbol table
        SymTable[id] = varType;
        
        // Initialize if needed
        if (hasInit) {
            TempsResults[id] = initVal;
        }
    }

    return true;
}


// 6. Type ::= INTEGER | FLOAT | BOOLEAN | STRING | CHARACTER
bool Type(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    
    if (tok != INT && tok != FLOAT && tok != BOOL && tok != STRING && tok != CHAR) {
        ParseError(line, "Incorrect Declaration Type.");
        return false;
    }
    return true;
}

// 7. StmtList ::= Stmt { Stmt }
bool StmtList(istream& in, int& line) {
    bool status;
    LexItem tok;
    
    status = Stmt(in, line);
    if (!status) {
        ParseError(line, "Syntactic error in statement list.");
        return false;
    }
    
    tok = Parser::GetNextToken(in, line);
    while (tok != END && tok != ELSIF && tok != ELSE) {
        Parser::PushBackToken(tok);
        
        status = Stmt(in, line);
        if (!status) {
            ParseError(line, "Syntactic error in statement list.");
            return false;
        }
        
        tok = Parser::GetNextToken(in, line);
    }
    
    Parser::PushBackToken(tok);
    return true;
}

// 8. Stmt ::= AssignStmt | PrintStmts | GetStmt | IfStmt
bool Stmt(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    
    //cout << "Current token: " << tok << " at line " << line << endl;  
    
    if (tok == PUT || tok == PUTLN) {
        Parser::PushBackToken(tok);
        return PrintStmts(in, line);  
    }
    else if (tok == IDENT) {
        Parser::PushBackToken(tok);
        return AssignStmt(in, line);  
    }
    else if (tok == GET) {
        Parser::PushBackToken(tok);
        return GetStmt(in, line);     
    }
    else if (tok == IF) {
        Parser::PushBackToken(tok);
        return IfStmt(in, line);      
    }
    
    //cout << "Unrecognized token: " << tok.GetLexeme() << endl;
    Parser::PushBackToken(tok);
    ParseError(line, "Invalid statement: Expected assignment, print, get, or if");
    return false;
}

// 9. PrintStmts ::= (PutLine | Put) ( Expr) ;
bool PrintStmts(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    
    // Check for PUT or PUTLN
    bool isPutln = (tok == PUTLN);
    if (tok != PUT && !isPutln) {
        ParseError(line, "Missing Put or PutLine keyword");
        return false;
    }

    // Check for opening parenthesis
    tok = Parser::GetNextToken(in, line);
    if (tok != LPAREN) {
        ParseError(line, "Missing left parenthesis");
        return false;
    }

    // Evaluate the expression
    Value retVal;
    if (!Expr(in, line, retVal)) {
        ParseError(line, "Invalid expression in print statement");
        return false;
    }

    // Check for closing parenthesis
    tok = Parser::GetNextToken(in, line);
    if (tok != RPAREN) {
        ParseError(line, "Missing right parenthesis");
        return false;
    }

    // Check for semicolon
    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line, "Missing semicolon at end of statement");
        return false;
    }

    // Perform the actual output
    if (isPutln) {
        cout << retVal << endl;  // PUTLN adds newline
    } else {
        cout << retVal;         // PUT doesn't add newline
    }

    return true;
}

// 10. GetStmt := Get (Var) ;

bool GetStmt(istream& in, int& line) {
    LexItem tok;
    LexItem idtok;
    
    // 1. Check for GET keyword
    tok = Parser::GetNextToken(in, line);
    if (tok != GET) {
        ParseError(line, "Missing GET keyword");
        return false;
    }

    // 2. Check for opening parenthesis
    tok = Parser::GetNextToken(in, line);
    if (tok != LPAREN) {
        ParseError(line, "Missing left parenthesis");
        return false;
    }

    // 3. Get the variable to store input
    if (!Var(in, line, idtok)) {
        ParseError(line, "Invalid variable in GET statement");
        return false;
    }
    string varName = idtok.GetLexeme();

    // 4. Check if variable is declared
    if (SymTable.find(varName) == SymTable.end()) {
        ParseError(line, "Undeclared variable: " + varName);
        return false;
    }

    // 5. Check for closing parenthesis
    tok = Parser::GetNextToken(in, line);
    if (tok != RPAREN) {
        ParseError(line, "Missing right parenthesis");
        return false;
    }

    // 6. Check for semicolon
    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line, "Missing semicolon");
        return false;
    }

    // 7. Perform the actual input operation based on variable type
    Token varType = SymTable[varName];
    Value inputVal;
    string inputStr;
    
    try {
        switch(varType) {
            case INT: {
                int i;
                if (!(cin >> i)) {
                    ParseError(line, "Invalid integer input");
                    return false;
                }
                inputVal = Value(i);
                break;
            }
            case FLOAT: {
                double d;
                if (!(cin >> d)) {
                    ParseError(line, "Invalid float input");
                    return false;
                }
                inputVal = Value(d);
                break;
            }
            case BOOL: {
                string boolStr;
                cin >> boolStr;
                // Convert to lowercase for case-insensitive comparison
                transform(boolStr.begin(), boolStr.end(), boolStr.begin(), ::tolower);
                if (boolStr == "true") {
                    inputVal = Value(true);
                } else if (boolStr == "false") {
                    inputVal = Value(false);
                } else {
                    ParseError(line, "Invalid boolean input - must be 'true' or 'false'");
                    return false;
                }
                break;
            }
            case CHAR: {
                char c;
                if (!(cin >> c)) {
                    ParseError(line, "Invalid character input");
                    return false;
                }
                inputVal = Value(c);
                break;
            }
            case STRING: {
                // For strings, we read the entire line (until newline)
                cin.ignore(); // Ignore any remaining whitespace from previous input
                getline(cin, inputStr);
                inputVal = Value(inputStr);
                break;
            }
            default: {
                ParseError(line, "Invalid type for GET operation");
                return false;
            }
        }
    } catch (...) {
        ParseError(line, "Error during input operation");
        return false;
    }
    // 8. Store the input value in TempsResults
    TempsResults[varName] = inputVal;
    return true;
}

// 11. IfStmt ::= IF Expr THEN StmtList { ELSIF Expr THEN StmtList } [ ELSE StmtList ] END IF ;
bool IfStmt(istream& in, int& line) {
    LexItem tok;
    Value condVal;
    bool conditionMet = false;
    
    // Check IF
    tok = Parser::GetNextToken(in, line);
    if (tok != IF) {
        ParseError(line, "Missing IF keyword");
        return false;
    }
    
    // Evaluate IF condition
    if (!Expr(in, line, condVal)) {
        ParseError(line, "Missing or invalid condition after IF");
        return false;
    }
    
    // Verify condition is boolean
    if (!condVal.IsBool()) {
        ParseError(line, "Run-Time Error-IF condition must be boolean");
        return false;
    }
    
    // Check THEN
    tok = Parser::GetNextToken(in, line);
    if (tok != THEN) {
        ParseError(line, "Missing THEN after IF condition");
        return false;
    }
    
    // Execute THEN block if condition is true
    if (condVal.GetBool()) {
        conditionMet = true;
        if (!StmtList(in, line)) {
            ParseError(line, "Invalid statement list in THEN block");
            return false;
        }
    }
    else {
        // Skip THEN block
        while (true) {
            tok = Parser::GetNextToken(in, line);
            if (tok == ELSIF || tok == ELSE || tok == END) {
                Parser::PushBackToken(tok);
                break;
            }
            if (tok == DONE || tok == ERR) {
                ParseError(line, "Unexpected end of IF statement");
                return false;
            }
        }
    }
    
    // Handle optional ELSIF clauses
    tok = Parser::GetNextToken(in, line);
    while (tok == ELSIF && !conditionMet) {
        // Evaluate ELSIF condition
        if (!Expr(in, line, condVal)) {
            ParseError(line, "Missing or invalid condition after ELSIF");
            return false;
        }
        
        // Verify condition is boolean
        if (!condVal.IsBool()) {
            ParseError(line, "Run-Time Error-ELSIF condition must be boolean");
            return false;
        }
        
        // Check THEN
        tok = Parser::GetNextToken(in, line);
        if (tok != THEN) {
            ParseError(line, "Missing THEN after ELSIF condition");
            return false;
        }
        
        // Execute ELSIF block if condition is true and no previous condition was met
        if (condVal.GetBool() && !conditionMet) {
            conditionMet = true;
            if (!StmtList(in, line)) {
                ParseError(line, "Invalid statement list in ELSIF block");
                return false;
            }
        }
        else {
            // Skip ELSIF block
            while (true) {
                tok = Parser::GetNextToken(in, line);
                if (tok == ELSIF || tok == ELSE || tok == END) {
                    Parser::PushBackToken(tok);
                    break;
                }
                if (tok == DONE || tok == ERR) {
                    ParseError(line, "Unexpected end of IF statement");
                    return false;
                }
            }
        }
        
        tok = Parser::GetNextToken(in, line);
    }
    
    // Handle optional ELSE clause if no condition was met
    if (tok == ELSE && !conditionMet) {
        if (!StmtList(in, line)) {
            ParseError(line, "Invalid statement list in ELSE block");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
    }
    else if (tok != END) {
        Parser::PushBackToken(tok);
    }
    
    // Check END IF
    if (tok != END) {
        tok = Parser::GetNextToken(in, line);
    }
    
    if (tok != END) {
        ParseError(line, "Missing END after IF statement");
        return false;
    }
    
    tok = Parser::GetNextToken(in, line);
    if (tok != IF) {
        ParseError(line, "Missing IF after END");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line, "Missing semicolon at end of IF statement");
        return false;
    }
    
    return true;
}
// 12. AssignStmt ::= Var := Expr ;
bool AssignStmt(istream& in, int& line) {
    // 1. Get the target variable
    LexItem idtok;
    if (!Var(in, line, idtok)) {
        ParseError(line, "Invalid assignment target");
        return false;
    }
    string varName = idtok.GetLexeme();

    // 2. Check for assignment operator
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != ASSOP) {
        ParseError(line, "Missing assignment operator :=");
        return false;
    }

    // 3. Evaluate the right-hand expression
    Value rhsVal;
    if (!Expr(in, line, rhsVal)) {
        ParseError(line, "Invalid expression in assignment");
        return false;
    }

    // 4. Verify type compatibility
    Token varType = SymTable[varName];
    bool typeError = false;
    
    switch(varType) {
        case INT:
            if (!rhsVal.IsInt()) typeError = true;
            break;
        case FLOAT:
            if (!rhsVal.IsReal()) typeError = true;
            break;
        case BOOL:
            if (!rhsVal.IsBool()) typeError = true;
            break;
        case STRING:
            if (!rhsVal.IsString()) typeError = true;
            break;
        case CHAR:
            if (!rhsVal.IsChar()) typeError = true;
            break;
        default:
            typeError = true;
    }

    if (typeError) {
        ParseError(line, "Type mismatch in assignment");
        return false;
    }

    // 5. Store the value
    TempsResults[varName] = rhsVal;

    // 6. Check for semicolon
    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line, "Missing semicolon at end of assignment");
        return false;
    }

    return true;
}

// 13. Var ::= IDENT
bool Var(istream& in, int& line, LexItem & idtok) {
    LexItem tok = Parser::GetNextToken(in, line);
    
    if (tok != IDENT) {
        ParseError(line, "Not a valid variable name");
        return false;
    }
    
    // Check if declared (but NOT if initialized)
    if (SymTable.find(tok.GetLexeme()) == SymTable.end()) {
        ParseError(line, "Undeclared variable: " + tok.GetLexeme());
        return false;
    }
    
    idtok = tok; // Return the identifier token
    return true;
}

// 14. Expr ::= Relation {(AND | OR) Relation }
bool Expr(istream& in, int& line, Value & retVal) {
    // Get first Relation
    Value leftVal;
    if (!Relation(in, line, leftVal)) {
        return false;
    }

    LexItem tok;
    while (true) {
        tok = Parser::GetNextToken(in, line);
        
        // Check for logical operators
        if (tok != AND && tok != OR) {
            Parser::PushBackToken(tok);
            break;
        }

        // Verify left operand is boolean
        if (!leftVal.IsBool()) {
            ParseError(line, "Run-Time Error-Left operand of logical operation must be boolean");
            return false;
        }

        // Short-circuit evaluation
        if (tok == AND && leftVal.GetBool() == false) {
            // Skip evaluating right operand for AND with false
            Parser::GetNextToken(in, line); // Dummy read to consume the operator
            Relation(in, line, retVal); // Evaluate but ignore result
            retVal = Value(false);
            return true;
        }
        if (tok == OR && leftVal.GetBool() == true) {
            // Skip evaluating right operand for OR with true
            Parser::GetNextToken(in, line); // Dummy read to consume the operator
            Relation(in, line, retVal); // Evaluate but ignore result
            retVal = Value(true);
            return true;
        }

        // Get right Relation
        Value rightVal;
        if (!Relation(in, line, rightVal)) {
            ParseError(line, "Missing expression after logical operator");
            return false;
        }

        // Verify right operand is boolean
        if (!rightVal.IsBool()) {
            ParseError(line, "Run-Time Error-Right operand of logical operation must be boolean");
            return false;
        }

        // Perform the logical operation
        try {
            if (tok == AND) {
                leftVal = leftVal && rightVal;
            } else {
                leftVal = leftVal || rightVal;
            }
        } catch (...) {
            ParseError(line, "Run-Time Error-Illegal logical operation");
            return false;
        }
    }

    retVal = leftVal;
    return true;
}

// 15. Relation ::= SimpleExpr [ ( = | /= | < | <= | > | >= ) SimpleExpr ]
bool Relation(istream& in, int& line, Value & retVal) {
    // Get left SimpleExpr
    Value leftVal;
    if (!SimpleExpr(in, line, leftVal)) {
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    
    // Check if it's a relational operator
    if (tok != EQ && tok != NEQ && tok != LTHAN && 
        tok != LTE && tok != GTHAN && tok != GTE) {
        // Not a relational operator, so just return the SimpleExpr value
        Parser::PushBackToken(tok);
        retVal = leftVal;
        return true;
    }

    // Get right SimpleExpr
    Value rightVal;
    if (!SimpleExpr(in, line, rightVal)) {
        ParseError(line, "Missing expression after relational operator");
        return false;
    }

    // Perform the comparison
    try {
        switch (tok.GetToken()) {
            case EQ:    retVal = leftVal == rightVal; break;
            case NEQ:   retVal = leftVal != rightVal; break;
            case LTHAN: retVal = leftVal < rightVal; break;
            case LTE:   retVal = leftVal <= rightVal; break;
            case GTHAN: retVal = leftVal > rightVal; break;
            case GTE:   retVal = leftVal >= rightVal; break;
            default:
                ParseError(line, "Invalid relational operator");
                return false;
        }
    } catch (...) {
        ParseError(line, "Run-Time Error-Illegal operand types for comparison");
        return false;
    }

    return true;
}

// 16. SimpleExpr ::= STerm { ( + | - | & ) STerm }
bool SimpleExpr(istream& in, int& line, Value & retVal) {
    // Get first STerm
    Value leftVal;
    if (!STerm(in, line, leftVal)) {
        ParseError(line, "Missing operand");
        return false;
    }
    LexItem tok;
    while (true) {
        tok = Parser::GetNextToken(in, line);   
        // Check for additive operators or concatenation
        if (tok != PLUS && tok != MINUS && tok != CONCAT) {
            Parser::PushBackToken(tok);
            break;
        }
        // Get next STerm
        Value rightVal;
        if (!STerm(in, line, rightVal)) {
            ParseError(line, "Missing operand after operator");
            return false;
        }
        // Perform operation based on token type
        try {
            if (tok == PLUS) {
                leftVal = leftVal + rightVal;
            } 
            else if (tok == MINUS) {
                leftVal = leftVal - rightVal;
            }
            else if (tok == CONCAT) {
                leftVal = leftVal.Concat(rightVal);
            }
        } 
        catch (...) {
            ParseError(line, "Run-Time Error-Illegal operation");
            return false;
        }
    }
    retVal = leftVal;
    return true;
}

// 17. STerm ::= [ ( + | - ) ] Term
bool STerm(istream& in, int& line, Value & retVal) {
    LexItem tok = Parser::GetNextToken(in, line);
    int sign = 1; // Default to positive
    
    // Check for unary + or -
    if (tok == PLUS || tok == MINUS) {
        sign = (tok == PLUS) ? 1 : -1;
    } else {
        Parser::PushBackToken(tok);
    }
    
    // Here's where retVal gets populated:
    if (!Term(in, line, sign, retVal)) { // Term will fill retVal
        ParseError(line, "Missing term after unary sign");
        return false;
    }
    
    // Now we can validate the signed result
    if (sign != 1) { // Only need to check if sign was applied
        if (!retVal.IsInt() && !retVal.IsReal()) {
            ParseError(line, "Run-Time Error-Illegal operand type for sign operation");
            return false;
        }
        
        // Apply sign if needed (values are already signed from Term/Factor/Primary)
        // The sign was propagated all the way down to Primary where it was applied
    }
    
    return true;
}

// 18. Term ::= Factor { ( * | / | MOD ) Factor }
bool Term(istream& in, int& line, int sign, Value & retVal) {
    // Get first Factor (with sign applied)
    Value leftVal;
    if (!Factor(in, line, sign, leftVal)) {
        return false;
    }

    LexItem tok;
    while (true) {
        tok = Parser::GetNextToken(in, line);
        
        // Check for multiplicative operators
        if (tok != MULT && tok != DIV && tok != MOD) {
            Parser::PushBackToken(tok);
            break;
        }

        // Get next Factor (sign is 1 since sign only applies to first term)
        Value rightVal;
        if (!Factor(in, line, 1, rightVal)) {
            ParseError(line, "Missing factor after operator");
            return false;
        }

        // Perform operation based on token type
        try {
            if (tok == MULT) {
                leftVal = leftVal * rightVal;
            } 
            else if (tok == DIV) {
                // Check for division by zero
                if ((rightVal.IsInt() && rightVal.GetInt() == 0) ||
                    (rightVal.IsReal() && rightVal.GetReal() == 0.0)) {
                    ParseError(line, "Run-Time Error-Illegal division by zero");
                    return false;
                }
                leftVal = leftVal / rightVal;
            } 
            else if (tok == MOD) {
                // MOD requires integer operands
                if (!leftVal.IsInt() || !rightVal.IsInt()) {
                    ParseError(line, "Run-Time Error-Illegal operand types for MOD");
                    return false;
                }
                // Check for mod by zero
                if (rightVal.GetInt() == 0) {
                    ParseError(line, "Run-Time Error-Illegal mod by zero");
                    return false;
                }
                leftVal = leftVal % rightVal;
            }
        } catch (...) {
            ParseError(line, "Run-Time Error-Illegal operation");
            return false;
        }
    }

    retVal = leftVal;
    return true;
}

// 19. Factor ::= Primary [** Primary ] | NOT Primary
bool Factor(istream& in, int& line, int sign, Value & retVal){
    LexItem tok = Parser::GetNextToken(in, line);
    // CAse 1 NOT Primary
    if (tok == NOT) {
        Value primVal;
        if (!Primary(in, line, 1, primVal)) {  // NOT ignores incoming sign
            ParseError(line, "Missing primary after NOT");
            return false;
        }
        
        // Type check - NOT requires boolean operand
        if (!primVal.IsBool()) {
            ParseError(line, "Run-Time Error-Illegal operand type for NOT operation");
            return false;
        }
        
        
        try {
            retVal = !primVal;  // Use overloaded ! operator
        } catch (...) {
            ParseError(line, "Run-Time Error-Illegal NOT operation");
            return false;
        }
        return true;
    }        

    // Case 2: Primary [** Primary]    
    Parser::PushBackToken(tok);
    Value baseVal;
    if (!Primary(in, line, sign, baseVal)) {
        ParseError(line, "Missing primary");
        return false;
    }
    
    tok = Parser::GetNextToken(in, line);
    if (tok == EXP) {
        Value expVal;
        LexItem signTok = Parser::GetNextToken(in, line);
        int expSign = 1;
        
        if (signTok == PLUS) {
            expSign = 1;
        } else if (signTok == MINUS) {
            expSign = -1;
        } else {
            Parser::PushBackToken(signTok);
        }
        
        if (!Primary(in, line, expSign, expVal)) {
            ParseError(line, "Missing exponent after **");
            return false;
        }
        
        // Type checking - both operands must be real for exponentiation
        if (!baseVal.IsReal() || !expVal.IsReal()) {
            ParseError(line, "Run-Time Error-Exponentiation requires float operands");
            return false;
        }
        
        try {
            retVal = baseVal.Exp(expVal);  // Use the Exp member function
        } catch (...) {
            ParseError(line, "Run-Time Error-Illegal exponentiation operation");
            return false;
        }
    } 
    else {
        Parser::PushBackToken(tok);
        retVal = baseVal;  // No exponentiation, just return the primary
    }
    
    return true;
}

// 20. Primary ::= Name | ICONST | FCONST | SCONST | BCONST | CCONST | (Expr)
bool Primary(istream& in, int& line, int sign, Value & retVal) {
    LexItem tok = Parser::GetNextToken(in, line);
    
    // Parenthesized expressions
    if (tok == LPAREN) {
        if (!Expr(in, line, retVal)) {
            ParseError(line, "Invalid expression in parentheses");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
        if (tok != RPAREN) {
            ParseError(line, "Missing right parenthesis");
            return false;
        }
        return true;
    }
    // Variables (delegate to Name)
    if (tok == IDENT) {
        Parser::PushBackToken(tok);
        return Name(in, line, sign, retVal);
    }
    // Literals
    if (tok == ICONST) {
        retVal = Value(stoi(tok.GetLexeme()) * sign);
        return true;
    }
    if (tok == FCONST) {
        retVal = Value(stod(tok.GetLexeme()) * sign);
        return true;
    }
    if (tok == SCONST) {
        retVal = Value(tok.GetLexeme());
        return true;
    }
    if (tok == CCONST) {
        retVal = Value(tok.GetLexeme()[0]);
        return true;
    }
    if (tok == BCONST) {
        retVal = Value(tok.GetLexeme() == "true");
        return true;
    }   
    ParseError(line, "Invalid primary expression");
    return false;
}

// 21. Name ::= IDENT [ ( Range ) ]
bool Name(istream& in, int& line, int sign, Value & retVal) {
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != IDENT) {
        ParseError(line, "Expected an identifier");
        return false;
    }
    
    string varName = tok.GetLexeme();
    
    // Check if variable is declared
    if (SymTable.find(varName) == SymTable.end()) {
        ParseError(line, "Undeclared variable: " + varName);
        return false;
    }
    
    // Check if variable is initialized
    if (TempsResults.find(varName) == TempsResults.end()) {
        ParseError(line, "Uninitialized variable: " + varName);
        return false;
    }
    
    retVal = TempsResults[varName];
    
    // Handle indexing if present
    tok = Parser::GetNextToken(in, line);
    if (tok == LPAREN) {
        Value idx1, idx2;
        if (!Range(in, line, idx1, idx2)) {
            ParseError(line, "Invalid array/string index");
            return false;
        }
        
        // Handle string indexing
        if (retVal.IsString()) {
            string str = retVal.GetString();
            int len = str.length();
            int start = idx1.GetInt();
            
            if (idx2.IsInt()) { // Substring access
                int end = idx2.GetInt();
                if (start < 0 || end >= len || start > end) {
                    ParseError(line, "String index out of bounds");
                    return false;
                }
                retVal = Value(str.substr(start, end-start+1));
            } 
            else { // Single character access
                if (start < 0 || start >= len) {
                    ParseError(line, "String index out of bounds");
                    return false;
                }
                retVal = Value(str[start]);
            }
        }
        
        tok = Parser::GetNextToken(in, line);
        if (tok != RPAREN) {
            ParseError(line, "Missing ) after index");
            return false;
        }
    } 
    else {
        Parser::PushBackToken(tok);
    }
    
    return true;
}

// 22. Range ::= SimpleExpr [. . SimpleExpr ]
bool Range(istream& in, int& line, Value & retVal1, Value & retVal2) {
    // Parse first SimpleExpr (start index)
    if (!SimpleExpr(in, line, retVal1)) {
        ParseError(line, "Missing start index in range");
        return false;
    }

    // Check if first value is integer (required for indexing)
    if (!retVal1.IsInt()) {
        ParseError(line, "Range indices must be integers");
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    
    // Check for range operator (..)
    if (tok == DOT) {
        tok = Parser::GetNextToken(in, line);
        if (tok != DOT) {
            ParseError(line, "Missing second dot in range operator");
            return false;
        }

        // Parse second SimpleExpr (end index)
        if (!SimpleExpr(in, line, retVal2)) {
            ParseError(line, "Missing end index in range");
            return false;
        }

        // Check if second value is integer
        if (!retVal2.IsInt()) {
            ParseError(line, "Range indices must be integers");
            return false;
        }

        // Validate range (start <= end)
        if (retVal1.GetInt() > retVal2.GetInt()) {
            ParseError(line, "Invalid range - start index > end index");
            return false;
        }
    } 
    else {
        // Single index case
        Parser::PushBackToken(tok);
        retVal2.SetType(VERR); // Mark as no second value
    }

    return true;
}