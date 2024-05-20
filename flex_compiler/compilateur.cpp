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
	if (current!=KEYWORD) {
		Error("keyword expected.");
	}
	if (strcmp(lexer->YYText(),keyword)!=0) {
		string err(keyword);
		string err_msg = "'"+err+"' keyword expected.";
		Error(err_msg);
	}
	current=(TOKEN) lexer->yylex();
}

// Statement := AssignementStatement | IfStatement | WhileStatement | ForStatement | BlockStatement | DisplayStatement | CaseStatement
// IfStatement := "IF" Expression "THEN" Statement ["ELSE" Statement]
// WhileStatement := "WHILE" Expression "DO" Statement
// ForStatement := "FOR" AssignementStatement ("TO" | "DOWNTO") Expression "DO" Statement
// BlockStatement := "BEGIN" Statement { ";" Statement } "END"
// DisplayStatement := "DISPLAY" Expression
// CaseStatement := "CASE" Expression "OF" CaseListElement {";" CaseListElement} ["ELSE" Statement] "END"
// CaseListElement := CaseLabel ":" Statement
// CaseLabel := CharConst {"," CharConst} | {Digit}+ ".." {Digit}+ | Number {"," Number} | Identifier

// Program := [VarDeclarationPart] StatementPart
// VarDeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
// VarDeclaration := Identifer {"," Identifier} ":" Type
// StatementPart := Statement {";" Statement} "."
// Statement := AssignementStatement
// AssignementStatement := Letter ":=" Expression

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
// SimpleExpression := Term {AdditiveOperator Term}
// Term := Factor {MultiplicativeOperator Factor}
// Factor := "(" Expression ")" | Number | Identifier | CharConst
// Identifier := Letter{Letter|Digit}
// Number := {digit}+(\.{digit}+)?
// CharConst := "'" Letter "'"

// AdditiveOperator := "+" | "-" | "||"
// MultiplicativeOperator := "*" | "/" | "%" | "&&"
// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
// Letter := "a"|...|"z"
// Type := "INTEGER" | "BOOLEAN" | "DOUBLE" | "CHAR"
	
// Identifier := Letter{Letter|Digit}
enum TYPES Identifier(void){
	enum TYPES type;
	if (!IsDeclared(lexer->YYText())){			// Triggers an error if the variable is not declared
		cerr<<"Error: Variable '"<<lexer->YYText()<<"' not declared."<<endl;
		Error(".");
	}
	type=DeclaredVariables[lexer->YYText()];	// Get type of the variable
	cout<<"\tpush\t"<<lexer->YYText()<<endl;
	current=(TOKEN) lexer->yylex();				// Advance to next token
	return type;
}

// Number := {digit}+(\.{digit}+)?
enum TYPES Number(void) {					// We assume that little endian architecture is used
	enum TYPES type;
	string num = lexer->YYText();
	double d;								// 64-bit float
	unsigned long long *l;					// Pointer to 64-bit unsigned integer
	if (num.find('.') != string::npos) {	// Token is a DOUBLE
		d = stod(num);						// Convert string TOKEN to double
		l = (unsigned long long *) &d;		// Get the address of the double
        unsigned int high_part = *l;
        unsigned int low_part = *(((unsigned int *)l) + 1);
        cout<<"\tsubq\t$8, %rsp\t\t\t# allocate 8 bytes on stack's top" << endl;
        cout<<"\tmovl\t$"<<high_part<<", (%rsp)\t# 32-bit high part of "<<d<<endl;
        cout<<"\tmovl\t$"<<low_part<< ", 4(%rsp)\t# 32-bit low part of "<<d<<endl;
		type = DOUBLE;
	} 
	else {									// Token is an INTEGER
		cout<<"\tpush\t$"<<stoi(lexer->YYText())<<endl;
		type = INTEGER;
	}
	current=(TOKEN) lexer->yylex(); 		// Advance to next token
	return type;
}

// CharConst := "'" Letter "'"
enum TYPES CharConst(void){
	cout<<"\tmovq\t$0, %rax"<<endl;
	cout<<"\tmovb\t$"<<lexer->YYText()<<",%al"<<endl;
	cout<<"\tpush\t%rax\t# push a 64-bit version of "<<lexer->YYText()<<endl;
	current=(TOKEN) lexer->yylex();			// Advance to next token
	return CHAR;
}

enum TYPES Expression(void);				// Called by Term() and calls Term()

// Factor := "(" Expression ")" | Number | Identifier | CharConst
enum TYPES Factor(void){
	TYPES type;
	switch(current) {							
	case RPARENT:							// If token is '(', call Expression() and check if next token is ')'
		current=(TOKEN) lexer->yylex();		// Consume '(' and advance to next token
		type = Expression();				// Get expression and its type
		if (current!=LPARENT) {							
			Error("')' expected.");			// Triggers an error if token is not ')'
		}
		else {
			current=(TOKEN) lexer->yylex();	// Consume ')' and advance to next token
		}
		break;
	case NUMBER:							// If token is a number, call Number()
		type = Number();
		break;
	case ID:								// If token is an identifier, call Identifier()
		type = Identifier();
		break;
	case CHARCONST: 						// If token is a character, call CharConst()
		type = CharConst();
		break;
	default:								// Triggers an error if token is not '(', number, identifier or character
		Error("'(' or number or letter or char expected.");
	}
	return type;
}

// MultiplicativeOperator := "*" | "/" | "%" | "&&"
OPMUL MultiplicativeOperator(void) {
	OPMUL opmul;
	if (strcmp(lexer->YYText(),"*")==0) {
		opmul = MUL;
	}
	else if (strcmp(lexer->YYText(),"/")==0) {
		opmul = DIV;
	}
	else if (strcmp(lexer->YYText(),"%")==0) {
		opmul = MOD;
	}
	else if (strcmp(lexer->YYText(),"&&")==0) {
		opmul = AND;
	}
	else {
		opmul=WTFM;
	}
	current=(TOKEN) lexer->yylex();
	return opmul;
}

// Term := Factor {MultiplicativeOperator Factor}
enum TYPES Term(void) {
	TYPES type1, type2;
	OPMUL mulop;
	type1 = Factor();									// Get first factor and its type
	while(current==MULOP) {
		mulop=MultiplicativeOperator();					// Save operator in local variable
		type2 = Factor();								// Get second factor and its type
		if (type1==CHAR) {								// Triggers an error if the types are characters
			Error("TYPES error: cannot apply multipicative operations to CHAR.");
		}
		if (type1!=type2) {								// Triggers an error if the types are different
			Error("TYPES error: cannot apply multipicative operations between different types.");
		}
		switch(mulop) {
			case AND:
				if (type2!=BOOLEAN) {					// AND operator can only be applied to booleans
					Error("TYPES error: cannot apply AND operator to non-boolean types.");
				}
				cout<<"\tpop \t%rbx"<<endl;				// Get first operand
				cout<<"\tpop \t%rax"<<endl;				// Get second operand
				cout<<"\tmulq\t%rbx"<<endl;				// a * b -> %rdx:%rax
				cout<<"\tpush\t%rax\t\t# AND"<<endl;	// Store result
				break;
			case MUL:
				if (type2!=INTEGER && type2!=DOUBLE) {	// Multiplication can only be applied to integers or doubles
					Error("TYPES error: cannot apply MUL operator to non-numerical types.");
				}
				if (type2==INTEGER) {						// Multiplication of two integers
					cout<<"\tpop \t%rbx"<<endl;				// Get first operand
					cout<<"\tpop \t%rax"<<endl;				// Get second operand
					cout<<"\tmulq\t%rbx"<<endl;				// a * b -> %rdx:%rax
					cout<<"\tpush\t%rax\t\t# MUL"<<endl;	// Store result
				}
				else {										// Multiplication of two doubles
					cout<<"\tfldl\t(%rsp)"<<endl;			// Store first operand in %st(0)
					cout<<"\tfldl\t8(%rsp)"<<endl;			// Store second operand in %st(0) (first operand is now in %st(1))
					cout<<"\tfmulp\t%st(0), %st(1)"<<endl;	// Multiply %st(0) by %st(1) and store result in %st(1), then depile FPU stack (%st(0) now contains the result)
					cout<<"\taddq\t$8, %rsp"<<endl;			// Depile CPU's stack
					cout<<"\tfstpl\t(%rsp)"<<endl;			// Depile %st(0) and put it on top of the CPU stack
				}
				break;
			case DIV:
				if (type2!=INTEGER && type2!=DOUBLE) {		// Division can only be applied to integers or doubles
					Error("TYPES error: cannot apply DIV operator to non-numerical types.");
				}
				if (type2==INTEGER) {						// Division of two integers
					cout<<"\tpop \t%rbx"<<endl;				// Get first operand
					cout<<"\tpop \t%rax"<<endl;				// Get second operand
					cout<<"\tmovq\t$0, %rdx"<<endl; 		// Higher part of numerator set to 0
					cout<<"\tdiv\t%rbx"<<endl;				// Quotient goes to %rax
					cout<<"\tpush\t%rax\t\t# DIV"<<endl;	// Store result
				}
				else {										// Division if two doubles
					cout<<"\tfldl\t(%rsp)"<<endl;			// Store first operand in %st(0)
					cout<<"\tfldl\t8(%rsp)"<<endl;			// Store second operand in %st(0) (first operand is now in %st(1))
					cout<<"\tfdivp\t%st(0), %st(1)"<<endl;	// Divide %st(0) by %st(1) and store result in %st(1), then depile FPU stack (%st(0) now contains the result)
					cout<<"\taddq\t$8, %rsp"<<endl;			// Depile CPU's stack
					cout<<"\tfstpl\t(%rsp)"<<endl;			// Depile %st(0) and put it on top of the CPU stack
				}
				break;
			case MOD:
				if (type2!=INTEGER) {						// MOD operator can only be applied to integers
					Error("TYPES error: cannot apply MOD operator to non-integer types.");
				}
				cout<<"\tpop \t%rbx"<<endl;					// Get first operand
				cout<<"\tpop \t%rax"<<endl;					// Get second operand
				cout<<"\tmovq\t$0, %rdx"<<endl; 			// Higher part of numerator set to 0
				cout<<"\tdiv\t%rbx"<<endl;					// Remainder goes to %rdx
				cout<<"\tpush\t%rdx\t\t# MOD"<<endl;		// Store result
				break;
			default:
				Error("multiplicative operator expected.");
		}
	}
	return type1;
}

// AdditiveOperator := "+" | "-" | "||"
OPADD AdditiveOperator(void) {
	OPADD opadd;
	if (strcmp(lexer->YYText(),"+")==0) {
		opadd = ADD;
	}
	else if (strcmp(lexer->YYText(),"-")==0) {
		opadd = SUB;
	}
	else if (strcmp(lexer->YYText(),"||")==0) {
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
	type1 = Term();												// Get first term and its type
	while(current==ADDOP) {										// Loop to get all terms
		adop=AdditiveOperator();								// Save operator in local variable
		type2 = Term();											// Get second term and its type
		if (type1!=type2) {										// Triggers an error if the types are different
			Error("TYPES error: cannot add/substract/or different types.");
		}
		else if (type1==CHAR) {									// Triggers an error if the types are characters
			Error("TYPES error: cannot add/substract/or characters.");
		}
		switch(adop) {
			case OR:
				if (type2!=BOOLEAN) {							// OR operator can only be applied to booleans
					Error("TYPES error: cannot apply OR operator to non-boolean types.");
				}
				cout<<"\tpop \t%rbx"<<endl;						// Get first operand
				cout<<"\tpop \t%rax"<<endl;						// Get second operand
				cout<<"\taddq\t%rbx, %rax\t\t# OR"<<endl;		// Operand1 OR operand2
				cout<<"\tpush\t%rax"<<endl;						// Store result
				break;			
			case ADD:
				if (type2!=INTEGER && type2!=DOUBLE) {			// Addition can only be applied to integers or doubles
					Error("TYPES error: cannot add non-numerical types.");
				}
				if (type2==INTEGER) {
					cout<<"\tpop \t%rbx"<<endl;					// Get first operand
					cout<<"\tpop \t%rax"<<endl;					// Get second operand
					cout<<"\taddq\t%rbx, %rax\t\t# ADD"<<endl;	// Add both operands
					cout<<"\tpush\t%rax"<<endl;					// Store result
				}
				else {
					cout<<"\tfldl\t(%rsp)"<<endl;				// Store first operand in %st(0)
					cout<<"\tfldl\t8(%rsp)"<<endl;				// Store second operand in %st(0) (first operand is now in %st(1))
					cout<<"\tfaddp\t%st(0), %st(1)"<<endl;		// Add %st(0) to %st(1) and store result in %st(1), then depile FPU stack (%st(0) now contains the result)
					cout<<"\taddq\t$8, %rsp"<<endl;				// Depile CPU's stack
					cout<<"\tfstpl\t(%rsp)"<<endl;				// Depile %st(0) and put it on top of the CPU stack
				}
				break;			
			case SUB:
				if (type2!=INTEGER && type2!=DOUBLE) {			// Substraction can only be applied to integers or doubles
					Error("TYPES error: cannot substract non-numerical types.");
				}
				if (type2==INTEGER) {
					cout<<"\tpop \t%rbx"<<endl;					// Get first operand
					cout<<"\tpop \t%rax"<<endl;					// Get second operand
					cout<<"\tsubq\t%rbx, %rax\t\t# SUB"<<endl;	// Substract both operands
					cout<<"\tpush\t%rax"<<endl;					// Store result
				}
				else {
					cout<<"\tfldl\t(%rsp)"<<endl;				// Store first operand in %st(0)
					cout<<"\tfldl\t8(%rsp)"<<endl;				// Store second operand in %st(0) (first operand is now in %st(1))
					cout<<"\tfsubp\t%st(0), %st(1)"<<endl;		// Substract %st(0) from %st(1) and store result in %st(1), then depile FPU stack (%st(0) now contains the result)
					cout<<"\taddq\t$8, %rsp"<<endl;				// Depile CPU's stack
					cout<<"\tfstpl\t(%rsp)"<<endl;				// Depile %st(0) and put it on top of the CPU stack
				}
				break;
			default:
				Error("additive operator expected.");
		}
	}
	return type1;
}

// Type := "INTEGER" | "BOOLEAN" | "DOUBLE" | "CHAR"
TYPES Type(void) {
	TYPES type;
	if (strcmp(lexer->YYText(),"INTEGER")==0) {
		type = INTEGER;
	}
	else if (strcmp(lexer->YYText(),"BOOLEAN")==0) {
		type = BOOLEAN;
	}
	else if (strcmp(lexer->YYText(),"DOUBLE")==0) {
		type = DOUBLE;
	}
	else if (strcmp(lexer->YYText(),"CHAR")==0) {
		type = CHAR;
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
	if (current!=ID) {
		Error("identifier expected.");
	}
	identifiers.insert(lexer->YYText());		// Store identifier in set
	current=(TOKEN)lexer->yylex();				// Consume identifier and advance to next token

	while(current==COMMA) {						// Loop to get all identifiers
		current=(TOKEN)lexer->yylex();			// Consume ',' and advance to next token
		if (current!=ID) {
			Error("identifier expected.");
		}
		identifiers.insert(lexer->YYText());	// Store identifier in set
		current=(TOKEN)lexer->yylex();			// Consume identifier and advance to next token
	}

	if (current!=COLON) {
		Error("':' expected.");
	}
	current=(TOKEN)lexer->yylex();				// Consume ':' and advance to next token

	TYPES type = Type();						// Get type of the variable
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
				Error("unknown type."); 
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
	if (current!=DOT) {							// Triggers an error if token is not '.'
		Error("'.' expected.");
	}
	current=(TOKEN)lexer->yylex();
}


// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
OPREL RelationalOperator(void) {
	OPREL oprel;
	if (strcmp(lexer->YYText(),"==")==0) {
		oprel=EQU;
	}
	else if (strcmp(lexer->YYText(),"!=")==0) {
		oprel=DIFF;
	}
	else if (strcmp(lexer->YYText(),"<")==0) {
		oprel=INF;
	}
	else if (strcmp(lexer->YYText(),">")==0) {
		oprel=SUP;
	}
	else if (strcmp(lexer->YYText(),"<=")==0) {
		oprel=INFE;
	}
	else if (strcmp(lexer->YYText(),">=")==0) {
		oprel=SUPE;
	}
	else {
		oprel=WTFR;
	}
	current=(TOKEN) lexer->yylex();
	return oprel;
}

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
enum TYPES Expression(void) {
	TYPES type1, type2;
	OPREL oprel;
	type1 = SimpleExpression();														// Get first simple expression and its type
	if (current==RELOP) {
		oprel=RelationalOperator(); 												// Save operator in local variable
		type2 = SimpleExpression();													// Get second simple expression and its type
		if (type1!=type2) {															// Triggers an error if the types are different
			Error("TYPES error: cannot compare different types.");
		}
		cout<<"\tpop \t%rax"<<endl;
		cout<<"\tpop \t%rbx"<<endl;
		cout<<"\tcmpq\t%rax, %rbx"<<endl;
		switch(oprel) {
			case EQU:
				cout<<"\tje  \tVrai"<<++TagNumber<<"\t\t# If equal"<<endl;			// Jump if equal
				break;
			case DIFF:
				cout<<"\tjne \tVrai"<<++TagNumber<<"\t\t# If different"<<endl;		// Jump if different
				break;
			case SUPE:
				cout<<"\tjae \tVrai"<<++TagNumber<<"\t\t# If above or equal"<<endl;	// Jump if above or equal
				break;
			case INFE:
				cout<<"\tjbe \tVrai"<<++TagNumber<<"\t\t# If below or equal"<<endl;	// Jump if below or equal
				break;
			case INF:
				cout<<"\tjb  \tVrai"<<++TagNumber<<"\t\t# If below"<<endl;			// Jump if below
				break;
			case SUP:
				cout<<"\tja  \tVrai"<<++TagNumber<<"\t\t# If above"<<endl;			// Jump if above
				break;
			default:
				Error("relational operator expected.");
		}
		cout<<"\tpush\t$0\t\t# False"<<endl;
		cout<<"\tjmp \tNext"<<TagNumber<<endl;
		cout<<"Vrai"<<TagNumber<<":\tpush\t$0xFFFFFFFFFFFFFFFF\t\t# True"<<endl;	
		cout<<"Next"<<TagNumber<<":"<<endl;
		return BOOLEAN;																// Return BOOLEAN if the expression is relational
	}
	return type1;																	// Return the type of the expression if not
}

// AssignementStatement := Identifier ":=" Expression
string AssignementStatement(void) {
	enum TYPES type1, type2;
	string variable;
	if (current!=ID)						// Triggers an error if token is not an identifier
		Error("identifier expected.");
	if (!IsDeclared(lexer->YYText())) {		// Triggers an error if the identifier is not declared
		cerr << "Error : variable '"<<lexer->YYText()<<"' is not declared."<<endl;
		Error(".");
	}
	variable=lexer->YYText();
	type1 = DeclaredVariables[variable];	// Get type of the variable
	current=(TOKEN) lexer->yylex();			// Consume identifier and advance to next token

	if (current!=ASSIGN) {					// Triggers an error if token is not ':='
		Error("':=' expected.");
	}
	current=(TOKEN) lexer->yylex();
	type2 = Expression();
	if (type1!=type2) {						// Triggers an error if the types are different
		Error("TYPES error: cannot assign different types.");
	}
	cout<<"\tpop \t"<<variable<<endl;
	return variable;						// Return the variables name
}

void Statement(void);	

// DisplayStatement := "DISPLAY" Expression
void DisplayStatement(void) {
	enum TYPES type;
	unsigned long localTag=++TagNumber;
	CheckReadKeyword("DISPLAY");											// Check if keyword is 'DISPLAY'
	type = Expression();

	cout<<"DISPLAY"<<localTag<<":"<<endl;									// Label for DISPLAY
	switch(type) {
		case INTEGER:
			cout<<"\tpop \t%rsi\t\t# Value to display"<<endl;
			cout<<"\tmovq\t$FormatString1, %rdi\t\t#%llu"<<endl;			// Get INTEGER format for printf
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<"\tpush\t%rbp\t\t# Save the value in %rbp (modified by printf)"<<endl;
			cout<<"\tcall\tprintf@PLT"<<endl;								// Call printf (will display the value)
			cout<<"\tpop \t%rbp\t\t# Restore %rbp value"<<endl;
			break;
		case BOOLEAN:
			cout<<"\tpop \t%rsi\t\t# Value to display"<<endl;
			cout<<"\tcmpq\t$0, %rsi"<<endl;									// Compare value to 0
			cout<<"\tje  \tFALSE"<<localTag<<endl;							// Jump to FALSE if value is 0
			cout<<"\tmovq\t$TrueString, %rdi\t\t# TRUE"<<endl;				// Get TRUE string for printf
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<"\tpush\t%rbp\t\t# Save the value in %rbp (modified by printf)"<<endl;
			cout<<"\tjmp \tDISPLAYend"<<localTag<<endl;
			cout<<"FALSE"<<localTag<<":"<<endl;								// Label for FALSE
			cout<<"\tmovq\t$FalseString, %rdi\t\t# FALSE"<<endl;			// Get FALSE string for printf
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<"\tpush\t%rbp\t\t# Save the value in %rbp (modified by printf)"<<endl;
			cout<<"DISPLAYend"<<localTag<<":"<<endl;
			cout<<"\tcall\tprintf@PLT"<<endl;								// Call printf (will display TRUE or FALSE)
			cout<<"\tpop \t%rbp\t\t# Restore %rbp value"<<endl;
			break;
		case CHAR:
			cout<<"\tpop \t%rsi\t\t\t# get character in the 8 lowest bits of %si"<<endl;
			cout<<"\tmovq\t$FormatString3, %rdi\t# \"%c\\n\""<<endl;		// Get CHAR format for printf
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<"\tpush\t%rbp\t\t# Save the value in %rbp (modified by printf)"<<endl;
			cout<<"\tcall\tprintf@PLT"<<endl;								// Call printf (will display the character)
			cout<<"\tpop \t%rbp\t\t# Restore %rbp value"<<endl;
			break;
		case DOUBLE:														// Code provided by Pierre Jourlin
			cout<<"\tmovsd\t(%rsp), %xmm0\t\t# &stack top -> %xmm0"<<endl;
			cout<<"\tsubq\t$16, %rsp\t\t# allocation for 3 additional doubles"<<endl;
			cout<<"\tmovsd\t%xmm0, 8(%rsp)"<<endl;
			cout<<"\tmovq\t$FormatString2, %rdi\t# \"%lf\\n\""<<endl;
			cout<<"\tmovq\t$1, %rax"<<endl;
			cout<<"\tcall\tprintf"<<endl;
			cout<<"nop"<<endl;
			cout<<"\taddq\t$24, %rsp\t\t\t# pop nothing"<<endl;
			break;
		default:
			cerr<<"Type: "<<type<<endl;
			Error("type cannot be displayed.");
	}
	cout<<"\tmovq\t$10, %rdi\t\t# ASCII code for newline character"<<endl;	// Get ASCII code for newline character
    cout<<"\tcall\tputchar@PLT"<<endl;										// Call putchar (will display newline character and effectively skip a line)
}

// IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
void IfStatement(void) {
	enum TYPES type;
	unsigned long localTag=++TagNumber;

	CheckReadKeyword("IF");
	cout<<"IF"<<localTag<<":"<<endl; 								// Label for IF
	type = Expression();
	if (type!=BOOLEAN) {
		Error("TYPES error: 'IF' expression must be boolean.");		// Triggers an error if the expression is not boolean in 'IF' statement
	}
	cout<<"\tpop \t%rax"<<endl;
	cout<<"\tcmpq\t$0, %rax"<<endl;
	cout<<"\tje \tIFfalse"<<localTag<<"\t\t# jump to ELSE "<<endl;	// Jump to ELSE if 'IF' expression is false (even if there is no else)

	CheckReadKeyword("THEN");
	cout<<"IFtrue"<<localTag<<":\t\t\t# THEN"<<endl; 				// Label for THEN
	Statement();
	cout<<"\tjmp \tIFend"<<localTag<<"\t\t# jump to endIf"<<endl;	// Jump to end of 'IF' statement
	cout<<"IFfalse"<<localTag<<":\t\t\t# ELSE"<<endl; 				// Label for ELSE (even if there is no else)

	if (current==KEYWORD && strcmp(lexer->YYText(),"ELSE")==0) {
		CheckReadKeyword("ELSE");
		Statement();
	}
	cout<<"IFend"<<localTag<<":"<<endl; 							// Label for end of 'IF' statement
}

// WhileStatement := "WHILE" Expression "DO" Statement
void WhileStatement(void) {
	unsigned long localTag=++TagNumber;

	CheckReadKeyword("WHILE");
	cout<<"WHILE"<<localTag<<":"<<endl; 									// Label for WHILE
	Expression();
	cout<<"\tpop \t%rax"<<endl;
	cout<<"\tcmpq\t$0, %rax"<<endl;
	cout<<"\tje \tWHILEend"<<localTag<<"\t\t# jump to end of WHILE"<<endl;	// Jump to end of 'WHILE' statement if expression is false

	CheckReadKeyword("DO");
	cout<<"WHILEtrue"<<localTag<<":\t\t\t# DO"<<endl; 						// Label for DO
	Statement();
	cout<<"\tjmp \tWHILE"<<localTag<<endl;									// Jump to 'WHILE' statement
	cout<<"WHILEend"<<localTag<<":"<<endl; 									// Label for end of 'WHILE' statement

}

// ForStatement := "FOR" AssignementStatement "TO" Expression "DO" Statement
void ForStatement(void) {
	unsigned long localTag=++TagNumber;
	enum TYPES type;
	cout<<"FOR"<<localTag<<":"; 												// Label for FOR
	CheckReadKeyword("FOR");

	string loop_var = AssignementStatement();
	if (DeclaredVariables[loop_var]!=INTEGER) {
		Error("TYPES error: loop variable must be integer.");					// Triggers an error if the loop variable is not integer
	}
	if(strcmp(lexer->YYText(),"TO")==0) {										// If keyword is 'TO'
		CheckReadKeyword("TO");	
		type=Expression();
		if(type!=INTEGER) {
			Error("TYPES error: 'TO' expression must be integer.");				// Triggers an error if the expression is not integer in 'TO' statement
		}
		cout<<"TO"<<localTag<<":"; 												// Label for TO
		cout<<"\tmovq\t(%rsp), %rax"<<endl; 									// Necessary to avoid 'too many memory references'
		cout<<"\tcmpq\t%rax, "<<loop_var<<endl;
		cout<<"\tjae \tFORend"<<localTag<<"\t\t# jump at the end of FOR"<<endl; // Jump at the end of 'FOR' statement if loop_var is above or equal expression

		CheckReadKeyword("DO");
		Statement();
		cout<<"\tincq\t"<<loop_var<<"\t\t# loop_var++"<<endl;
		cout<<"\tjmp \tTO"<<localTag<<endl;										// Jump to 'TO' statement
		cout<<"FORend"<<localTag<<":"<<endl; 									// Label for end of 'FOR' statement
	}
	else {																		// If keyword is 'DOWNTO'
		CheckReadKeyword("DOWNTO");
		type=Expression();
		if(type!=INTEGER) {
			Error("TYPES error: 'DOWNTO' expression must be integer.");			// Triggers an error if the expression is not integer in 'DOWNTO' statement
		}
		cout<<"DOWNTO"<<localTag<<":"; 											// Label for DOWNTO
		cout<<"\tmovq\t(%rsp), %rax"<<endl; 									// Necessary to avoid 'too many memory references'
		cout<<"\tcmpq\t%rax, "<<loop_var<<endl;
		cout<<"\tjbe \tFORend"<<localTag<<"\t\t# jump at the end of FOR"<<endl; // Jump at the end of 'FOR' statement if loop_var is below or equal expression

		CheckReadKeyword("DO");
		Statement();
		cout<<"\tdecq\t"<<loop_var<<"\t\t# loop_var--"<<endl;
		cout<<"\tjmp \tDOWNTO"<<localTag<<endl;									// Jump to 'DOWNTO' statement
		cout<<"FORend"<<localTag<<":"<<endl; 									// Label for end of 'FOR' statement
	}
}

// BlockStatement := "BEGIN" Statement { ";" Statement } "END"
void BlockStatement(void) {
	unsigned long localTag=++TagNumber;

	CheckReadKeyword("BEGIN");
	cout<<"BEGIN"<<localTag<<":"<<endl; 									// Label for BEGIN
	Statement();
	while(current==SEMICOLON) {
		current=(TOKEN) lexer->yylex();
		Statement();
	}

	CheckReadKeyword("END");
	cout<<"END"<<localTag<<":"<<endl; 										// Label for END
}

// CaseLabel := CharConst {"," CharConst} | {Digit}+ ".." {Digit}+ | Number {"," Number} | Identifier
enum TYPES CaseLabel(unsigned long localTag, unsigned long caseTag) {
	enum TYPES type1, type2;
	unsigned long loop, end_loop;						// Local variable to loop when necessary ({Digit}+ ".." {Digit}+)
	
	if (current==CHARCONST) {							// CharConst {"," CharConst}
		type1 = CharConst();
		do {
			cout<<"\tpop \t%rbx"<<endl;
			cout<<"\tpop \t%rax"<<endl;
			cout<<"\tcmpq\t%rax, %rbx"<<endl;
			cout<<"\tje  \tCaseStatement"<<"_"<<caseTag<<"_"<<localTag<<endl;
			cout<<"\tpush\t%rax\t\t # push 'CASE' Expression on the stack"<<endl;
			if (current!=COMMA) {
				break;
			}
			current=(TOKEN) lexer->yylex();				// Consume ',' and advance to next token
			if(current!=CHARCONST) {
				Error("character expected.");
			}
			type2 = CharConst();
		}
		while (true);
		cout<<"\tjmp \tendCaseElement_"<<caseTag<<"_"<<localTag<<endl;
		return CHAR;
	}
	else if (current==NUMBER) {							// Number {"," Number} | {Digit}+ ".." {Digit}+
		try {
			loop = atoi(lexer->YYText());				// Get first digit for loop
		} catch (invalid_argument e) {
			Error("INTEGER expected.");
		}
		type1 = Number();								// INTEGER or DOUBLE
		
		if (type1==INTEGER) {
			if (current==DOT) {							// {Digit}+ ".." {Digit}+
				current=(TOKEN) lexer->yylex();
				if (current!=DOT) {
					Error("'..' expected.");
				}
				current=(TOKEN) lexer->yylex();			// Consume '..' and advance to next token	
				try {
					end_loop = atoi(lexer->YYText());	// Get second digit for loop
				} catch (invalid_argument e) {
					Error("INTEGER expected.");
				}
				if (end_loop<=loop) {
					Error("second INTEGER must be stricly greater than the first one.");
				}
				cout<<"\tpop \t%rbx"<<endl;
				cout<<"\tpop \t%rax"<<endl;
				cout<<"\tcmpq\t%rax, %rbx"<<endl;
				cout<<"\tje  \tCaseStatement"<<"_"<<caseTag<<"_"<<localTag<<endl;
				cout<<"\tpush\t%rax\t\t # push 'CASE' Expression on the stack"<<endl;
				while (loop<end_loop) {					// Loop to get all numbers between loop and end_loop-1
					cout<<"\tpush\t$"<<++loop<<endl;
					cout<<"\tpop \t%rbx"<<endl;
					cout<<"\tpop \t%rax"<<endl;
					cout<<"\tcmpq\t%rax, %rbx"<<endl;
					cout<<"\tje  \tCaseStatement"<<"_"<<caseTag<<"_"<<localTag<<endl;
					cout<<"\tpush\t%rax\t\t # push 'CASE' Expression on the stack"<<endl;
				}
				type2 = Number();						// Will push end_loop on the stack
				if (type2!=INTEGER) {					// Should not happen
					Error("INTEGER expected.");
				}
				cout<<"\tpop \t%rbx"<<endl;
				cout<<"\tpop \t%rax"<<endl;
				cout<<"\tcmpq\t%rax, %rbx"<<endl;
				cout<<"\tje  \tCaseStatement"<<"_"<<caseTag<<"_"<<localTag<<endl;
				cout<<"\tpush\t%rax\t\t # push 'CASE' Expression on the stack"<<endl;
				cout<<"\tjmp \tendCaseElement_"<<caseTag<<"_"<<localTag<<endl;
				return INTEGER;		
			}
			else if (current==COLON || current==COMMA) {// {Digit}+ { "," {Digit}+ }
				cout<<"\tpop \t%rbx"<<endl;
				cout<<"\tpop \t%rax"<<endl;
				cout<<"\tcmpq\t%rax, %rbx"<<endl;
				cout<<"\tje  \tCaseStatement"<<"_"<<caseTag<<"_"<<localTag<<endl;
				cout<<"\tpush\t%rax\t\t # push 'CASE' Expression on the stack"<<endl;
				while(current==COMMA) {
					current=(TOKEN) lexer->yylex();		// Consume ',' and advance to next token
					if(current!=NUMBER) {
						Error("number expected.");
					}
					type2 = Number();
					cout<<"\tpop \t%rbx"<<endl;
					cout<<"\tpop \t%rax"<<endl;
					cout<<"\tcmpq\t%rax, %rbx"<<endl;
					cout<<"\tje  \tCaseStatement"<<"_"<<caseTag<<"_"<<localTag<<endl;
					cout<<"\tpush\t%rax\t\t # push 'CASE' Expression on the stack"<<endl;
					if (type1!=type2) {
						Error("Same type expected in list element.");
					}
				}
				cout<<"\tjmp \tendCaseElement_"<<caseTag<<"_"<<localTag<<endl;
				return type1;
			}
			else {
				Error("':' or ',' expected.");
			}
		}
		else if (type1==DOUBLE) {
			// INCOMPLETE
		}
	}
	else if (current==ID) {								// Identifier
		if (!IsDeclared(lexer->YYText())) {
			cerr << "Error : variable '"<<lexer->YYText()<<"' is not declared"<<endl;
			Error(".");
		}
		if (DeclaredVariables[lexer->YYText()]!=CHAR) {
			Error("CHAR expected.");
		}
		return DeclaredVariables[lexer->YYText()];
	}
	else {
		Error("case label expected (CHAR or NUMBER or IDENTIFIER).");
	}
	return WTFT;
}

// CaseListElement := CaseLabel ":" Statement
enum TYPES CaseListElement(unsigned long localTag, unsigned long caseTag) {
	enum TYPES type;
	type = CaseLabel(localTag, caseTag);
	if (current!=COLON) {
		Error("':' expected.");
	}
	current=(TOKEN) lexer->yylex();										// Consume ':' and advance to next token
	if (current==ID || current==KEYWORD) {
		cout<<"CaseStatement_"<<caseTag<<"_"<<localTag<<":"<<endl;		// Label for CASE statement
		Statement();
		cout<<"\tjmp \tENDCase"<<localTag<<endl;						// Jump to END of "CASE" statement
	}
	else {
		Error("statement expected.");
	}
	return type;
}

// CaseStatement := "CASE" Expression "OF" CaseListElement {";" CaseListElement} ["ELSE" Statement] "END"
void CaseStatement(void) {
	unsigned long localTag=++TagNumber;
	unsigned long caseTag=0;
	enum TYPES type1, type2;
	CheckReadKeyword("CASE");
	cout<<"CASE"<<localTag<<":"<<endl; 										// Label for CASE
	type1 = Expression();
	CheckReadKeyword("OF");
	cout<<"CaseElement_"<<++caseTag<<"_"<<localTag<<":"<<endl;
	type2 = CaseListElement(localTag, caseTag);
	cout<<"endCaseElement_"<<caseTag<<"_"<<localTag<<":"<<endl;				// Label for END of "CASE" element
	if (type1!=type2) {
		Error("TYPES error: 'CASE' expression and 'CASE' element must have the same type.");
	}
	while (current==SEMICOLON) {
		current=(TOKEN) lexer->yylex();										// Consume ';' and advance to next token
		cout<<"CaseElement_"<<++caseTag<<"_"<<localTag<<":"<<endl;
		type2 = CaseListElement(localTag, caseTag);
		cout<<"endCaseElement_"<<caseTag<<"_"<<localTag<<":"<<endl;			// Label for END of "CASE" element
		if (type1!=type2) {
			Error("TYPES error: 'CASE' expression and 'CASE' element must have the same type.");
		}
	}
	if (current!=KEYWORD) {
		Error("keyword expected (ELSE or END).");
	}
	if (strcmp(lexer->YYText(),"ELSE")==0) {
		CheckReadKeyword("ELSE");
		cout<<"ELSECase"<<localTag<<":"<<endl; 								// Label for ELSE
		cout<<"\tpop \t%rax"<<endl;											// 'CASE' Expression value is not useful anymore, so we pop it
		cout<<"\txor \t%rax, %rax\t\t# rax = 0"<<endl;						// Set rax to 0
		Statement();
	}
	CheckReadKeyword("END");
	cout<<"ENDCase"<<localTag<<":"<<endl; 									// Label for END
}

// Statement := AssignementStatement | IfStatement | WhileStatement | ForStatement | BlockStatement | DisplayStatement | CaseStatement
void Statement(void) {
	if (current==ID) {
		AssignementStatement();
	}
	else if (current==KEYWORD) {
		if (strcmp(lexer->YYText(),"DISPLAY")==0) {					// Check if keyword is 'DISPLAY'
			DisplayStatement();
		}
		else if (strcmp(lexer->YYText(),"IF")==0) {					// Check if keyword is 'IF'
			IfStatement();
		}
		else if (strcmp(lexer->YYText(), "WHILE")==0) {				// Check if keyword is 'WHILE'
			WhileStatement();
		}
		else if (strcmp(lexer->YYText(),"FOR")==0) {				// Check if keyword is 'FOR'
			ForStatement();
		}
		else if (strcmp(lexer->YYText(),"BEGIN")==0) {				// Check if keyword is 'BEGIN'
			BlockStatement();
		}
		else if (strcmp(lexer->YYText(),"CASE")==0) {				// Check if keyword is 'CASE'
			CaseStatement();
		}
		else {
			Error("keyword not identified (must be IF or WHILE or FOR or BEGIN or DISPLAY.)");
		}
	}
	else {
		Error("keyword or identifier expected.");
	}
}


// StatementPart := Statement {";" Statement} "."
void StatementPart(void) {
	cout<<"\t.text\t\t# The following lines contain the program"<<endl;
	cout<<"\t.globl main\t# The main function must be visible from outside"<<endl;
	cout<<"main:\t\t\t# The main function body :"<<endl;
	cout<<"\tmovq\t%rsp, %rbp\t# Save the position of the stack's top"<<endl;
	Statement();
	while(current==SEMICOLON) {
		current=(TOKEN) lexer->yylex();												// Consume ';' and advance to next token
		Statement();
	}
	if (current!=DOT) {
		Error("caractère '.' attendu");
	}
	current=(TOKEN) lexer->yylex();													// Consume '.' and advance to next token
}

// Program := [DeclarationPart] StatementPart
void Program(void){	
	cout<<"\t.data"<<endl;
    // cout<<"\t.align 8" << endl;
	cout<<"FormatString1:\t.string \"%llu\"\t# used by printf to display 64-bit unsigned integers"<<endl; 
	cout<<"FormatString2:\t.string \"%lf\"\t# used by printf to display 64-bit floating point numbers"<<endl; 
	cout<<"FormatString3:\t.string \"%c\"\t# used by printf to display a 8-bit single character"<<endl; 
	cout<<"TrueString: \t.string \"TRUE\"\t# used by printf to display the boolean value TRUE"<<endl; 
	cout<<"FalseString:\t.string \"FALSE\"\t# used by printf to display the boolean value FALSE"<<endl;
	VarDeclarationPart();
	StatementPart();	
}

int main(void){
	cout<<"\t\t\t\t# This code was produced by the compiler made by Elliot Pozucek"<<endl; 		// Header for the gcc assembler / linker
	current=(TOKEN) lexer->yylex();																// Get first token
	Program();
	
	cout<<"\n\tmovq\t%rbp, %rsp\t\t# Restore the position of the stack's top"<<endl;			// Trailer for the gcc assembler / linker
	cout<<"\tret\t\t\t# Return from main function"<<endl;
	if (current!=FEOF) {
		cerr<<"Unexpected characters at the end of the program: [" << current << "]";			// Unexpected characters at the end of the program
		Error("."); 
	}
}
		
			





