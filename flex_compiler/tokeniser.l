%{
// This is our Lexical tokeniser 
// It should be compiled into cpp with :
// flex++ -d -otokeniser.cpp tokeniser.l 
// And then compiled into object with
// g++ -c tokeniser.cpp
// tokens can be read using lexer->yylex()
// lexer->yylex() returns the type of the lexicon entry (see enum TOKEN in tokeniser.h)
// and lexer->YYText() returns the lexicon entry as a string

#include "tokeniser.h"
#include <iostream>

using namespace std;

%}

%option noyywrap
%option c++
%option yylineno

charconst  \'\\?.\'
boolconst (TRUE|FALSE)
ws      [ \t\n\r]+
alpha   [A-Za-z]
digit   [0-9]
number  {digit}+(\.{digit}+)?
id	{alpha}({alpha}|{digit})*
keyword (IF|THEN|ELSE|WHILE|DO|FOR|TO|DOWNTO|BEGIN|END|BOOLEAN|INTEGER|DOUBLE|VAR|DISPLAY|CASE|OF)
addop	(\+|\-|\|\|)
mulop	(\*|\/|%|\&\&)
relop	(\<|\>|"=="|\<=|\>=|!=)
unknown [^\"A-Za-z0-9 \n\r\t\(\)\<\>\=\!\%\&\|\}\-\;\.]+

%%

{keyword}	{ return KEYWORD; }
{boolconst} { return BOOLCONST; }
{id}		{ return ID; }
{number}	{ return NUMBER; }
{charconst} { return CHARCONST; }
{addop}		{ return ADDOP; }
{mulop}		{ return MULOP; }
{relop}		{ return RELOP; }
"["		{ return LBRACKET; }
"]"		{ return RBRACKET; }
","		{ return COMMA; }
";"		{ return SEMICOLON; }
":"		{ return COLON; }
"."		{ return DOT; }
":="	{ return ASSIGN; }
"("		{ return LPARENT; }
")"		{ return RPARENT; }
"!"		{ return NOT; }
{ws}    { /* skip blanks and tabs */ }
"(*"    { /* Skip comments between '(*' and '*)' */
		int c;
		while((c = yyinput()) != 0){
     			if(c == '*'){
     	    			if((c = yyinput()) == ')')
    	        			break;
    	     			else
  	          			unput(c);
  	   		}	
		}
	}
<<EOF>>	{ return FEOF; }
{unknown}	{ return UNKNOWN; }

%%