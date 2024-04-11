//  A compiler from a very simple Pascal-like structured language LL(k)
//  to 64-bit 80x86 Assembly langage
//  Copyright (C) 2019 Pierre Jourlin
//  Copyright (C) 2024 Elliot Pozucek 
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


// Grammar :

// Number := Digit{Digit}
// Letter := "a"|...|"z"
// AdditiveOperator := "+" | "-" | "||"
// MultiplicativeOperator := "*" | "/" | "%" | "&&"
// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  

// Expression := ArithmeticExpression [RelationalOperator ArithmeticExpression]
// ArithmeticExpression := Term {AdditiveOperator Term}
// Term := Factor {MultiplicativeOperator Factor}
// Factor := Number | Letter | "(" Expression ")"| "!" Factor

#include <string>
#include <iostream>
#include <cstdlib>

using namespace std;

// relational operators
// 0: '<' | 1: '>' | 2: '<=' | 3: '>=' | 4: '==' | 5: '!=' | 6: 'unknown'
int str_oprel[7] = {0,1,2,3,4,5,6};
// Current char, next char
char current, lookedAhead;				
int NlookedAhead = 0;

// ReadChar function
void ReadChar(void) {
	// Char has already been read
	if(NlookedAhead > 0){
		current = lookedAhead; 			
		NlookedAhead--;
	}
	else{
        // Read character and skip spaces until non space character is read
        while(cin.get(current) && (current==' '||current=='\t'||current=='\n'));
	}
}

// LookAhead function
void LookAhead(void) {
	while(cin.get(lookedAhead) && (lookedAhead==' '||lookedAhead=='\t'||lookedAhead=='\n'));
	NlookedAhead++;
}


// Error function
void Error(string s, char current){
	cerr<< s << current << endl;
	exit(-1);
}


// Letter := "a"|"b"|"c"|"d"|"e"|"f"|"g"|"h"|"i"|"j"|"k"|"l"|"m"|"n"|"o"|"p"|"q"|"r"|"s"|"t"|"u"|"v"|"w"|"x"|"y"|"z"
void Letter(void) {
	if((current<'a')||(current>'z'))
		Error("Letter expected", current);
	else{
		cout << "\tpush "<<current<<endl; 		// push letter on the stack
		ReadChar();
	}
}


// Number := Digit {Digit}
void Number(void) {
	unsigned long long number;
	if((current<'0')||(current>'9'))
		Error("Number expected", current);
	else{
		number = current - '0';
	}
	ReadChar();
	while((current>='0')&&(current<='9')){
		number = number*10 + current - '0';
		ReadChar();
	}
	cout << "\tpush $"<<number<<endl;		// push number on the stack
}


// MultiplicativeOperator := "*" | "/" | "%" | "&&"
void MultiplicativeOperator(void) {
	if(current!='*'&&current!='/'&&current!='%'&&current!='&') {
		Error("Multiplicative operator expected", current);
	}
	else if(current!='&') {
		ReadChar();
	}
	else {
		LookAhead();
		if(lookedAhead!='&') {
			Error("Multiplicative operator expected", current);
		}
		else {
			ReadChar();
			ReadChar();
		}
	}
}


// AdditiveOperator := "+" | "-" | "||"
void AdditiveOperator(void){
	if(current!='+'&&current!='-'&&current!='|') {
		Error("Additive operator expected", current);
	}
	else if(current!='|') {
		ReadChar();
	}
	else {
		LookAhead();
		if(lookedAhead!='|') {
			Error("Comparison with '||'", current);
		}
		else {
			ReadChar();
			ReadChar();
		}
	}
}


// RelationalOperator := "<" | ">" | "<=" | ">=" | "==" | "!=" | "unknown"
int RelationalOperator(void) {
	if((current!='>')&&(current!='<')&&(current!='!')&&(current!='=')){
		Error("Relational operator expected", current);
	}
	char oprel = current;
	LookAhead();
	if(lookedAhead=='='||lookedAhead=='>'){
		/* Error(" ", lookedAhead); */
		switch (lookedAhead)
		{
		case '=':
			if (oprel=='<'){
				ReadChar();
				ReadChar();
				return str_oprel[2];		// '<='
			}
			else if (oprel=='>'){
				ReadChar();
				ReadChar();
				return str_oprel[3];		// '>='
			}
			else if (oprel=='='){
				ReadChar();
				ReadChar();
				return str_oprel[4];		// '=='
			}
			else if (oprel=='!'){
				ReadChar();
				ReadChar();
				return str_oprel[5];		// '!='
			}
			else {
				// Should not happen
				Error("Relational operator expected", current);
			}
		case '>':
			if (oprel=='<'){
				ReadChar();
				ReadChar();	
				return str_oprel[5];		// '<>'
			}
			else {
				// Should not happen
				Error("Relational operator expected", current);
			}
		default:
			// Should not happen
			Error("Relational operator expected", current);
		}

	}
	else {
		switch (oprel) 
		{
		case '<':		// '<'
			ReadChar();
			return str_oprel[0];
		case '>':		// '>'
			ReadChar();
			return str_oprel[1];
		default:
			// Should not happen
			Error("Relational operator expected", current);
		}
	}
	ReadChar();
	return str_oprel[6];		// unknown operator
}


void ArithmeticExpression(void);			// Called by Term() and calls Term()

// Expression := ArithmeticExpression [RelationalOperator ArithmeticExpression]
void Expression(void) {
	int oprel;
	ArithmeticExpression(); 		// get first operand
	if((current=='<')||(current=='>')||(current=='!')||(current=='=')) {
		oprel = RelationalOperator(); 		// get relational operator
		ArithmeticExpression(); 		// get second operand

		cout << "\tpop %rax"<<endl; 		// get first operand
		cout << "\tpop %rbx"<<endl; 		// get second operand
		cout << "\tcmpq %rax, %rbx"<<endl;		// compare both operands

		switch (oprel)
		{
		case 0: 		// '<' 
			cout << "\tjb True\t\t # Jump if below" <<endl; 		// jump if below
			break;
		case 1: 		// '>'
			cout << "\tja True\t\t # Jump if above" <<endl; 		// jump if above
			break;
		case 2: 		// '<='
			cout << "\tjbe True\t # Jump if below or equal" <<endl; 		// jump if below or equal
			break;
		case 3: 		// '>='
			cout << "\tjae True\t # Jump if above or equal" <<endl; 		// jump if above or equal
			break;
		case 4: 		// '=='
			cout << "\tje True\t\t # Jump if equal" <<endl; 		// jump if equal
			break;
		case 5: 		// '!='
			cout << "\tjne True\t # Jump if not equal" <<endl; 		// jump if not equal
			break;
		case 6: 		// unknown operator
			Error("Unknown operator", current);
		default: 		// should not happen
			Error("Relational operator expected", current);
		}

		cout << "\tjmp False" <<endl; 		// jump to False if no condition is met
		cout << "False:\tPush $0\t# Faux" <<endl; 		// False
		cout << "\tjmp EndExp" <<endl; 		// jump to EndExp
		cout << "True:\tPush $-1\t# Vrai" <<endl; 		// True
		cout << "EndExp:" <<endl; 		// EndExp
	}
}


// Factor := Number | "(" Expression ")"
void Factor(void) {
	if(current=='('){
		ReadChar();
		Expression();
		if(current!=')')
			Error("')' was expected", current);
		else
			ReadChar();
	}
	else {
		if (current>='0' && current <='9') {
			Number();
		}
		else {
			if(current>='a'&& current<='z') {
				Letter();
			}
			else {
				Error("'(' or number ou letter expected", current);
			}
		}
	}
}


// Term := Factor {MultiplicativeOperator Factor}
void Term(void) {
	char mulop;
	Factor();
	while(current=='*'||current=='/'||current=='%'||current=='&') {
		mulop=current;		// Save operator in local variable
		MultiplicativeOperator();
		Factor();

		cout << "\tpop %rbx"<<endl;	// get first operand
		cout << "\tpop %rax"<<endl;	// get second operand

		switch (mulop)
		{
		case '*':		// '*'
		case '&':		// '&&'
			cout << "\timulq	%rbx, %rax"<<endl;	// %rax * %rbx -> %rdx:%rax
			cout << "\tpush %rax"<<endl;		// store result
			break;
		case '/':		// '/'
			cout << "\tmovq $0, %rdx"<<endl;	// clear %rdx (high part of numerator)
			cout << "\tdivq	%rbx"<<endl;		// %rdx:%rax / %rbx -> %rax (quotient goes in %rax)
			cout << "\tpush %rax"<<endl;		// store result
			break;
		case '%':		// '%
			cout << "\tmovq $0, %rdx"<<endl;	// clear %rdx (high part of numerator)
			cout << "\tdivq	%rbx"<<endl;		// %rdx:%rax / %rbx -> %rdx (remainder goes in %rdx)
			cout << "\tpush %rdx"<<endl;		// store result
			break;
		default:
			Error("Multiplicative operator expected", current);
		}
	}
}


// ArithmeticExpression := Term {AdditiveOperator Term}
void ArithmeticExpression(void){
	char adop;
	Term();
	while(current=='+'||current=='-'||current=='|') {
		adop=current;		// Save operator in local variable
		AdditiveOperator();
		Term();

		cout << "\tpop %rbx"<<endl;	// get first operand
		cout << "\tpop %rax"<<endl;	// get second operand

		if(adop=='+'||adop=='|')
			cout << "\taddq	%rbx, %rax"<<endl;	//  %rax + %rbx -> %rax
		else
			cout << "\tsubq	%rbx, %rax"<<endl;	// %rax - %rbx -> %rax
		cout << "\tpush %rax"<<endl;			// store result
	}
}


int main(void) {
	// Header for gcc assembler / linker
	cout << "\t\t\t# This code was produced by the CERI Compiler"<<endl;
	cout << "\t.text\t\t# The following lines contain the program"<<endl;
	cout << "\t.globl main\t# The main function must be visible from outside"<<endl;
	cout << "main:\t\t\t# The main function body :"<<endl;
	cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top"<<endl;

	// Let's proceed to the analysis and code production
	ReadChar();
	Expression();
	ReadChar();
	// Trailer for the gcc assembler / linker
	cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top"<<endl;
	cout << "\tret\t\t\t# Return from main function"<<endl;
	if(cin.get(current)){
		cerr <<"Expecting end of programm : char found ["<<current<<"]";
		Error(".",' '); // unexpected characters at the end of program
	}

}
		
			





