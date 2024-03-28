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

// Build with "make compilateur"


#include <string>
#include <iostream>
#include <cstdlib>

using namespace std;

char current, lookedAhead;				// Current char, next char
int NlookedAhead = 0;


// ReadChar function
void ReadChar(void) {
	if(NlookedAhead > 0){
		current = lookedAhead; 		// Char has already been read
		NlookedAhead--;
	}
	else{
        // Read character and skip spaces until 
        // non space character is read
        while(cin.get(current) && (current==' '||current=='\t'||current=='\n'));
	}
}

// LookAhead function
void LookAhead(void) {
	while(cin.get(lookedAhead) && (lookedAhead==' '||lookedAhead=='\t'||lookedAhead=='\n'));
	NlookedAhead++;
}

void Error(string s){
	cerr<< s << endl;
	exit(-1);
}


// AdditiveOperator := "+" | "-"	
void AdditiveOperator(void){
	if(current=='+'||current=='-')
		ReadChar(); 	// Read next character
	else
		Error("Additive operator expected");
}
		

// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
void Digit(void){
	if((current<'0')||(current>'9'))
		Error("Digit expected");
	else{
		cout << "\tpush $"<<current<<endl; 		// push digit on the stack
		ReadChar();
	}
}


// RelationalOperator := "<" | ">" | "=" | "!"
// next step : add "<=" | ">=" | "and replace "!" by "<>" and "=" "=="
char RelationalOperator(void) {
	if((current!='>')&&(current!='<')&&(current!='!')&&(current!='='))
		Error("Relational operator expected");
	char oprel = current;
	ReadChar();
	return oprel; 		// return relational operator
}

void ArithmeticExpression(void);			// Called by Term() and calls Term()

// Expression := ArithmeticExpression [RelationalOperator ArithmeticExpression]
void Expression(void) {
	char oprel;
	ArithmeticExpression(); 		// get first operand
	if((current=='<')||(current=='>')||(current=='!')||(current=='=')) {
		oprel = RelationalOperator(); 		// get relational operator
		ArithmeticExpression(); 		// get second operand

		cout << "\tpop %rax"<<endl; 		// get first operand
		cout << "\tpop %rbx"<<endl; 		// get second operand
		cout << "\tcmpq %rax, %rbx"<<endl;		// compare both operands

			char temp = current;
			ReadChar();
		switch (oprel)
		{
		case '<': // '<' | '<=' | '<>'
			if(current=='='){
				cout << "\tjbe True" <<endl; 		// jump if below or equal
			} else if(current=='>'){
				cout << "\tjne True" <<endl; 		// jump if not equal
			} else{
				current = temp;						// put back the character
				cout << "\tjb True" <<endl; 		// jump if below
			}
			break;
		case '>': // '>' | '>='
			if(current=='='){
				cout << "\tjae True" <<endl; 		// jump if above or equal
			} else{
				current = temp;						// put back the character
				cout << "\tja True" <<endl; 		// jump if above
			}
			break;
		case '=': // '=='
		if(current=='='){
				cout << "\tje True" <<endl; 		// jump if equal
			} else{
				Error("Relational operator expected");
			}
			break;
		case '!': // '!='
		if(current=='='){
				cout << "\tjne True" <<endl; 		// jump if not equal
			} else{
				Error("Relational operator expected");
			}
			break;
		default: 		// should not happen
			Error("Relational operator expected");
			break;
		}

		cout << "\tjmp False" <<endl; 		// jump to False if no condition is met
		cout << "False:\tPush $0\t# Faux" <<endl; 		// False
		cout << "\tjmp EndExp" <<endl; 		// jump to EndExp
		cout << "True:\tPush $-1\t# Vrai" <<endl; 		// True
		cout << "EndExp:" <<endl; 		// EndExp
	}
}


// Term := Digit | "(" ArithmeticExpression ")"
void Term(void){
	if(current=='('){
		ReadChar();
		ArithmeticExpression();
		if(current!=')')
			Error("')' was expected");
		else
			ReadChar();
	}
	else 
		if (current>='0' && current <='9')
			Digit();
	    else
			Error("'(' or digit expected");
}


// ArithmeticExpression := Term {AdditiveOperator Term}
void ArithmeticExpression(void){
	char adop;
	Term();
	while(current=='+'||current=='-'){
		adop=current;		// Save operator in local variable
		AdditiveOperator();
		Term();
		cout << "\tpop %rbx"<<endl;	// get first operand
		cout << "\tpop %rax"<<endl;	// get second operand
		if(adop=='+')
			cout << "\taddq	%rbx, %rax"<<endl;	// add both operands
		else
			cout << "\tsubq	%rbx, %rax"<<endl;	// substract both operands
		cout << "\tpush %rax"<<endl;			// store result
	}

}

int main(void){	// First version : Source code on standard input and assembly code on standard output
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
		Error("."); // unexpected characters at the end of program
	}

}
		
			





