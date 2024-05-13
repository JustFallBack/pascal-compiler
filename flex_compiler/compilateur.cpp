//  A compiler from a very simple Pascal-like structured language LL(k)
//  to 64-bit 80x86 Assembly langage
//  Copyright (C) 2019 Pierre Jourlin
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Build with "make compilateur"


#include <string>
#include <iostream>
#include <cstdlib>
#include <set>
#include <map>
#include <FlexLexer.h>
#include "tokeniser.h"
#include <cstring>

using namespace std;

enum OPREL {EQU, DIFF, INF, SUP, INFE, SUPE, WTFR};
enum OPADD {ADD, SUB, OR, WTFA};
enum OPMUL {MUL, DIV, MOD, AND ,WTFM};
enum TYPES {INTEGER, BOOLEAN, DOUBLE, CHAR ,WTFT};

TOKEN current;				// Current token


FlexLexer* lexer = new yyFlexLexer; // This is the flex tokeniser
// tokens can be read using lexer->yylex()
// lexer->yylex() returns the type of the lexicon entry (see enum TOKEN in tokeniser.h)
// and lexer->YYText() returns the lexicon entry as a string

	
map<string, TYPES> DeclaredVariables;
unsigned long TagNumber=0;

bool IsDeclared(const char *id){
	return DeclaredVariables.find(id)!=DeclaredVariables.end();
}


void Error(string s){
	// current = token index
	cerr << "Line n°"<<lexer->lineno()<<", read : '"<<lexer->YYText()<<"'("<<current<<"), but ";
	cerr<< s << endl;
	exit(-1);
}

// check if specified keyword is expected and read keyword
void CheckReadKeyword(const char *keyword) {
	if(current!=KEYWORD) {
		Error("keyword expected");
	}
	if(strcmp(lexer->YYText(),keyword)!=0) {
		string err(keyword);
		string err_msg = "'"+err+"' keyword expected";
		Error(err_msg);
	}
	current=(TOKEN) lexer->yylex();
}

// Statement := AssignementStatement | IfStatement | WhileStatement | ForStatement | BlockStatement
// IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
// WhileStatement := "WHILE" Expression "DO" Statement
// ForStatement := "FOR" AssignementStatement "To" Expression "DO" Statement
// BlockStatement := "BEGIN" Statement { ";" Statement } "END"

// Program := [VarDeclarationPart] StatementPart
// VarDeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
// VarDeclaration := Identifer {"," Identifier} ":" Type
// StatementPart := Statement {";" Statement} "."
// Statement := AssignementStatement
// AssignementStatement := Letter "=" Expression

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
// SimpleExpression := Term {AdditiveOperator Term}
// Term := Factor {MultiplicativeOperator Factor}
// Factor := Number | Identifier | "(" Expression ")"
// Identifier := Letter{Letter|Digit}
// Number := Digit{Digit}

// AdditiveOperator := "+" | "-" | "||"
// MultiplicativeOperator := "*" | "/" | "%" | "&&"
// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
// Letter := "a"|...|"z"
// Type := "BOOLEAN" | "CHAR" | "INTEGER" | "DOUBLE"
	
// Identifier := Letter{Letter|Digit}
enum TYPES Identifier(void){
	cout << "\tpush "<<lexer->YYText()<<endl;
	current=(TOKEN) lexer->yylex();
	return INTEGER;
}

// Number := Digit{Digit}
enum TYPES Number(void){
	cout <<"\tpush $"<<atoi(lexer->YYText())<<endl;		// Get next token without changing current
	current=(TOKEN) lexer->yylex();						// Advance to next token
	return INTEGER;
}

enum TYPES Expression(void);			// Called by Term() and calls Term()

// Factor := Number | Identifier | "(" Expression ")"
enum TYPES Factor(void){
	TYPES type;											// Type of the factor
	if(current==RPARENT){
		current=(TOKEN) lexer->yylex();					// consume '(' and advance to next token
		type = Expression();							// get expression and its type
		if(current!=LPARENT) {							// triggers an error if token is not ')'
			Error("')' expected");
		}
		else {
			current=(TOKEN) lexer->yylex();				// consume ')' and advance to next token
		}
	}
	else {
		if (current==NUMBER) {
			type = Number();
		}
		else {
			if(current==ID) {
				type = Identifier();
			}
			else {
				Error("'(' or number or letter expected");
			}
		}
	}
	return type;
}

// MultiplicativeOperator := "*" | "/" | "%" | "&&"
OPMUL MultiplicativeOperator(void) {
	OPMUL opmul;
	if(strcmp(lexer->YYText(),"*")==0) {
		opmul = MUL;
	}
	else if(strcmp(lexer->YYText(),"/")==0) {
		opmul = DIV;
	}
	else if(strcmp(lexer->YYText(),"%")==0) {
		opmul = MOD;
	}
	else if(strcmp(lexer->YYText(),"&&")==0) {
		opmul = AND;
	}
	else {
		opmul=WTFM;
	}
	current=(TOKEN) lexer->yylex();
	return opmul;
}

// Term := Factor {MultiplicativeOperator Factor}
enum TYPES Term(void){
	TYPES type1, type2;
	OPMUL mulop;
	type1 = Factor();								// Get first factor and its type
	while(current==MULOP){
		mulop=MultiplicativeOperator();				// Save operator in local variable
		type2 = Factor();							// Get second factor and its type
		if(type1!=type2) {							// Triggers an error if the types are different
			Error("TYPES error: cannot mutiply/divide/modulo/and different types");
		}
		cout << "\tpop %rbx"<<endl;					// get first operand
		cout << "\tpop %rax"<<endl;					// get second operand
		switch(mulop){
			case AND:
				cout << "\tmulq	%rbx"<<endl;		// a * b -> %rdx:%rax
				cout << "\tpush %rax\t# AND"<<endl;	// store result
				break;
			case MUL:
				cout << "\tmulq	%rbx"<<endl;		// a * b -> %rdx:%rax
				cout << "\tpush %rax\t# MUL"<<endl;	// store result
				break;
			case DIV:
				cout << "\tmovq $0, %rdx"<<endl; 	// Higher part of numerator  
				cout << "\tdiv %rbx"<<endl;			// quotient goes to %rax
				cout << "\tpush %rax\t# DIV"<<endl;	// store result
				break;
			case MOD:
				cout << "\tmovq $0, %rdx"<<endl; 	// Higher part of numerator  
				cout << "\tdiv %rbx"<<endl;			// remainder goes to %rdx
				cout << "\tpush %rdx\t# MOD"<<endl;	// store result
				break;
			default:
				Error("multiplicative operator expected");
		}
	}
	return type1;
}

// AdditiveOperator := "+" | "-" | "||"
OPADD AdditiveOperator(void) {
	OPADD opadd;
	if(strcmp(lexer->YYText(),"+")==0) {
		opadd = ADD;
	}
	else if(strcmp(lexer->YYText(),"-")==0) {
		opadd = SUB;
	}
	else if(strcmp(lexer->YYText(),"||")==0) {
		opadd = OR;
	}
	else {
		opadd = WTFA;
	}
	current=(TOKEN) lexer->yylex();
	return opadd;
}

// SimpleExpression := Term {AdditiveOperator Term}
enum TYPES SimpleExpression(void) {
	TYPES type1, type2;
	OPADD adop;
	type1 = Term();											// Get first term and its type
	while(current==ADDOP){									// Loop to get all terms
		adop=AdditiveOperator();							// Save operator in local variable
		type2 = Term();										// Get second term and its type
		if(type1!=type2) {									// Triggers an error if the types are different
			Error("TYPES error: cannot add/substract/or different types");
		}
		cout << "\tpop %rbx"<<endl;							// get first operand
		cout << "\tpop %rax"<<endl;							// get second operand
		switch(adop){
			case OR:
				cout << "\taddq	%rbx, %rax\t# OR"<<endl;	// operand1 OR operand2
				break;			
			case ADD:
				cout << "\taddq	%rbx, %rax\t# ADD"<<endl;	// add both operands
				break;			
			case SUB:	
				cout << "\tsubq	%rbx, %rax\t# SUB"<<endl;	// substract both operands
				break;
			default:
				Error("additive operator expected");
		}
		cout << "\tpush %rax"<<endl;						// store result
	}
	return type1;
}

// Type := "BOOLEAN" | "CHAR" | "INTEGER" | "DOUBLE"
TYPES GetType(void) {
	TYPES type;
	if(strcmp(lexer->YYText(),"BOOLEAN")==0) {
		type = BOOLEAN;
	}
	else if(strcmp(lexer->YYText(),"INTEGER")==0) {
		type = INTEGER;
	}
	else if(strcmp(lexer->YYText(),"CHAR")==0) {
		type = CHAR;
	}
	else if(strcmp(lexer->YYText(),"DOUBLE")==0) {
		type = DOUBLE;
	}
	else {
		type = WTFT;
	}
	current=(TOKEN)lexer->yylex();
	return type;
}

// VarDeclaration := Identifier {"," Identifier} ":" Type
void VarDeclaration(void) {
	set<string> identifiers;					// Set to store identifiers
	if(current!=ID) {
		Error("identifier expected");
	}
	identifiers.insert(lexer->YYText());		// Store identifier in set
	current=(TOKEN)lexer->yylex();				// Consume identifier and advance to next token

	while(current==COMMA) {						// Loop to get all identifiers
		current=(TOKEN)lexer->yylex();			// Consume ',' and advance to next token
		if(current!=ID) {
			Error("identifier expected");
		}
		identifiers.insert(lexer->YYText());	// Store identifier in set
		current=(TOKEN)lexer->yylex();			// Consume identifier and advance to next token
	}

	if(current!=COLON) {
		Error("':' expected");
	}
	current=(TOKEN)lexer->yylex();				// Consume ':' and advance to next token

	cout << "\t.data" << endl;
    cout << "\t.align 8" << endl;
	TYPES type = GetType();
	for(set<string>::iterator i=identifiers.begin(); i!=identifiers.end(); ++i) {
		switch(type) {							// Print variable name and its type
			case INTEGER:
			case BOOLEAN:
				cout<<*i<<":\t.quad 0"<<endl;	
				break;
			case DOUBLE:
				cout<<*i<<":\t.double 0.0"<<endl;
				break;
			case CHAR:
				cout<<*i<<":\t.byte 0"<<endl;
				break;
			default:
				Error("unknown type"); 
		}
		DeclaredVariables[*i]=type;				// Add variable to declared variables map
	}
}

// VarDeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
void VarDeclarationPart(void) {
	CheckReadKeyword("VAR");					// Check if keyword is 'VAR', if yes advance to next token
	VarDeclaration();
	while(current == SEMICOLON) {				// Loop to get all VarDeclarations
		current = (TOKEN)lexer->yylex();
		VarDeclaration();
	}
	if(current!=DOT) {							// Triggers an error if token is not '.'
		Error("'.' expected");
	}
	current=(TOKEN)lexer->yylex();
}


// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
OPREL RelationalOperator(void){
	OPREL oprel;
	if(strcmp(lexer->YYText(),"==")==0)
		oprel=EQU;
	else if(strcmp(lexer->YYText(),"!=")==0)
		oprel=DIFF;
	else if(strcmp(lexer->YYText(),"<")==0)
		oprel=INF;
	else if(strcmp(lexer->YYText(),">")==0)
		oprel=SUP;
	else if(strcmp(lexer->YYText(),"<=")==0)
		oprel=INFE;
	else if(strcmp(lexer->YYText(),">=")==0)
		oprel=SUPE;
	else oprel=WTFR;
	current=(TOKEN) lexer->yylex();
	return oprel;
}

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
enum TYPES Expression(void){
	TYPES type1, type2;
	OPREL oprel;
	type1 = SimpleExpression();														// Get first simple expression and its type
	if(current==RELOP){
		oprel=RelationalOperator(); 												// Save operator in local variable
		type2 = SimpleExpression();													// Get second simple expression and its type
		if(type1!=type2) {															// Triggers an error if the types are different
			Error("TYPES error: cannot compare different types");
		}
		cout << "\tpop %rax"<<endl;
		cout << "\tpop %rbx"<<endl;
		cout << "\tcmpq %rax, %rbx"<<endl;
		switch(oprel){
			case EQU:
				cout << "\tje Vrai"<<++TagNumber<<"\t# If equal"<<endl;				// Jump if equal
				break;
			case DIFF:
				cout << "\tjne Vrai"<<++TagNumber<<"\t# If different"<<endl;		// Jump if different
				break;
			case SUPE:
				cout << "\tjae Vrai"<<++TagNumber<<"\t# If above or equal"<<endl;	// Jump if above or equal
				break;
			case INFE:
				cout << "\tjbe Vrai"<<++TagNumber<<"\t# If below or equal"<<endl;	// Jump if below or equal
				break;
			case INF:
				cout << "\tjb Vrai"<<++TagNumber<<"\t# If below"<<endl;				// Jump if below
				break;
			case SUP:
				cout << "\tja Vrai"<<++TagNumber<<"\t# If above"<<endl;				// Jump if above
				break;
			default:
				Error("Relational operator expected");
		}
		cout << "\tpush $0\t\t# False"<<endl;
		cout << "\tjmp Suite"<<TagNumber<<endl;
		cout << "Vrai"<<TagNumber<<":\tpush $0xFFFFFFFFFFFFFFFF\t\t# True"<<endl;	
		cout << "Suite"<<TagNumber<<":"<<endl;
		return BOOLEAN;																// return BOOLEAN if the expression is relational
	}
	return type1;																	// return the type of the expression if not
}

// AssignementStatement := Identifier ":=" Expression
string AssignementStatement(void){
	enum TYPES type1, type2;
	string variable;
	if(current!=ID)							// Triggers an error if token is not an identifier
		Error("identifier expected");
	if(!IsDeclared(lexer->YYText())){		// Triggers an error if the identifier is not declared
		cerr << "Error : variable '"<<lexer->YYText()<<"' is not declared"<<endl;
		exit(-1);
	}

	variable=lexer->YYText();
	type1 = DeclaredVariables[variable];	// Get type of the variable
	current=(TOKEN) lexer->yylex();			// consume identifier and advance to next token

	if(current!=ASSIGN){					// Triggers an error if token is not ':='
		Error("':=' expected");
	}
	current=(TOKEN) lexer->yylex();
	type2 = Expression();
	if(type1!=type2){						// Triggers an error if the types are different
		Error("TYPES error: cannot assign different types");
	}
	cout << "\tpop "<<variable<<endl;
	return variable;						// return the variables name
}

void Statement(void);	


// IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
void IfStatement(void) {
	enum TYPES type;
	unsigned long localTag=++TagNumber;

	CheckReadKeyword("IF");
	cout<<"IF"<<localTag<<":"<<endl; 								// label for IF
	type = Expression();
	if(type!=BOOLEAN) {
		Error("TYPES error: 'IF' expression must be boolean");		// Triggers an error if the expression is not boolean in 'IF' statement
	}
	cout<<"\tpop %rax"<<endl;
	cout<<"\tcmpq $0, %rax"<<endl;
	cout<<"\tje IFfalse"<<localTag<<"\t\t# jump to ELSE "<<endl;	// jump to ELSE if 'IF' expression is false (even if there is no else)

	CheckReadKeyword("THEN");
	cout<<"IFtrue"<<localTag<<":\t\t\t# THEN"<<endl; 				// label for THEN
	Statement();
	cout<<"\tjmp IFend"<<localTag<<"\t\t# jump to endIf"<<endl;		// jump to end of 'IF' statement
	cout<<"IFfalse"<<localTag<<":\t\t\t# ELSE"<<endl; 				// label for ELSE (even if there is no else)

	if(current==KEYWORD && strcmp(lexer->YYText(),"ELSE")==0) {
		CheckReadKeyword("ELSE");
		Statement();
	}
	cout<<"IFend"<<localTag<<":"<<endl; 							// label for end of 'IF' statement
}

// WhileStatement := "WHILE" Expression "DO" Statement
void WhileStatement(void) {
	unsigned long localTag=++TagNumber;

	CheckReadKeyword("WHILE");
	cout<<"WHILE"<<localTag<<":"<<endl; 								// label for WHILE
	Expression();
	cout<<"\tpop %rax"<<endl;
	cout<<"\tcmpq $0, %rax"<<endl;
	cout<<"\tje WHILEend"<<localTag<<"\t\t# jump to end of WHILE"<<endl; // jump to end of 'WHILE' statement if expression is false

	CheckReadKeyword("DO");
	cout<<"WHILEtrue"<<localTag<<":\t\t\t# DO"<<endl; 					// label for DO
	Statement();
	cout<<"\tjmp WHILE"<<localTag<<endl;								// jump to 'WHILE' statement
	cout<<"WHILEend"<<localTag<<":"<<endl; 								// label for end of 'WHILE' statement

}

// ForStatement := "FOR" AssignementStatement "TO" Expression "DO" Statement
void ForStatement(void) {
	unsigned long localTag=++TagNumber;

	cout<<"FOR"<<localTag<<":"; 											// label for FOR
	CheckReadKeyword("FOR");

	string loop_var = AssignementStatement();

	CheckReadKeyword("TO");
	Expression();
	cout<<"TO"<<localTag<<":"; 												// label for TO
	cout<<"\tmovq (%rsp), %rax"<<endl; 										// necessary to avoid 'too many memory references'
	cout<<"\tcmpq %rax, "<<loop_var<<endl;
	cout<<"\tjae FORend"<<localTag<<"\t\t# jump at the end of FOR"<<endl; 	// jump at the end of 'FOR' statement if loop_var is above or equal expression

	CheckReadKeyword("DO");
	Statement();
	cout<<"\tincq "<<loop_var<<"\t\t# loop_var++"<<endl;
	cout<<"\tjmp TO"<<localTag<<endl;										// jump to 'TO' statement
	cout<<"FORend"<<localTag<<":"<<endl; 									// label for end of 'FOR' statement
}


// BlockStatement := "BEGIN" Statement { ";" Statement } "END"
void BlockStatement(void) {
	unsigned long localTag=++TagNumber;

	CheckReadKeyword("BEGIN");
	cout<<"BEGIN"<<localTag<<":"<<endl; 									// label for BEGIN
	Statement();
	while(current==SEMICOLON) {
		current=(TOKEN) lexer->yylex();
		Statement();
	}

	CheckReadKeyword("END");
	cout<<"END"<<localTag<<":"<<endl; 										// label for END
}

// Statement := AssignementStatement | IfStatement | WhileStatement | ForStatement | BlockStatement
void Statement(void){
	if(current==ID) {
		AssignementStatement();
	}
	else if(current==KEYWORD) {
		if(strcmp(lexer->YYText(),"IF")==0) {						// Check if keyword is 'IF'
			IfStatement();
		}
		else if(strcmp(lexer->YYText(), "WHILE")==0) {				// Check if keyword is 'WHILE'
			WhileStatement();
		}
		else if(strcmp(lexer->YYText(),"FOR")==0) {					// Check if keyword is 'FOR'
			ForStatement();
		}
		else if(strcmp(lexer->YYText(),"BEGIN")==0) {				// Check if keyword is 'BEGIN'
			BlockStatement();
		}
		else {
			Error("keyword not identified (must be IF or WHILE or FOR or BEGIN)");
		}
	}
	else {
		Error("keyword or identifier expected");
	}
}


// StatementPart := Statement {";" Statement} "."
void StatementPart(void){
	cout << "\t.text\t\t# The following lines contain the program"<<endl;
	cout << "\t.globl main\t# The main function must be visible from outside"<<endl;
	cout << "main:\t\t\t# The main function body :"<<endl;
	cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top"<<endl;
	Statement();
	while(current==SEMICOLON){
		current=(TOKEN) lexer->yylex();												// consume ';' and advance to next token
		Statement();
	}
	if(current!=DOT)
		Error("caractère '.' attendu");
	current=(TOKEN) lexer->yylex();													// consume '.' and advance to next token
}

// Program := [DeclarationPart] StatementPart
void Program(void){														// Check if token is '['
	VarDeclarationPart();
	StatementPart();	
}

int main(void){
	cout << "\t\t\t\t# This code was produced by the compiler made by Elliot Pozucek"<<endl; 	// Header for the gcc assembler / linker
	current=(TOKEN) lexer->yylex();
	Program();
	
	cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top"<<endl;				// Trailer for the gcc assembler / linker
	cout << "\tret\t\t\t# Return from main function"<<endl;
	if(current!=FEOF){
		cerr <<"Caractères en trop à la fin du programme : ["<<current<<"]";					// unexpected characters at the end of program
		Error("."); 
	}
}
		
			





