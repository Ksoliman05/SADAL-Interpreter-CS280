#include "val.h"
#include <iostream>
#include <cmath>


// Arithmetic Operators


Value Value::operator+(const Value& op) const {
    if (IsInt() && op.IsInt())
        return Value(GetInt() + op.GetInt());
    if (IsReal() && op.IsReal())
        return Value(GetReal() + op.GetReal());

    cerr << "Run-Time Error: Illegal operands for +" << endl;
    return Value();  
}

Value Value::operator-(const Value& op) const {
    if (IsInt() && op.IsInt()){
        return Value(GetInt() - op.GetInt());
    }
    if (IsReal() && op.IsReal()){
        return Value(GetReal() - op.GetReal());
    }
    
    cerr << "Run-Time Error: Illegal operands for -" << endl;
    return Value();
}

Value Value::operator*(const Value& op) const {
    if (IsInt() && op.IsInt()){
        return Value(GetInt() * op.GetInt());
    }
    if (IsReal() && op.IsReal()){
        return Value(GetReal() * op.GetReal());
    }
    
    cerr << "Run-Time Error: Illegal operands for *" << endl;
    return Value();
}

Value Value::operator/(const Value& op) const {
    if (IsInt() && op.IsInt()) {
        if (op.GetInt() == 0) {
            cerr << "Run-Time Error: Division by zero" << endl;
            return Value();
        }
        return Value(GetInt() / op.GetInt());
    }
    if (IsReal() && op.IsReal()) {
        if (op.GetReal() == 0.0) {
            cerr << "Run-Time Error: Division by zero" << endl;
            return Value();
        }
        return Value(GetReal() / op.GetReal());
    }
    cerr << "Run-Time Error: Illegal operands for /" << endl;
    return Value();
}

Value Value::operator%(const Value& op) const {
    if (IsInt() && op.IsInt()) {
        if (op.GetInt() == 0) {
            cerr << "Run-Time Error: Division by zero" << endl;
            return Value();
        }
        return Value(GetInt() % op.GetInt());
    }
    cerr << "Run-Time Error: Illegal operands for %" << endl;
    return Value();
}


// Relational Operators

Value Value::operator==(const Value& op) const {
    if (IsInt() && op.IsInt())
        return Value(GetInt() == op.GetInt());
    if (IsReal() && op.IsReal())
        return Value(GetReal() == op.GetReal());
    if (IsBool() && op.IsBool())
        return Value(GetBool() == op.GetBool());
    if (IsChar() && op.IsChar())
        return Value(GetChar() == op.GetChar());
    if (IsString() && op.IsString())
        return Value(GetString() == op.GetString());
    
    cerr << "Run-Time Error: Illegal operands for ==" << endl;
    return Value();
}

Value Value::operator!=(const Value& op) const {
    if (IsInt() && op.IsInt())
        return Value(GetInt() != op.GetInt());
    if (IsReal() && op.IsReal())
        return Value(GetReal() != op.GetReal());
    if (IsBool() && op.IsBool())
        return Value(GetBool() != op.GetBool());
    if (IsChar() && op.IsChar())
        return Value(GetChar() != op.GetChar());
    if (IsString() && op.IsString())
        return Value(GetString() != op.GetString());
    
    cerr << "Run-Time Error: Illegal operands for !=" << endl;
    return Value();
}

Value Value::operator>(const Value& op) const {
    if (IsInt() && op.IsInt())
        return Value(GetInt() > op.GetInt());
    if (IsReal() && op.IsReal())
        return Value(GetReal() > op.GetReal());
    if (IsChar() && op.IsChar())
        return Value(GetChar() > op.GetChar());
    if (IsString() && op.IsString())
        return Value(GetString() > op.GetString());
    
    cerr << "Run-Time Error: Illegal operands for >" << endl;
    return Value();
}

Value Value::operator<(const Value& op) const {
    if (IsInt() && op.IsInt())
        return Value(GetInt() < op.GetInt());
    if (IsReal() && op.IsReal())
        return Value(GetReal() < op.GetReal());
    if (IsChar() && op.IsChar())
        return Value(GetChar() < op.GetChar());
    if (IsString() && op.IsString())
        return Value(GetString() < op.GetString());
    
    cerr << "Run-Time Error: Illegal operands for <" << endl;
    return Value();
}

Value Value::operator<=(const Value& op) const {
    if (IsInt() && op.IsInt())
        return Value(GetInt() <= op.GetInt());
    if (IsReal() && op.IsReal())
        return Value(GetReal() <= op.GetReal());
    if (IsChar() && op.IsChar())
        return Value(GetChar() <= op.GetChar());
    if (IsString() && op.IsString())
        return Value(GetString() <= op.GetString());
    
    cerr << "Run-Time Error: Illegal operands for >" << endl;
    return Value();}

Value Value::operator>=(const Value& op) const {
    if (IsInt() && op.IsInt())
        return Value(GetInt() >= op.GetInt());
    if (IsReal() && op.IsReal())
        return Value(GetReal() >= op.GetReal());
    if (IsChar() && op.IsChar())
        return Value(GetChar() >= op.GetChar());
    if (IsString() && op.IsString())
        return Value(GetString() >= op.GetString());
    
    cerr << "Run-Time Error: Illegal operands for >" << endl;
    return Value();
}


// Logical Operators

Value Value::operator&&(const Value& op) const {
    if (IsBool() && op.IsBool())
        return Value(GetBool() && op.GetBool());
    
    cerr << "Run-Time Error: Illegal operands for &&" << endl;
    return Value();
}

Value Value::operator||(const Value& op) const {
    if (IsBool() && op.IsBool())
        return Value(GetBool() || op.GetBool());
    
    cerr << "Run-Time Error: Illegal operands for ||" << endl;
    return Value();
}

Value Value::operator!(void) const {
    if (IsBool())
        return Value(!GetBool());
    
    cerr << "Run-Time Error: Illegal operands for !" << endl;
    return Value();
}

// Special SADAL Operators

Value Value::Concat(const Value& op) const {
    if(IsString() && op.IsString()){
        Value result(GetString() + op.GetString());
        result.SetstrLen(result.GetString().length());
        return result;    
    }
    if(IsString() && op.IsChar()){
        Value result(GetString() + string(1, op.GetChar()));
        result.SetstrLen(result.GetString().length());
        return result;
    }
    if(IsChar() && op.IsString()){
        Value result(string(1, GetChar()) + op.GetString());
        result.SetstrLen(result.GetString().length());
        return result;
    }
    if(IsChar() && op.IsChar()){
        Value result(string(1, GetChar()) + string(1, op.GetChar()));
        result.SetstrLen(result.GetString().length());
        return result;
    }

    cerr << "Run-Time Error: Illegal operands for Concat" << endl;
    return Value();
}

Value Value::Exp(const Value& op) const {
    if(IsReal() && op.IsReal()){
        if(op.GetReal() == 0.0) {
            return Value(1.0);
        }
        else if(GetReal() == 0.0 && op.GetReal() > 0) {
            return Value(0.0);
        }
        else if(GetReal() == 0.0 && op.GetReal() < 0) {
            cerr << "Run-Time Error: Zero raised to negative power" << endl;
            return Value();
        }
        else if(op.GetReal() < 0) {
            double positive_power = pow(GetReal(), -op.GetReal());
            return Value(1.0 / positive_power);
        }
        return Value(pow(GetReal(), op.GetReal()));
    }
    cerr << "Run-Time Error: Illegal operands for Exp" << endl;
    return Value();
}