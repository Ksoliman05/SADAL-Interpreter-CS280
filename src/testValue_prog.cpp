#include <iostream>
#include <string>
#include <vector> 
using namespace std; 
#include "val.h"
#include "val.cpp"

// === Arithmetic ===
void ExecuteAdd(const Value& val1, const Value& val2) {
	cout << val1 << " + " << val2 << " is " << (val1 + val2) << endl;
}
void ExecuteSub(const Value& val1, const Value& val2) {
	cout << val1 << " - " << val2 << " is " << (val1 - val2) << endl;
}
void ExecuteDiv(const Value& val1, const Value& val2) {
	cout << val1 << " / " << val2 << " is " << (val1 / val2) << endl;
}
void ExecuteMod(const Value& val1, const Value& val2) {
	cout << val1 << " % " << val2 << " is " << (val1 % val2) << endl;
}

void ExecuteMult(const Value& val1, const Value& val2) {
	cout << val1 << " * " << val2 << " is " << (val1 * val2) << endl;
}

	// === Relational ===
void ExecuteEq(const Value& val1, const Value& val2) {
	cout << val1 << " == " << val2 << " is " << (val1 == val2) << endl;
}
void ExecuteNeq(const Value& val1, const Value& val2) {
	cout << val1 << " != " << val2 << " is " << (val1 != val2) << endl;
}
void ExecuteGThan(const Value& val1, const Value& val2) {
	cout << val1 << " > " << val2 << " is " << (val1 > val2) << endl;
}
void ExecuteLThan(const Value& val1, const Value& val2) {
	cout << val1 << " < " << val2 << " is " << (val1 < val2) << endl;
}
void ExecuteGThanE(const Value& val1, const Value& val2) {
	cout << val1 << " >= " << val2 << " is " << (val1 >= val2) << endl;
}
void ExecuteLThanE(const Value& val1, const Value& val2) {
	cout << val1 << " <= " << val2 << " is " << (val1 <= val2) << endl;
}

// === Logical ===
void ExecuteAnd(const Value& val1, const Value& val2) {
	cout << val1 << " && " << val2 << " is " << (val1 && val2) << endl;
}
void ExecuteOr(const Value& val1, const Value& val2) {
	cout << val1 << " || " << val2 << " is " << (val1 || val2) << endl;
}
void ExecuteNot(const Value& val1) {
	cout << "!" << val1 << " is " << (!val1) << endl;
}

// === Special ===
void ExecuteExp(const Value& val1, const Value& val2) {
	cout << val1 << " ** " << val2 << " is " << (val1.Exp(val2)) << endl;
}
void ExecuteConcat(const Value& val1, const Value& val2) {
	cout << val1 << " & " << val2 << " is " << (val1.Concat(val2)) << endl;
}

// === Main ===
int main(int argc, char *argv[]) {
	Value ErrorVal;
	double num1 = 9.25;
	Value doubleVal1(num1);
	double num2 = 4.0;
	Value doubleVal2(num2);
	string str1 = "CS280";
	Value StrVal1(str1); 
	string str2 = "Fall 2024";
	Value StrVal2(str2);
	int intval1 = 18;
	Value intVal1(intval1);
	int intval2 = 6;
	Value intVal2(intval2);
	char ch1 = 'S';
	Value chVal1(ch1);
	char ch2 = 'z';
	Value chVal2(ch2);
	bool b1 = true;
	Value bVal1(b1);
	bool b2 = false;
	Value bVal2(b2);

	vector<Value> vals({
		ErrorVal, intVal1, intVal2, doubleVal1, doubleVal2,
		StrVal1, StrVal2, chVal1, chVal2, bVal1, bVal2
	});

	if (argc < 2) {
		cerr << "No Specified arguments." << endl;
		return 0;
	}

	string arg(argv[1]);
	for (int i = 0; i < vals.size(); ++i) {
		for (int j = 0; j < vals.size(); ++j) {
			if (i == j) continue;
			if (arg == "-add") ExecuteAdd(vals[i], vals[j]);
			else if (arg == "-sub") ExecuteSub(vals[i], vals[j]);
			else if (arg == "-mul") ExecuteMult(vals[i], vals[j]);
			else if (arg == "-div") ExecuteDiv(vals[i], vals[j]);
			else if (arg == "-mod") ExecuteMod(vals[i], vals[j]);
			else if (arg == "-exp") ExecuteExp(vals[i], vals[j]);
			else if (arg == "-cat") ExecuteConcat(vals[i], vals[j]);
			else if (arg == "-eq") ExecuteEq(vals[i], vals[j]);
			else if (arg == "-neq") ExecuteNeq(vals[i], vals[j]);
			else if (arg == "-gt") ExecuteGThan(vals[i], vals[j]);
			else if (arg == "-lt") ExecuteLThan(vals[i], vals[j]);
			else if (arg == "-gte") ExecuteGThanE(vals[i], vals[j]);
			else if (arg == "-lte") ExecuteLThanE(vals[i], vals[j]);
			else if (arg == "-and") ExecuteAnd(vals[i], vals[j]);
			else if (arg == "-or") ExecuteOr(vals[i], vals[j]);
		}
		// Unary logical test
		if (arg == "-not") {
			ExecuteNot(vals[i]);
		}
	}

	return 0;
}