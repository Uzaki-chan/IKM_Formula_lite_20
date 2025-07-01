#define CALCULATE_TREE_H
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <random>
#include <stack>

using namespace std;

bool is_digit(char c) { return c >= '0' && c <= '9'; }
bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
bool is_space(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

class Node {
public:
    string value;
    Node* left;
    Node* right;

    Node(string val) : value(val), left(nullptr), right(nullptr) {}
    Node(string val, Node* l, Node* r) : value(val), left(l), right(r) {}

};

class Parser {
private:
    string input;
    int pos;
    char current_char;

    void advance() {
        pos++;
        if (pos < input.size()) {
            current_char = input[pos];
        }
        else {
            current_char = '\0';
        }
    }

    void skipWhitespace() {
        while (current_char != '\0' && is_space(current_char)) {
            advance();
        }
    }

public:
    Parser(const string& s) : input(s), pos(-1) {
        advance();
        skipWhitespace();
    }

    Node* parse() {
        if (current_char == '\0') {
            throw string("Пустое выражение");
        }
        Node* node = parseExpr();
        if (current_char != '\0') {
            throw string("Непредвиденные символы в конце выражения");
        }
        return node;
    }

private:
    Node* parseExpr() {
        Node* node = parseTerm();
        while (current_char == '+' || current_char == '-') {
            char op = current_char;
            advance();
            skipWhitespace();
            Node* right = parseTerm();
            node = new Node(string(1, op), node, right);
        }
        return node;
    }

    Node* parseTerm() {
        Node* node = parseUnary();
        while (current_char == '*' || current_char == '/') {
            char op = current_char;
            advance();
            skipWhitespace();
            Node* right = parseUnary();
            node = new Node(string(1, op), node, right);
        }
        return node;
    }

    Node* parseUnary() {
        if (current_char == '-') {
            advance();
            skipWhitespace();
            Node* child = parseUnary();
            return new Node("-", child, nullptr);
        }
        return parsePrimary();
    }

    Node* parsePrimary() {
        if (is_digit(current_char)) {
            string num;
            while (is_digit(current_char)) {
                num += current_char;
                advance();
            }
            skipWhitespace();
            return new Node(num);
        }
        else if (is_alpha(current_char)) {
            string var(1, current_char);
            advance();
            skipWhitespace();
            return new Node(var);
        }
        else if (current_char == '(') {
            advance();
            skipWhitespace();
            Node* node = parseExpr();
            if (current_char != ')') {
                throw string("Ожидается ')'");
            }
            advance();
            skipWhitespace();
            return node;
        }
        else {
            throw string("Непредвиденный символ: ") + current_char;
        }
    }
};

int eval(Node* node, const vector<pair<char, int>>& varMap) {
    if (!node->left && !node->right) {
        if (is_digit(node->value[0])) {
            return stoi(node->value);
        }
        else {
            char varName = node->value[0];
            for (const auto& p : varMap) {
                if (p.first == varName) {
                    return p.second;
                }
            }
            throw string("Неопределенная переменная: ") + varName;
        }
    }

    if (node->value == "-" && !node->right) {
        int val = eval(node->left, varMap);
        return -val;
    }

    if (node->left && node->right) {
        int leftVal = eval(node->left, varMap);
        int rightVal = eval(node->right, varMap);
        if (node->value == "+") return leftVal + rightVal;
        if (node->value == "-") return leftVal - rightVal;
        if (node->value == "*") return leftVal * rightVal;
        if (node->value == "/") {
            if (rightVal == 0) {
                throw string("Деление на ноль");
            }
            return leftVal / rightVal;
        }
    }

    throw string("Некорректная структура узла");
}

string print(Node* node) {
    if (!node->left && !node->right) {
        return node->value;
    }
    if (node->value == "-" && !node->right) {
        string child = print(node->left);
        if (node->left->left || node->left->right) {
            return "-(" + child + ")";
        }
        return "-" + child;
    }
    string left = print(node->left);
    string right = print(node->right);
    return "(" + left + " " + node->value + " " + right + ")";
}

void collectVariables(Node* node, vector<char>& vars) {
    if (!node) return;
    if (!node->left && !node->right) {
        if (node->value.size() == 1 && is_alpha(node->value[0])) {
            char var = node->value[0];
            for (char v : vars) {
                if (v == var) return;
            }
            vars.push_back(var);
        }
        return;
    }
    collectVariables(node->left, vars);
    collectVariables(node->right, vars);
}

Node* deepCopy(Node* node) {
    if (!node) return nullptr;
    Node* copy = new Node(node->value);
    copy->left = deepCopy(node->left);
    copy->right = deepCopy(node->right);
    return copy;
}

bool isSame(Node* a, Node* b) {
    if (!a && !b) return true;
    if (!a || !b) return false;
    if (a->value != b->value) return false;
    return isSame(a->left, b->left) && isSame(a->right, b->right);
}

// Вспомогательная функция для рекурсивного удаления поддерева
void deleteSubtree(Node* node) {
    if (node == nullptr) return;
    deleteSubtree(node->left);
    deleteSubtree(node->right);
    delete node;
}
Node* simplify(Node* node) {
    if (!node) return nullptr;

    // Рекурсивное упрощение дочерних узлов
    node->left = simplify(node->left);
    node->right = simplify(node->right);

    // Правило: -(-a) → a
    if (node->value == "-" && node->left && !node->right) {
        Node* child = node->left;
        if (child->value == "-" && child->left && !child->right) {
            Node* result = child->left;
            // Отсоединяем и удаляем промежуточные узлы
            child->left = nullptr;
            node->left = nullptr;
            delete child;
            delete node;
            return simplify(result);
        }
    }

    // Правило: +(-a) → -a
    if (node->value == "+" && node->left && !node->right) {
        Node* child = node->left;
        if (child->value == "-" && child->left && !child->right) {
            node->left = nullptr;
            delete node;
            return simplify(child);
        }
    }

    // Правила для константных выражений
    if (node->value == "*" && node->left && node->right) {
        // 0 * a → 0
        if (node->left->value == "0" || node->right->value == "0") {
            Node* zero = new Node("0");
            delete node;
            return zero;
        }
        // 1 * a → a
        if (node->left->value == "1") {
            Node* result = node->right;
            node->right = nullptr;
            delete node;
            return result;
        }
        if (node->right->value == "1") {
            Node* result = node->left;
            node->left = nullptr;
            delete node;
            return result;
        }
    }

    if (node->value == "+" && node->left && node->right) {
        // a + 0 → a
        if (node->left->value == "0") {
            Node* result = node->right;
            node->right = nullptr;
            delete node;
            return result;
        }
        // 0 + a → a
        if (node->right->value == "0") {
            Node* result = node->left;
            node->left = nullptr;
            delete node;
            return result;
        }
    }

    if (node->value == "-" && node->left && node->right) {
        // a - 0 → a
        if (node->right->value == "0") {
            Node* result = node->left;
            node->left = nullptr;
            delete node;
            return result;
        }
    }

    // Правило: (a * b) + (a * c) → a * (b + c)
    if (node->value == "+" && node->left && node->right &&
        node->left->value == "*" && node->right->value == "*") {

        Node* a_left = nullptr;
        Node* b = nullptr;
        Node* a_right = nullptr;
        Node* c = nullptr;
        bool found = false;

        // Проверяем комбинации множителей
        for (int i = 0; i < 2 && !found; i++) {
            a_left = (i == 0) ? node->left->left : node->left->right;
            b = (i == 0) ? node->left->right : node->left->left;

            for (int j = 0; j < 2 && !found; j++) {
                a_right = (j == 0) ? node->right->left : node->right->right;
                c = (j == 0) ? node->right->right : node->right->left;

                if (isSame(a_left, a_right)) {
                    found = true;
                }
            }
        }

        if (found) {
            // Отсоединяем использованные узлы
            if (a_left == node->left->left) {
                node->left->left = nullptr;
            }
            else {
                node->left->right = nullptr;
            }
            if (a_right == node->right->left) {
                node->right->left = nullptr;
            }
            else {
                node->right->right = nullptr;
            }

            // Создаем новое поддерево
            Node* sum_node = new Node("+", b, c);
            Node* new_node = new Node("*", a_left, sum_node);

            // Удаляем старые узлы
            delete node->left;
            delete node->right;
            delete node;

            return simplify(new_node);
        }
    }

    // Правило: (a * b) - (a * c) → a * (b - c)
    if (node->value == "-" && node->left && node->right &&
        node->left->value == "*" && node->right->value == "*") {

        Node* a_left = nullptr;
        Node* b = nullptr;
        Node* a_right = nullptr;
        Node* c = nullptr;
        bool found = false;

        for (int i = 0; i < 2 && !found; i++) {
            a_left = (i == 0) ? node->left->left : node->left->right;
            b = (i == 0) ? node->left->right : node->left->left;

            for (int j = 0; j < 2 && !found; j++) {
                a_right = (j == 0) ? node->right->left : node->right->right;
                c = (j == 0) ? node->right->right : node->right->left;

                if (isSame(a_left, a_right)) {
                    found = true;
                }
            }
        }

        if (found) {
            // Отсоединяем использованные узлы
            if (a_left == node->left->left) {
                node->left->left = nullptr;
            }
            else {
                node->left->right = nullptr;
            }
            if (a_right == node->right->left) {
                node->right->left = nullptr;
            }
            else {
                node->right->right = nullptr;
            }

            Node* diff_node = new Node("-", b, c);
            Node* new_node = new Node("*", a_left, diff_node);

            delete node->left;
            delete node->right;
            delete node;

            return simplify(new_node);
        }
    }

    return node;
}

string generateRandomFormula(mt19937& gen, uniform_int_distribution<>& dist, int depth) {
    if (depth == 0 || dist(gen) < 3) {
        if (dist(gen) % 2) {
            return to_string(dist(gen) % 9 + 1);
        }
        else {
            char c = 'a' + dist(gen) % 26;
            return string(1, c);
        }
    }

    int choice = dist(gen) % 5;
    if (choice == 0) {
        return "-" + generateRandomFormula(gen, dist, depth - 1);
    }
    else {
        string op;
        switch (dist(gen) % 4) {
        case 0: op = "+"; break;
        case 1: op = "-"; break;
        case 2: op = "*"; break;
        case 3: op = "/"; break;
        }
        string left = generateRandomFormula(gen, dist, depth - 1);
        string right = generateRandomFormula(gen, dist, depth - 1);
        return "(" + left + " " + op + " " + right + ")";
    }
}

