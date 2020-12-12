#ifndef MACHINE_H
#define MACHINE_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <iomanip>
#include "class.h"
#include "lexer.h"

Reader Prepare_primary(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Primary>' ::= ( <IDs_checker> ) | <Empty>\n";
	}

	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() == "(") {
		latest = IDs_checker(out, source, Lexer_call(out, source), false, "");
	}

	else {
		return latest;
	}

	if (latest.getLexeme() == ")") {
		std::cerr << "Syntax Error: Expected " << ")" << " on line " << line << "\n";
		std::cerr << "Received " << latest.getToken() << " \"" << latest.getLexeme() << "\"\n";
		exit(1);
	}

	else{
		return Lexer_call(out, source);
	}
}

Reader Primary(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Primary> ::= <Identifier> <Primary>' | <Integer> | ( <Expression> ) | <Real> | true | false\n";
	}

	if (latest.getToken() == "identifier") {
		arithmetic_Table.push_back(latest);
		token.push_back(instr(instr_address, "PUSHM", get_address(latest.getLexeme())));
		instr_address++;
		return Prepare_primary(out, source);
	}

	else if (latest.getLexeme() == "(") {
		latest = Expression(out, source, Lexer_call(out, source));
		if (latest.getLexeme() == ")") {
			return Lexer_call(out, source);
		}

		else {
			std::cerr << "Syntax Error: Expected " << ")" << " on line " << line << "\n";
			exit(1);
		}
	}

	else if (latest.getToken() == "real"){
		return Lexer_call(out, source);
	}

	else if (latest.getToken() == "int") {
		arithmetic_Table.push_back(latest);
		token.push_back(instr(instr_address, "PUSHI", latest.getLexeme()));
		instr_address++;
		return Lexer_call(out, source);
	}

	else if (latest.getLexeme() == "true" || latest.getLexeme() == "false") {
		if (latest.getLexeme() != "true"){
			token.push_back(instr(instr_address, "PUSHM", "0"));
			instr_address++;
		}

		else{
			token.push_back(instr(instr_address, "PUSHM", get_address(latest.getLexeme())));
			instr_address++;
		}
		return Lexer_call(out, source);
	}

	else{
		std::cerr << "Syntax Error: Expected " << "identifier or int or (or real or true or false)" << " on line " << line << "\n";
		exit(1);
	}
}

Reader Factor(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Factor> ::= - <Primary> | <Primary>\n";
	}

	if (latest.getLexeme() == "-"){
		return Primary(out, source, Lexer_call(out, source));
	}

	else{
		return Primary(out, source, latest);
	}
}

Reader prepare_term(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Term>' ::= * <Factor> <Term>' | / <Factor> <Term>' | <Empty>\n";
	}
	Reader save = latest;
	if (latest.getLexeme() == "*" || latest.getLexeme() == "/") {
		latest = Factor(out, source, Lexer_call(out, source));
		arithmetic_Check();
		if (save.getLexeme() == "*"){
			token.push_back(instr(instr_address, "MUL", "NULL"));
			instr_address++;
		}

		else if (save.getLexeme() == "/"){
			token.push_back(instr(instr_address, "DIV", "NULL"));
			instr_address++;
		}
		return prepare_term(out, source, latest);
	}

	else{
		return latest;
	}
}

Reader Term(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Term> ::= <Factor> <Term>'\n";
	}
	latest = Factor(out, source, latest);
	return prepare_term(out, source, latest);
}

Reader Prepare_Expression(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Expression>' ::= + <Term> <Expression>' | - <Term> <Expression>' | <Empty>\n";
	}
	std::string save = latest.getLexeme();
	if (latest.getLexeme() == "+" || latest.getLexeme() == "-") {

		latest = Term(out, source, Lexer_call(out, source));
		arithmetic_Check();

		if (save == "+") {
			token.push_back(instr(instr_address, "ADD", "NULL"));
			instr_address++;
		}

		else if (save == "-"){
			token.push_back(instr(instr_address, "SUB", "NULL"));
		  instr_address++;
		}

		return Prepare_Expression(out, source, latest);
	}
	else {
		return latest;
	}
}

Reader Expression(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Expression> ::= <Term> <Expression>'\n";
	}
	latest = Term(out, source, latest);
	return Prepare_Expression(out, source, latest);
}


void Relop(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Relop> ::= == | != | > | < | <= | =>\n";
	}

	if (latest.getLexeme() == "==" || latest.getLexeme() == "!=" || latest.getLexeme() == ">" || latest.getLexeme() == "<" || latest.getLexeme() == "<=" || latest.getLexeme() == "=>"){
		return;
	}
}

Reader Set_conditions(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Condition> ::= <Expression> <Relop> <Expression>\n";
	}

	Reader latest = Expression(out, source, Lexer_call(out, source));
	Relop(out, source, latest);
	Reader save = Expression(out, source, Lexer_call(out, source));

	if (latest.getLexeme() == "==") {
		token.push_back(instr(instr_address, "EQU", "NULL"));
		instr_address++;
		push_stack(instr_address--);
		token.push_back(instr(instr_address, "JUMPZ", "NULL"));
		instr_address++;
	}

	else if (latest.getLexeme() == "!=") {
		token.push_back(instr(instr_address, "NEQ", "NULL"));
		instr_address++;
		push_stack(instr_address--);
		token.push_back(instr(instr_address, "JUMPZ", "NULL"));
		instr_address++;
	}

	else if (latest.getLexeme() == "<") {
		token.push_back(instr(instr_address, "LES", "NULL"));
		instr_address++;
		push_stack(instr_address--);
		token.push_back(instr(instr_address, "JUMPZ", "NULL"));
		instr_address++;
	}

	else if (latest.getLexeme() == "<=") {
		token.push_back(instr(instr_address, "LES", "NULL"));
		instr_address++;
		push_stack(instr_address--);
		token.push_back(instr(instr_address, "JUMPZ", "NULL"));
		instr_address++;
	}

	else if (latest.getLexeme() == ">") {
		token.push_back(instr(instr_address, "GRT", "NULL"));
		instr_address++;
		push_stack(instr_address--);
		token.push_back(instr(instr_address, "JUMPZ", "NULL"));
		instr_address++;
	}

	else if (latest.getLexeme() == ">=") {
		token.push_back(instr(instr_address, "GRT", "NULL"));
		instr_address++;
		push_stack(instr_address--);
		token.push_back(instr(instr_address, "JUMPZ", "NULL"));
		instr_address++;
	}

	return save;

}

void While_statement(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<While_statement> ::= while ( <Set_conditions> ) <Statement_reader_checker>\n";
	}
	std::string addr = std::to_string(instr_address);
	token.push_back(instr(instr_address, "LABEL", "NULL"));
	instr_address++;
	Lexeme_Check(out, source, "(");
	Reader latest = Set_conditions(out, source);

	if (latest.getLexeme() != ")") {
		std::cerr << "Error: Expected " << ")" << " on line " << line << "\n";
		exit(1);
	}
	Statement_reader_checker(out, source, Lexer_call(out, source));
	token.push_back(instr(instr_address, "JUMP", addr));
	instr_address++;
	int addr1 = pop_stack();
	token[addr1].setOprnd(std::to_string(instr_address));
	return;
}

void Read_files_through(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Read_files_through> ::= get ( <IDs_checker> );\n";
	}

	Lexeme_Check(out, source, "(");
	Reader latest = Lexer_call(out, source);
	if (file){
		out << "\t<IDs_checker> ::= <Identifier> <IDs_checker>'\n";
	}

	if (latest.getToken() != "identifier"){
		std::cerr << "Syntax Error: Expected " << "an identifier" << " on line " << line << "\n";
		exit(1);

	while (latest.getToken() == "identifier" || latest.getLexeme() == ",") {
		if (latest.getToken() == "identifier") {
			token.push_back(instr(instr_address, "STDIN", "NULL"));
			instr_address++;
			token.push_back(instr(instr_address, "POPM", get_address(latest.getLexeme())));
			instr_address++;
		}
		latest = Lexer_call(out, source);
	}

	if (latest.getLexeme() != ")") {
		std::cerr << "Syntax Error: Expected " << ")" << " on line " << line << "\n";
		std::cerr << "Received " << latest.getToken() << " \"" << latest.getLexeme() << "\"\n";
		exit(1);
	}

	Lexeme_Check(out, source, ";");
}
}

void Print(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Print> ::= put ( <Expression> );\n";
	}

	Lexeme_Check(out, source, "(");
	Reader latest = Expression(out, source, Lexer_call(out, source));
	if (latest.getLexeme() != ")") {
		std::cerr << "Error: Expected " << ")" << " on line " << line << "\n";
		exit(1);
	}
	token.push_back(instr(instr_address, "STDOUT", "NULL"));
	instr_address++;
	Lexeme_Check(out, source, ";");

}

Reader Prepare_IDs(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<IDs_checker>' ::= ,  <IDs_checker>  |  <Empty>'\n";
	}
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() == ","){
		return IDs_checker(out, source, Lexer_call(out, source));
	}

	else{
		return latest;
	}
}

Reader Prepare_IDs(std::ofstream& out, std::ifstream& source, bool make, std::string a) {
	if (file){
		out << "\t<IDs_checker>' ::= ,  <IDs_checker>  |  <Empty>'\n";
	}
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() == ","){
		return IDs_checker(out, source, Lexer_call(out, source), make, a);
	}

	else{
		return latest;
	}
}

Reader IDs_checker(std::ofstream& out, std::ifstream& source, Reader latest, bool make, std::string a) {
	if (file){
		out << "\t<IDs_checker> ::= <Identifier> <IDs_checker>'\n";
	}

	if (latest.getToken() != "identifier"){
		std::cerr << "Error: Expected " << "an identifier" << " on line " << line << "\n";
		exit(1);
	}

	else if (make) {
		std::vector<std::string>::const_iterator word;
		word = std::find(keywords.begin(), keywords.end(), a);
		latest.setType(*word);
		make_Sym(latest);
	}

	else {
		arithmetic_Table.push_back(latest);
		token.push_back(instr(instr_address, "PUSHM", get_address(latest.getLexeme())));
		instr_address++;
	}

	return Prepare_IDs(out, source, make, a);
}

Reader IDs_checker(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file) {
		out << "\t<IDs_checker> ::= <Identifier> <IDs_checker>'\n";
	}

	if (latest.getToken() != "identifier"){
		std::cerr << "Syntax Error: Expected " << "an identifier" << " on line " << line << "\n";
		exit(1);
	}
	return Prepare_IDs(out, source);
}

void statement_Body_checker(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<statement_Body_checker> ::= { < Statement_reader_checker List> }\n";
	}

	if (latest.getLexeme() != "{") {
		std::cerr << "Syntax Error: Expected " << "{" << " on line " << line << "\n";
		exit(1);
	}
	latest = Lexer_call(out, source);
	latest = State_List(out, source, latest);
	if (latest.getLexeme() != "}") {
		std::cerr << "Syntax Error: Expected " << "}" << " on line " << line << "\n";
		exit(1);
	}
}

void Statement_Qualifier_checker(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Statement_Qualifier_checker> ::= real | int | bool\n";
	}

	if (latest.getLexeme() == "real" || latest.getLexeme() == "int" || latest.getLexeme() == "boolean"){
		return;
	}

	else{
		std::cerr << "Syntax Error: Expected " << "real, int, or bool" << " on line " << line << "\n";
		exit(1);
	}
}

Reader declaration(std::ofstream& out, std::ifstream& source, Reader a) {
	if (file){
		out << "\t<Read_Parameter> ::= <Statement_Qualifier_checker> <IDs_checker>\n";
	}
	Statement_Qualifier_checker(out, source, a);
	Reader latest = Lexer_call(out, source);

	return IDs_checker(out, source, latest, true, a.getLexeme());
}

void Read_Parameter(std::ofstream& out, std::ifstream& source, Reader a) {
	if (file){
		out << "\t<Read_Parameter> ::= <IDs_checker> <Statement_Qualifier_checker>\n";
	}
	Reader latest = IDs_checker(out, source, a);
	Statement_Qualifier_checker(out, source, latest);
}

Reader prepare_declartion_list(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Declaration List>\' ::= <Declaration List>  |  <Empty>\n";
	}
	if (latest.getLexeme() == "int" || latest.getLexeme() == "boolean" || latest.getLexeme() == "real"){
		return declartion_list(out, source, latest);
	}
	else{
		return latest;
	}
}

Reader declartion_list(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Declaration List> ::= <Declaration> ; <Declaration List>\'\n";
	}
	Reader l = declaration(out, source, latest);
	if (l.getLexeme() != ";"){
		std::cerr << "Syntax Error: Expected " << ";" << " on line " << line << "\n";
		std::cerr << "Received " << latest.getToken() << " \"" << latest.getLexeme() << "\"\n";
		exit(1);
	}
	return prepare_declartion_list(out, source, Lexer_call(out, source));
}

Reader prepare_Parameter_list(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Read_Parameter List>\' ::= ,  <Read_Parameter List>  |  <Empty>\n";
	}
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() == ","){
		return Parameter_List(out, source, Lexer_call(out, source));
	}
	else{
		return latest;
	}
}

Reader Parameter_List(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Read_Parameter List> ::= <Read_Parameter> <Read_Parameter List>'\n";
	}
	Read_Parameter(out, source, latest);
	return prepare_Parameter_list(out, source);
}

Reader prepare_state_list(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Statement_reader_checker List>\' ::= <Statement_reader_checker List>  |  <Empty>\n";
	}

	if (latest.getToken() == "identifier") {
		return State_List(out, source, latest);
	}
	else if (latest.getLexeme() == "{") {
		return State_List(out, source, latest);
	}
	else if (latest.getLexeme() == "if") {
		return State_List(out, source, latest);
	}
	else if (latest.getLexeme() == "return") {
		return State_List(out, source, latest);
	}
	else if (latest.getLexeme() == "get") {
		return State_List(out, source, latest);
	}
	else if (latest.getLexeme() == "while") {
		return State_List(out, source, latest);
	}
	else if (latest.getLexeme() == "put") {
		return State_List(out, source, latest);
	}
	else {
		return latest;
	}
}

Reader State_List(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Statement_reader_checker List> ::= <Statement_reader_checker> <Statement_reader_checker List>\'\n";
	}
	Statement_reader_checker(out, source, latest);

	return prepare_state_list(out, source, Lexer_call(out, source));
}

Reader OPL(std::ofstream& out, std::ifstream& source) {
	if (file) {
		out << "\t<Opt Read_Parameter List> ::= <Read_Parameter List> | <Empty>\n";
	}
	Reader latest = Lexer_call(out, source);

	if (latest.getToken() == "identifier") {
		return Parameter_List(out, source, latest);
	}

	return latest;
}

Reader Opt_Declaration_List(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Opt Declaration List> ::= <Declaration List> | <Empty>\n";
	}

	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() == "int") {
		return declartion_list(out, source, latest);
	}

	else if (latest.getLexeme() == "boolean") {
		return declartion_list(out, source, latest);
	}

	else if (latest.getLexeme() == "real") {
		return declartion_list(out, source, latest);
	}

	else {
		return latest;
	}

}

void Function_checker(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Function> ::= function <Identifier> ( <Opt Read_Parameter List> ) <Opt Declaration List> <statement_Body_checker>\n";
	}

	Reader latest = Lexer_call(out, source);
	latest = IDs_checker(out, source, latest);

	if (latest.getLexeme() != "(") {
		std::cerr << "Syntax Error: Expected " << "(" << " on line " << line << "\n";
		exit(1);
	}
	if (latest.getLexeme() != ")") {
		std::cerr << "Syntax Error: Expected " << ")" << " on line " << line << "\n";
		exit(1);
	}
	latest = OPL(out, source);
	latest = Opt_Declaration_List(out, source);
	statement_Body_checker(out, source, latest);
}

Reader prepare_function_definition(std::ofstream& out, std::ifstream& source) {
	if (file) {
		out << "\t<Function Definitions>' ::= <Function Definitions> | <Empty>\n";
	}

	Reader latest = Lexer_call(out, source);

	if (latest.getLexeme() == "function") {
		return function_definition(out, source);
	}

	else {
		return latest;
	}
}

Reader function_definition(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Function Definitions> ::= <Function> <Function Definitions>'\n";
	}
	Function_checker(out, source);
	return prepare_function_definition(out, source);
}

Reader OFD(std::ofstream& out, std::ifstream& source) {

	if (file){
		out << "\t<Opt Function Definitions> ::= <Function Definitions> | <Empty>\n";
	}
	Reader latest = Lexer_call(out, source);
	if (latest.getLexeme() == "function") {
		return function_definition(out, source);
	}
	else{
		return latest;
	}
}

void Statement_reader_checker(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Statement_reader_checker ::= <Assign> | <Compound> | <If> | <While_statement> | <Print> | <Read_files_through> | <Return>\n";
	}

  if (latest.getToken() == "identifier"){
		Assign(out, source, latest);
	}

	else if (latest.getLexeme() == "{") {
		Compound(out, source);
	}

	else if (latest.getLexeme() == "if") {
		If_Statement(out, source);
	}

	else if (latest.getLexeme() == "while") {
		While_statement(out, source);
	}

	else if (latest.getLexeme() == "put") {
		Print(out, source);
	}

	else if (latest.getLexeme() == "get") {
		Read_files_through(out, source);
	}

	else if (latest.getLexeme() == "return") {
		Return(out, source);
	}

	else{
		std::cerr << "Syntax Error: Expected " << "{ or identifier or if or return or put or get or while" << " on line " << line << "\n";
		exit(1);
	}

}

void Compound(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Compound> ::= { <Statement_reader_checker List> }\n";
	}
	Reader latest = State_List(out, source, Lexer_call(out, source));
	if (latest.getLexeme() != "}"){
		std::cerr << "Syntax Error: Expected " << "}" << " on line " << line << "\n";
		exit(1);
	}
}

void Assign(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<Assign> :: <Identifier> = <Expression>; \n";
	}

	std::string save = latest.getLexeme();
	Lexeme_Check(out, source, "=");

	latest = Expression(out, source, Lexer_call(out, source));
	token.push_back(instr(instr_address, "POPM", get_address(save)));
	instr_address++;

	if (latest.getLexeme() != ";"){
		std::cerr << "Syntax Error: Expected " << ";" << " on line " << line << "\n";
		exit(1);
	}

}

void If_Statement(std::ofstream& out, std::ifstream& source) {
	if (file) {
		out << "\t<If> ::= if ( <Set_conditions> ) <Statement_reader_checker> <If>'\n";
	}

	Lexeme_Check(out, source, "(");
	Reader latest = Set_conditions(out, source);

	if (latest.getLexeme() != ")"){
		std::cerr << "Syntax Error: Expected " << ")" << " on line " << line << "\n";
		exit(1);
	}

	Statement_reader_checker(out, source, Lexer_call(out, source));
	int addr = instr_address;
	int addr1 = pop_stack();

	token[addr1].setOprnd(std::to_string(addr));
	If_Prime(out, source, Lexer_call(out, source));
	token.push_back(instr(instr_address, "LABEL", ""));
	instr_address++;

}

void If_Prime(std::ofstream& out, std::ifstream& source, Reader latest) {
	if (file){
		out << "\t<If>' ::= fi | else <Statement_reader_checker> fi\n";
	}

	if (latest.getLexeme() == "fi") {
		return;
	}

	else if (latest.getLexeme() == "else") {
		Statement_reader_checker(out, source, Lexer_call(out, source));
		Lexeme_Check(out, source, "fi");
	}
}

void Return(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Return> ::= return <Return>'\n";
	}
	Return_Prime(out, source);
	return;
}

void Return_Prime(std::ofstream& out, std::ifstream& source) {
	if (file) {
		out << "\t<Return>' ::= ; | <Expression>;\n";
	}

	Reader latest = Lexer_call(out, source);

	if (latest.getLexeme() != ";"){
		latest = Expression(out, source, latest);
	}

	else {
		return;
	}

	if (latest.getLexeme() != ";") {
		std::cerr << "Syntax Error: Expected " << ";" << " on line " << line << "\n";
		exit(1);
	}

}

void Rat20F(std::ofstream& out, std::ifstream& source);

void Rat20F(std::ofstream& out, std::ifstream& source) {
	if (file){
		out << "\t<Rat20F> ::= $$ <Opt Declaration List> <Statement_reader_checker List> $$\n";
	}

	Lexeme_Check(out, source, "$$");

	Reader latest = Opt_Declaration_List(out, source);

	latest = State_List(out, source, latest);
	
	if (latest.getLexeme() != "$$"){
		std::cerr << "Syntax Error: Expected " << "$$" << " on line " << line << "\n";
		exit(1);
	}

}
#endif
