#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <random>
#include <stack>
#include "calculate_tree.h"

using namespace std;


int main() {
    setlocale(LC_ALL, "Russian");
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, 10);

    

    string choice="";
    while (choice != "4") {
        system("cls");
        cout << "Калькулятор выражений (на основе деревий)" << endl;
        cout << "1. Ввод с клавиатуры" << endl;
        cout << "2. Ввод из файла" << endl;
        cout << "3. Случайная генерация" << endl;
        cout << "4. Выход" << endl;
        cout << "Выберите опцию: ";
        bool nextWrong = 1;
        cin >> choice;
        string formula;
        cin.ignore();

        if (choice == "1") {
            cout << "Введите формулу: ";
            getline(cin, formula);
        }
        else if (choice == "2") {
            string filename;
            cout << "Введите имя файла: ";
            getline(cin, filename);
            ifstream file(filename);
            if (!file) {
                cerr << "Ошибка открытия файла: " << filename << endl;
                return 1;
            }
            getline(file, formula);
        }
        else if (choice == "3") {
            formula = generateRandomFormula(gen, dist, 3 + dist(gen) % 2);
            cout << "Сгенерированная формула: " << formula << endl;
        }
        else if (choice == "4") {
            cout << "Досвидания" << endl;
            return 1;
        }
        else {
            cerr << "Неверный выбор" << endl;
            nextWrong = 0;
        }
        if (nextWrong) {
            try {
                Parser parser(formula);
                Node* root = parser.parse();
                cout << "Полученное выражение: " << print(root) << endl;

                Node* simplified = simplify(deepCopy(root));
                cout << "Упрощенное выражение: " << print(simplified) << endl;

                vector<char> variables;
                collectVariables(simplified, variables);
                vector<pair<char, int>> varMap;

                if (!variables.empty()) {
                    cout << "Введите значения переменных:" << endl;
                    for (char var : variables) {
                        cout << var << " = ";
                        int val;
                        cin >> val;
                        varMap.push_back(make_pair(var, val));
                    }
                }

                int result = eval(simplified, varMap);
                cout << "Результат вычисления: " << result << endl;

                delete root;
                delete simplified;

            }
            catch (string& e) {
                cerr << "Ошибка: " << e << endl;
            }
        }
        cout << "Хотите воспользоваться ещё раз?" << endl;
        string ChoiceEnd;
        cout << "1 - Да/ 0 - Нет" << endl;
        cin >> ChoiceEnd;
        if (ChoiceEnd == "0") {
            choice = "4";
        }
        else if (ChoiceEnd == "1") {
            cout << "Хорошо" << endl;
        }
        else {
            cout << "Будем считать что да" << endl;
        }

    }
    cout << "Досвидания";
    return 0;
}