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
#include <string>

using namespace std;

// Global variables
string declaredProcName;
map<string, bool> defVar;
map<string, Token> SymTable;       // Tracks variable types
map<string, Value> TempsResults;   // Stores variable values

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
    
    if (error_count == 0) {
        cout << endl << "(DONE)" << endl;
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
    if (tok.GetLexeme() != declaredProcName) {
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

// 5. DeclStmt ::= IDENT {, IDENT } : Type [:= Expr] ;
bool DeclStmt(istream& in, int& line) {
    LexItem tok;
    vector<string> identifiers;
    
    tok = Parser::GetNextToken(in, line);
    if (tok != IDENT) {
        ParseError(line, "Missing identifier in declaration");
        return false;
    }
    identifiers.push_back(tok.GetLexeme());
    
    while (true) {
        tok = Parser::GetNextToken(in, line);
        
        if (tok == STRING) {
            ParseError(line, "Invalid name for an Identifier: (string)");
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
        identifiers.push_back(tok.GetLexeme());
    }
    
    tok = Parser::GetNextToken(in, line);
    if (tok != COLON) {
        ParseError(line, "Missing colon in declaration");
        return false;
    }
    
    tok = Parser::GetNextToken(in, line);
    if (tok != INT && tok != FLOAT && tok != BOOL && tok != STRING && tok != CHAR) {
        ParseError(line, "Invalid type specification");
        return false;
    }
    Token varType;
    varType = tok.GetToken();
    
    tok = Parser::GetNextToken(in, line);
    bool initialized = (tok == ASSOP);
    Value initVal;
    
    if (initialized) {
        if (!Expr(in, line, initVal)) {
            ParseError(line, "Invalid initialization expression");
            return false;
        }
        
        if (varType == INT) {
            if (!initVal.IsInt()) {
                ParseError(line, "INT Type mismatch in DeclStmt");
                return false;
            }
        }
        else if (varType == FLOAT) {
            if (!initVal.IsReal()) {
                ParseError(line, "FLOAT type mismatch in DeclStmt");
                return false;
            }
        }
        else if (varType == BOOL) {
            if (!initVal.IsBool()) {
                ParseError(line, "BOOL Type mismatch in DeclStmt");
                return false;
            }
        }
        else if (varType == STRING) {
            if (!initVal.IsString()) {
                ParseError(line, "STRING Type mismatch in DeclStmt");
                return false;
            }
        }
        else if (varType == CHAR) {
            if (!initVal.IsChar()) {
                ParseError(line, "CHAR Type mismatch in DeclStmt");
                return false;
            }
        }
        
        tok = Parser::GetNextToken(in, line);
    }
    
    if (tok != SEMICOL) {
        ParseError(line, "Missing semicolon at end of declaration");
        return false;
    }
    
    for (int i = 0; i < identifiers.size(); i++) {
        if (SymTable.find(identifiers[i]) != SymTable.end()) {
            ParseError(line, "Variable redefinition: " + identifiers[i]);
            return false;
        }
        
        SymTable[identifiers[i]] = varType;
        
        if (initialized) {
            TempsResults[identifiers[i]] = initVal;
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
    
    Parser::PushBackToken(tok);
    ParseError(line, "Invalid statement: Expected assignment, print, get, or if");
    return false;
}

// 9. PrintStmts ::= (PutLine | Put) ( Expr) ;
bool PrintStmts(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    bool isPutln = false; 
    if(tok == PUTLN){
        isPutln = true;
    } 
    if (tok != PUT && !isPutln) {
        ParseError(line, "Missing Put or PutLine keyword");
        return false;
    }
    tok = Parser::GetNextToken(in, line);
    if (tok != LPAREN) {
        ParseError(line, "Missing left parenthesis");
        return false;
    }
    Value retVal;
    if (!Expr(in, line, retVal)) {
        ParseError(line, "Invalid expression in print statement");
        return false;
    }
    tok = Parser::GetNextToken(in, line);
    if (tok != RPAREN) {
        ParseError(line, "Missing right parenthesis");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line, "Missing semicolon at end of statement");
        return false;
    }
    if (isPutln) {
        cout << retVal << endl;  
    } else {
        cout << retVal;         
    }
    return true;
}


// 10. GetStmt := Get (Var) ;
bool GetStmt(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    LexItem idtok;
    if (tok != GET) {
        ParseError(line, "Missing GET keyword");
        return false;
    }
    tok = Parser::GetNextToken(in, line);
    if (tok != LPAREN) {
        ParseError(line, "Missing left parenthesis in GetStmt");
        return false;
    }
    if (!Var(in, line, idtok)) {
        ParseError(line, "Missing variable in GetStmt");
        return false;
    }
    string varName = idtok.GetLexeme();

    tok = Parser::GetNextToken(in, line);
    if (tok != RPAREN) {
        ParseError(line, "Missing right parenthesis in GetStmt");
        return false;
    }
    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line, "Missing semicolon in GetStmt");
        return false;
    }

    Token type = SymTable[varName];
    Value input;

    if(type == INT){
        int i;
        if(!(cin >> i)){
            ParseError(line, "Invalid int input in GetStmt");
            return false;
        }
        input = Value(i);
    }
    else if(type == FLOAT){
        double i;
        if(!(cin >> i)){
            ParseError(line, "Invalid double input in GetStmt");
            return false;
        }
        input = Value(i);
    }
    else if(type == BOOL){
        string bStr;
        cin >> bStr;
        if(bStr == "true"){
            input = Value(true);
        }
        else if(bStr == "false"){
            input = Value(false);
        }
        else{
            ParseError(line, "Invalid BOOL input for GetStmt");
            return false;
        }
    }
    else if(type == CHAR){
        char i;
        if(!(cin>>i)){
            ParseError(line, "Invalid CHAR input in GetStmt");
            return false;
        }
        input = Value(i);
    }
    else if(type == STRING){
        string inpStr;
        cin >> inpStr;
        input = Value(inpStr);
    }
    else{
        ParseError(line,"Invalid type for GetStmt");
        return false;
    }
    TempsResults[varName] = input;
    return true;
}

// 11. IfStmt ::= IF Expr THEN StmtList { ELSIF Expr THEN StmtList } [ ELSE StmtList ] END IF ;
bool IfStmt(istream& in, int& line) {
    LexItem tok;
    // Track whether any condition has been met (IF or any ELSIF)
    bool conditionMet = false;
    
    // Check for IF keyword
    tok = Parser::GetNextToken(in, line);
    if (tok != IF) {
        ParseError(line, "Missing IF keyword");
        return false;
    }
    
    // Parse and evaluate the IF condition
    Value condVal;
    if (!Expr(in, line, condVal)) {
        ParseError(line, "Missing or invalid condition after IF");
        return false;
    }
    
    // Ensure the condition is a boolean value
    if (!condVal.IsBool()) {
        ParseError(line, "IF condition must be bool");
        return false;
    }
    
    // Check for THEN keyword after the condition
    tok = Parser::GetNextToken(in, line);
    if (tok != THEN) {
        ParseError(line, "Missing THEN after IF condition");
        return false;
    }
    
    // If the main IF condition is true, execute its statement block
    if (condVal.GetBool()) {
        conditionMet = true;  // Mark that a condition has been met
        if (!StmtList(in, line)) {
            return false;  // Error in statement list
        }
    } 
    // If the main IF condition is false, skip its statement block
    else {
        while (true) {
            tok = Parser::GetNextToken(in, line);
            if (tok == ELSIF) {
                // Found the next ELSIF clause, push it back for processing in the next step
                Parser::PushBackToken(tok);
                break;
            }
        }
    }
    
    // Get the next token to check for ELSIF/ELSE/END
    tok = Parser::GetNextToken(in, line);
    
    // Process all ELSIF clauses in sequence
    while (tok == ELSIF) {
        // If no condition has been met yet, evaluate this ELSIF
        if (!conditionMet) {
            // Parse and evaluate the ELSIF condition
            if (!Expr(in, line, condVal)) {
                ParseError(line, "Missing or invalid condition after ELSIF");
                return false;
            }
            
            // Ensure the ELSIF condition is a boolean value
            if (!condVal.IsBool()) {
                ParseError(line, "Run-Time Error-ELSIF condition must be boolean");
                return false;
            }
            
            // Check for THEN keyword after the ELSIF condition
            tok = Parser::GetNextToken(in, line);
            if (tok != THEN) {
                ParseError(line, "Missing THEN after ELSIF condition");
                return false;
            }
            
            // If this ELSIF condition is true, execute its statement block
            if (condVal.GetBool()) {
                conditionMet = true;  // Mark that a condition has been met
                if (!StmtList(in, line)) {
                    return false;  // Error in statement list
                }
            } 
            // If this ELSIF condition is false, skip its statement block
            else {
                while (true) {
                    tok = Parser::GetNextToken(in, line);
                    if (tok == ELSIF || tok == ELSE) {
                        // Found next ELSIF or ELSE clause, push it back for processing
                        Parser::PushBackToken(tok);
                        break;
                    }
                }
            }
        }
        // If a previous condition was met, skip this entire ELSIF clause
        else {
            // Skip until we find ELSIF, ELSE, or END
            while (true) {
                tok = Parser::GetNextToken(in, line);
                if (tok == ELSIF || tok == ELSE) {
                    Parser::PushBackToken(tok);
                    break;
                }
            }
        }
        
        // Get the next token to check for another ELSIF, ELSE, or END
        tok = Parser::GetNextToken(in, line);
    }
    
    // Process the ELSE clause if present
    if (tok == ELSE) {
        // If no condition has been met yet, execute the ELSE block
        if (!conditionMet) {
            if (!StmtList(in, line)) {
                return false;  // Error in statement list
            }
        } 
        // If a previous condition was met, skip the ELSE block
        else {
            while (true) {
                tok = Parser::GetNextToken(in, line);
                if (tok == END) {
                    // Found END keyword, push it back for processing
                    Parser::PushBackToken(tok);
                    break;
                }
            }
        }
        
        // Get the next token which should be END
        tok = Parser::GetNextToken(in, line);
    }
    
    // Check for END keyword to terminate the IF statement
    if (tok != END) {
        ParseError(line, "Missing END after IF statement");
        return false;
    }
    
    // Check for IF keyword after END (complete syntax is "END IF;")
    tok = Parser::GetNextToken(in, line);
    if (tok != IF) {
        ParseError(line, "Missing IF after END");
        return false;
    }
    
    // Check for semicolon to terminate the IF statement
    tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) {
        ParseError(line, "Missing semicolon at end of IF statement");
        return false;
    }
    
    // Successfully parsed the IF statement
    return true;
}
// 12. AssignStmt ::= Var := Expr ;
bool AssignStmt(istream& in, int& line) {
    // 1. Check variable
    LexItem idtok;
    if (!Var(in, line, idtok)) {
        ParseError(line, "Invalid variable in assignment");
        return false;
    }
    string varName = idtok.GetLexeme();

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != ASSOP) {
        ParseError(line, "Missing Assignment Operator");
        ParseError(line, "Invalid assignment statement.");
        return false;
    }

    Value right;
    if (!Expr(in, line, right)) {
        ParseError(line, "Invalid assignment statement.");
        return false;
    }

    Token varType = SymTable[varName];
    if(varType==INT && !right.IsInt()){
        ParseError(line, "INT Type mismatch in assignment");
        return false;
    }   
    if(varType==FLOAT && !right.IsReal()){
        ParseError(line, "FLOAT Type mismatch in assignment");
        return false;
    }   
    if(varType==BOOL && !right.IsBool()){
        ParseError(line, "BOOL Type mismatch in assignment");
        return false;
    }   
    if(varType==STRING && !right.IsString()){
        ParseError(line, "STRING Type mismatch in assignment");
        return false;
    }
    if(varType==CHAR && !right.IsChar()){
        ParseError(line, "CHAR Type mismatch in assignment");
        return false;
    }          
   
    TempsResults[varName] = right;
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
    string varName = tok.GetLexeme();

    // Check if variable is declared 
    if (SymTable.find(varName) == SymTable.end()) {
        ParseError(line, "Undeclared variable: " + varName);
        return false;
    }

    idtok = tok; // Return the identifier token
    return true;
}

// 14. Expr ::= Relation {(AND | OR) Relation }
bool Expr(istream& in, int& line, Value & retVal) {
    if (!Relation(in, line, retVal)) {
        return false;
    }
    
    LexItem tok;
    while (true) {
        tok = Parser::GetNextToken(in, line);   
        if (tok != AND && tok != OR) {
            Parser::PushBackToken(tok);
            break;
        }

        Value right;
        if (!Relation(in, line, right)) {
            ParseError(line, "Missing relation after logical operator");
            return false;
        }

        if (tok == AND) {
            if (retVal.GetBool() == false) {
                break;  
            }
            retVal = retVal && right;
        }
        else if (tok == OR) {
            if (retVal.GetBool() == true) {
                break;  
            }
            retVal = retVal || right;
        }
    }
    return true;
}
// 15. Relation ::= SimpleExpr [ ( = | /= | < | <= | > | >= ) SimpleExpr ]
bool Relation(istream& in, int& line, Value & retVal) {
    if(!SimpleExpr(in,line, retVal)){
        //ParseError(line, "Missing Relation in EXPR");
        return false;
    }
    LexItem tok = Parser::GetNextToken(in,line);
    if(tok != EQ && tok != NEQ && tok != LTHAN && tok != LTE && tok!= GTHAN && tok!= GTE){
        Parser::PushBackToken(tok);
        return true;
    }
    Value right;
    if(!SimpleExpr(in,line, right)){
        ParseError(line, "Missing SimpleExpr after Operator in Relation");
        return false;
    }

    Token relOperator = tok.GetToken();
    if(relOperator == EQ){
        retVal = retVal == right;
    }
    else if(relOperator == NEQ){
        retVal = retVal != right;
    }
    else if(relOperator == LTHAN){
        retVal = retVal < right;
    }
    else if(relOperator == LTE){
        retVal = retVal <= right;
    }
    else if(relOperator == GTHAN){
        retVal = retVal > right;
    }
    else if(relOperator == GTE){
        retVal = retVal> right;
    }
    else{
        ParseError(line, "Invalid Relational Operator");
        return false;
    }
    return true;
}

// 16. SimpleExpr ::= STerm { ( + | - | & ) STerm }
bool SimpleExpr(istream& in, int& line, Value & retVal) {
    if (!STerm(in, line, retVal)) {
        ParseError(line, "Missing SimpleExpr in Relation");
        ParseError(line, "Missing operand");
        return false;
    }
    LexItem tok;
    while (true) {
        tok = Parser::GetNextToken(in, line);
        if (tok != PLUS && tok != MINUS && tok != CONCAT) {
            Parser::PushBackToken(tok);
            break;
        }
        Value right;
        if (!STerm(in, line, right)) {
            ParseError(line, "Missing operand after operator");
            return false;
        }
        if(tok == PLUS){
            retVal = retVal + right;
        }
        if(tok == MINUS){
            retVal = retVal - right;
        }
        if(tok == CONCAT){
            retVal = retVal.Concat(right);
        }
    }
    return true;
}

// 17. STerm ::= [ ( + | - ) ] Term
bool STerm(istream& in, int& line, Value & retVal) {
    LexItem tok = Parser::GetNextToken(in, line);
    int sign = 1; 
    if(tok == PLUS || tok == MINUS){
        if (tok == PLUS) {
            sign = 1;
        }
        if(tok == MINUS){
            sign = -1;
        }
        if (!Term(in, line, sign,retVal)) {
            ParseError(line, "Missing term after unary sign");
            return false;
        }
        return true;        
    }
    Parser::PushBackToken(tok);
    if (!Term(in, line, sign,retVal)) {
        ParseError(line, "Missing term");
        return false;
    }
    return true;
}


// 18. Term ::= Factor { ( * | / | MOD ) Factor }
bool Term(istream& in, int& line, int sign, Value & retVal) {
    if (!Factor(in, line, sign, retVal)) {
        return false;
    }

    LexItem tok;
    while (true) {
        tok = Parser::GetNextToken(in, line);
        
        if (tok != MULT && tok != DIV && tok != MOD) {
            Parser::PushBackToken(tok);
            break;
        }
        Value right;
        if (!Factor(in, line, 1, right)) {
            ParseError(line, "Missing factor after operator in term");
            return false;
        }

        if (tok == MULT){
            retVal = retVal * right;
        }
        else if(tok == DIV){
            retVal = retVal / right;
        }
        else if(tok == MOD){
            retVal = retVal % right;
        }else{
            retVal = retVal;
        }
    }
    return true;
}

// 19. Factor ::= Primary [** Primary ] | NOT Primary
bool Factor(istream& in, int& line, int sign, Value & retVal) {
    LexItem tok = Parser::GetNextToken(in, line);
    //Case 1: NOT
    if (tok == NOT) {
        if (!Primary(in, line, sign,retVal)) {  // NOT ignores incoming sign
            ParseError(line, "Missing primary after NOT");
            return false;
        }
        retVal = !retVal;
        return true;
    }
    
    //Case 2: Exponentiation
    Parser::PushBackToken(tok);
    if (!Primary(in, line, sign, retVal)) {
        ParseError(line, "Missing primary");
        return false;
    }
    
    tok = Parser::GetNextToken(in, line);
    if (tok == EXP) {
        LexItem tok = Parser::GetNextToken(in,line);
        int expSign;
        if (tok == PLUS) {
            expSign = 1;
        } else if (tok == MINUS) {
            expSign = -1;
        } else {
            Parser::PushBackToken(tok);
        }
        Value expVal;
        if (!Primary(in, line, expSign, expVal)) {  // Second primary uses default sign
            ParseError(line, "Missing primary after **");
            return false;
        }

        retVal = retVal.Exp(expVal);  // Use Exp
    } else {
        Parser::PushBackToken(tok);
        retVal = retVal;  // No exp just return the primary
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
        retVal = Value(stoi(tok.GetLexeme()));
        return true;
    }
    if (tok == FCONST) {
        retVal = Value(stod(tok.GetLexeme()));
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
    //we will need the varName to look it up in our tables
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

    tok = Parser::GetNextToken(in, line);
    if (tok == LPAREN) {
        Value startIdx, endIdx;
        if (!Range(in, line, startIdx, endIdx)){
            return false;
        } 

        //at this point the range is valid so we need to actually substring here
        if(!retVal.IsString()){
            ParseError(line,"Not a string");
        }

        string retValstr = retVal.GetString();
        int len = retValstr.length();
        int start = startIdx.GetInt();
        if (endIdx.IsInt()) {  // Substring access
            int end = endIdx.GetInt();
            if (start < 0 || end >= len || start > end) {
                ParseError(line, "String index out of bounds");
                return false;
            }
            retVal.SetString(retValstr.substr(start, end - start + 1));
        } else {  
            if (start < 0 || start >= len) {
                ParseError(line, "String index out of bounds");
                return false;
            }
                retVal = Value(retValstr[start]);
        }

        tok = Parser::GetNextToken(in, line);
        if (tok != RPAREN) {
            ParseError(line, "Missing closing parenthesis for range");
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
        Parser::PushBackToken(tok);
        retVal2.SetType(VERR); // Mark as no second value
    }

    return true;
}