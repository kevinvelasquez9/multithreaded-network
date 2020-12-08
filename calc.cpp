#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <iostream>
using std::vector;
using std::string;
using std::stringstream;
using std::unordered_map;
using std::cout;
using std::endl;

struct Calc {
};

class CalcImpl : public Calc {
    public:
        unordered_map<string, int> variables;
        
        int evalExpr(const char *expr, int *result) {
            vector<string> tokens = tokenize(expr);
            bool err = false;
            switch(tokens.size()) {
                case 1:
                    err = evalOne(tokens[0], result);
                    break;
                case 3:
                    err = evalThree(tokens, result);
                    break;
                case 5:
                    err = evalFive(tokens, result);
                    break;
            }
            if (!err) {
                cout << "Error" << endl;
                return 0;
            }
            return 1;
        }

        // Check if token is a valid number, and update the pointer
        bool is_num(string token, int *p) {
            // A variable
            if (is_alpha(token)) {
                if (variables.find(token) != variables.end()) {
                    *p = variables[token];
                    return true;
                }
            }
            // A number
            else {
                char *endptr;
                int num = (int) strtol(token.c_str(), &endptr, 10);
                if (*endptr == '\0') {
                    *p = num;
                    return true;
                }
            }
            return false;
        }

        // Compute s1 (op) s2
        bool compute(string s1, string op, string s2, int *result) {
            int num1 = 0;
            int num2 = 0;

            if (is_num(s1, &num1) && is_num(s2, &num2) && is_op(op)) {
                // Now we can compute
                if (op == "+") {
                    *result = num1 + num2;
                } else if (op == "-") {
                    *result = num1 - num2;
                } else if (op == "*") {
                    *result = num1 * num2;
                } else if (op == "/") {
                    if (num2 == 0) {
                        return false;
                    } 
                    *result = num1 / num2;
                }
                return true;
            }
            return false;
        }

        // Evaluate for one token
        bool evalOne(string token, int *result) {
            return is_num(token, result);
        }

        // Evaluate for three tokens
        bool evalThree(vector<string> tokens, int *result) {
            // If it is an assignment
            if (tokens[1] == "=") {
                // Make sure first token is alphabetical
                if (is_alpha(tokens[0])) {
                    int num;
                    if (is_num(tokens[2], &num)) {
                        variables[tokens[0]] = num;
                        *result = num;
                        return true;
                    }
                }
            }
            // Else, assume it is an operation
            return compute(tokens[0], tokens[1], tokens[2], result);
        }

        // Evaluate for five tokens
        bool evalFive(vector<string> tokens, int *result) {
            int ans;
            if (tokens[1] == "=" && compute(tokens[2], tokens[3], tokens[4], &ans)) {
                variables[tokens[0]] = ans;
                *result = ans;
                return true;
            }
            return false;
        }

        // Splits a string into a vector of strings
        vector<string> tokenize(const string &expr) {
            vector<string> vec;
            stringstream s(expr);
            string tok;
            while (s >> tok) {
                vec.push_back(tok);
            }
            return vec;
        }

        // Checks if string is alphabetical
        bool is_alpha(string s) {
            for (char const &c : s) {
                if (!isalpha(c)) {
                    return false;
                }
            }
            return true;
        }

        // Checks if string is an operator
        bool is_op(string op) {
            if (op == "+" || op == "-" || op == "/" || op == "*") {
                return true;
            }
            return false;
        }
};

extern "C" struct Calc *calc_create(void) {
    return new CalcImpl();
}

extern "C" void calc_destroy(struct Calc *calc) {
    CalcImpl *obj = static_cast<CalcImpl *>(calc);
    delete obj;
}

extern "C" int calc_eval(struct Calc *calc, const char *expr, int *result) {
    CalcImpl *obj = static_cast<CalcImpl *>(calc);
    return obj->evalExpr(expr, result);
}