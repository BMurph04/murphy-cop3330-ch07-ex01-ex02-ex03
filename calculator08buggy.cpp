/*
 *  UCF COP3330 Fall 2021 Assignment 6 Solution
 *  Copyright 2021 Brendon Murphy
 */

/*

The assignment operator could be used for ... However, it could be a problem because... 

*/

#include "std_lib_facilities.h"

struct Token {
	char kind; //Type of input
	double value; //Value of input if number
	string name; //Name of input
	Token(char ch) :kind(ch), value(0) { } //Initialize kind
	Token(char ch, double val) :kind(ch), value(val) { } //Initialize kind and value

	Token(char ch, string n) :kind(ch), name(n){ } //Initialize kind and name
};

class Token_stream {
	bool full; //If there is a token in the buffer or not
	Token buffer; //Keep a token put back using putback()
public:
	Token_stream() :full(0), buffer(0) { }

	Token get();
	void unget(Token t) { buffer = t; full = true; }

	void ignore(char);
};

const char let = 'L';
const char cons = 'C';
const char quit = 'Q';
const char print = ';';
const char number = '8';
const char name = 'a';

Token Token_stream::get()
{
	if (full) { 
		full = false; 
		return buffer; 
		}
	char ch;
	cin >> ch;
	switch (ch) {
	case quit:
	case print:
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case '=':
		{
		return Token(ch); //Each character represents itself
		}
	case '.': //Floating-point literal may start with dot
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9': //Numeric literal
	{	cin.unget(); //Put digit back into input stream
	double val;
	cin >> val; //Read floating-point number
	return Token(number, val);
	}
	default:
		if (isalpha(ch)) { //Reading a name
			string s;
			s += ch; //Apends character ch to end of string s
			while (cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) s += ch; //Adds ch to s and read again if ch is a letter or digit
			cin.unget();
			if (s == "let") return Token(let); //declaration keyword
			if(s == "cons") return Token(cons); //constant declaration keyword
			if (s == "quit") return Token(name); //quit
			return Token(name, s);
		}
		error("Bad token");
	}
}

void Token_stream::ignore(char c) //c is kind of token
{
	//check buffer
	if (full && c == buffer.kind) {
		full = false;
		return;
	}
	full = false;

	//search input
	char ch;
	while (cin >> ch)
		if (ch == c) return;
}

struct Variable {
	string name;
	double value;
	bool cons;
	Variable(string n, double v, bool c) :name(n), value(v), cons(c) { }
};

vector<Variable> names;

double get_value(string s)
{
	for (const Variable &n : names)
		if (n.name == s) return n.value; //Gets value of variable in vector
	error("get: undefined name ", s);
}

void set_value(string s, double d)
{
	for (Variable &n : names)
		if (n.name == s) {
			if(n.cons == true) error("set: constant variable ", s);
			else{ n.value = d; //Sets value of variable in vector
			return;
			}
		}
	error("set: undefined name ", s);
}

bool is_declared(string s)
{
	for (Variable &n : names)
		if (n.name == s) return true; //Returns true if variable is already in the names vector
	return false;
}



Token_stream ts;

double expression();

double assignment(string name)
//Assuming that we have seen name =,
{
	double d = expression(); 
	set_value(name, d); //Sets value of variable to number
	return d;
}

double primary()
{
	Token t = ts.get();
	string varname;
	switch (t.kind) {
	case '(': //Handle '(' expression ')'
	{	double d = expression();
		t = ts.get();
		if (t.kind != ')') error("'(' expected");
		return d;
	}
	case '-':
		return -primary();
	case '+':
		return primary();
	case number:
		return t.value;
	case name:
		varname = t.name;
		t = ts.get(); //read next token
		if(t.kind == '=' && is_declared(varname)) //if next token is '=' and name is declared, 
			return assignment(varname); //run assignment(name)
		//else, unget(t) and return get_value(t.name)
		else{
			ts.unget(t);
			return get_value(varname);
		}
	default:
		error("primary expected");
	}
}

double term()
{
	double left = primary();
	while (true) {
		Token t = ts.get(); //Get next token from token stream 
		switch (t.kind) {
		case '*':
			left *= primary();
			break;
		case '/':
		{	double d = primary();
		if (d == 0) error("divide by zero");
		left /= d;
		break;
		}
		default:
			ts.unget(t); //Put back into token stream
			return left;
		}
	}
}

double expression()
{
	double left = term();
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '+':
			left += term();
			break;
		case '-':
			left -= term();
			break;
		default:
			ts.unget(t); //Put t back into token stream
			return left; //Return answer after no more + or -
		}
	}
}

double declaration() //Adds var, val to names vector
//Assuming that we have seen let, 
{
	Token t = ts.get();
	if (t.kind != name) error("name expected in declaration"); //Throw error if first input is not a name
	string name = t.name;
	if (is_declared(name)) error(name, " declared twice"); //Throw error if input is declared twice
	Token t2 = ts.get();
	if (t2.kind != '=') error("= missing in declaration of ", name); //Throw error if there is no = in second input
	double d = expression();
	names.push_back(Variable(name, d, false));
	return d;
}

double constant()
//Assuming that we have seen cons,
{
	Token t = ts.get();
	if (t.kind != name) error("name expected in declaration"); //Throw error if first input is not a name
	string name = t.name;
	if (is_declared(name)) error(name, " declared twice"); //Throw error if input is declared twice
	Token t2 = ts.get();
	if (t2.kind != '=') error("= missing in declaration of ", name); //Throw error if there is no = in second input
	const double d = expression();
	names.push_back(Variable(name, d, true));
	return d;

}

double statement()
{
	Token t = ts.get();
	switch (t.kind) {
	case let: //Declaration following let
		return declaration();
	case cons:
		return constant();
	default: //If no let, run expression function 
		ts.unget(t);
		return expression();
	}
}

void clean_up_mess()
{
	ts.ignore(print);
}

const string prompt = "> ";
const string result = "= ";

void calculate()
{
	while (cin) try {
		cout << prompt;
		Token t = ts.get();
		while (t.kind == print) t = ts.get(); //Discard all prints first
		if (t.kind == quit){
			return; //Quit
		}
		ts.unget(t);
		cout << result << statement() << endl;
	}
	catch (runtime_error& e) {
		cerr << e.what() << endl; //Error message
		clean_up_mess();
	}
}

int main()

try {
	calculate();
	return 0;
}
catch (exception& e) {
	cerr << "exception: " << e.what() << endl;
	char c;
	while (cin >> c && c != ';');
	return 1;
}
catch (...) {
	cerr << "exception\n";
	char c;
	while (cin >> c && c != ';');
	return 2;
}
