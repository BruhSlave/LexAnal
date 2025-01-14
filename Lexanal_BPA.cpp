#include "framework.h"
#include "Lab4_Form.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

#define MAX_LOADSTRING 100
#define IDC_INPUT_EDIT 101
#define IDC_OUTPUT_EDIT 102
#define IDC_PROCESS_BUTTON 103

// Глобальные переменные
HINSTANCE hInst; // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING]; // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING]; // имя класса главного окна

// Прототипы функций
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

// Структура для токенов
struct Token {
    string type;
    string value;
};

// Регулярные выражения для токенов
const vector<pair<string, string>> TOKENS = {
    {"Number", R"(\b\d+\.\d+|\b\d+\b)"},               // Числа (целые и с плавающей точкой)
    {"MultilineComment", R"(/\*.*?\*/)"},              // Многострочные комментарии
    {"SingleLineComment", R"(//.*)"},                 // Однострочные комментарии
    {"Variable", R"([a-zA-Z_][a-zA-Z0-9_]*)"},        // Имена переменных
    {"Plus", R"(\+)"},                                // Плюс
    {"Minus", R"(-)"},                                // Минус
    {"Mult", R"(\*)"},                                // Умножение
    {"Division", R"(/)"},                             // Деление
    {"Modulo", R"(%)"},                               // Модуль (остаток от деления)
    {"Increment", R"(\+\+)"},                         // Инкремент
    {"Decrement", R"(--)"},                           // Декремент
    {"Equal", R"(==)"},                               // Равно
    {"NotEqual", R"(!=)"},                            // Не равно
    {"GreaterThan", R"(>)"},                          // Больше
    {"LessThan", R"(<)"},                             // Меньше
    {"GreaterEqual", R"(>=)"},                        // Больше или равно
    {"LessEqual", R"(<=)"},                           // Меньше или равно
    {"LogicalAnd", R"(&&)"},                          // Логическое И
    {"LogicalOr", R"(\|\|)"},                         // Логическое ИЛИ
    {"Not", R"(!)"},                                  // Логическое НЕ
    {"BitwiseAnd", R"(&)"},                           // Побитовое И
    {"BitwiseOr", R"(\|)"},                           // Побитовое ИЛИ
    {"Semicolon", R"(;)"},                            // Точка с запятой
    {"BitwiseXor", R"(\^)"},                          // Побитовое исключающее ИЛИ
    {"BitwiseNot", R"(~)"},                           // Побитовое НЕ
    {"LeftShift", R"(<<)"},                           // Сдвиг влево
    {"RightShift", R"(>>)"},                          // Сдвиг вправо
    {"Assign", R"(:=)"},                              // Присваивание
    {"AddAssign", R"(\+=)"},                          // Присваивание с добавлением
    {"SubAssign", R"(-=)"},                           // Присваивание с вычитанием
    {"MultAssign", R"(\*=)"},                         // Присваивание с умножением
    {"DivAssign", R"(/=)"},                           // Присваивание с делением
    {"ModuloAssign", R"(%=)"},                        // Присваивание с остатком
    {"OpenBracket", R"(\()"},                         // Открывающая круглая скобка
    {"CloseBracket", R"(\))"},                        // Закрывающая круглая скобка
    {"OpenBrace", R"(\{)"},                           // Открывающая фигурная скобка
    {"CloseBrace", R"(\})"},                          // Закрывающая фигурная скобка
    {"OpenSquareBracket", R"(\[)"},                   // Открывающая квадратная скобка
    {"CloseSquareBracket", R"(\])"},                  // Закрывающая квадратная скобка
    {"Comma", R"(,)"},                                // Запятая
    {"Dot", R"(\.)"},                                 // Точка
    {"Colon", R"(:)"},                                // Двоеточие
    {"Arrow", R"(->)"},                               // Стрелка
    {"DoubleColon", R"(::)"},                         // Двойное двоеточие
    {"QuestionMark", R"(\?)"},                        // Вопросительный знак
    {"StringLiteral", R"("[^"]*")"},                  // Строковые литералы
    {"CharLiteral", R"('([^']|\\')*')"},              // Символьные литералы
    {"Space", R"(\s+)"},                              // Пробелы
};

// Лексер для разбора текста
vector<Token> lexer(const string& input) {
    vector<Token> tokens;
    string regex_pattern;

    for (const auto& token : TOKENS) {
        if (!regex_pattern.empty()) {
            regex_pattern += "|";
        }
        regex_pattern += "(" + token.second + ")";
    }

    try {
        regex token_regex(regex_pattern);
        smatch match;
        string::const_iterator search_start = input.cbegin();

        while (regex_search(search_start, input.cend(), match, token_regex)) {
            for (size_t i = 1, token_index = 0; i < match.size(); ++i, ++token_index) {
                if (match[i].matched) {
                    if (token_index < TOKENS.size()) {
                        string type = TOKENS[token_index].first;
                        string value = match[i].str();
                        if (type != "Space") {  // Пропускаем пробелы
                            tokens.push_back({ type, value });
                        }
                        break;
                    }
                }
            }
            search_start = match.suffix().first;
        }
    }
    catch (const regex_error& e) {
        MessageBoxA(nullptr, e.what(), "Regex Error", MB_OK | MB_ICONERROR);
    }

    return tokens;
}

// Функция для обработки текста и отображения токенов
void ProcessInput(HWND hwndInput, HWND hwndOutput) {
    int length = GetWindowTextLength(hwndInput);
    if (length == 0) {
        SetWindowText(hwndOutput, L"No input text provided.");
        return;
    }

    WCHAR* buffer = new WCHAR[length + 1];
    GetWindowText(hwndInput, buffer, length + 1);

    wstring ws(buffer);
    delete[] buffer;
    string input(ws.begin(), ws.end());

    vector<Token> tokens = lexer(input);

    wstringstream output;
    output << L"Token\t\tType\r\n";
    output << L"-----------------------------------------\r\n";

    for (const auto& token : tokens) {
        output << wstring(token.value.begin(), token.value.end()) << L"\t\t"
            << wstring(token.type.begin(), token.type.end()) << L"\r\n";
    }

    SetWindowText(hwndOutput, output.str().c_str());
}

// Главная функция
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LAB4FORM, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB4FORM));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB4FORM));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LAB4FORM);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 800, 630, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hwndInput, hwndOutput, hwndButton;

    switch (message) {
    case WM_CREATE:
        hwndInput = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE,
            10, 10, 760, 200, hWnd, (HMENU)IDC_INPUT_EDIT, hInst, nullptr);

        hwndOutput = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
            10, 220, 760, 300, hWnd, (HMENU)IDC_OUTPUT_EDIT, hInst, nullptr);

        hwndButton = CreateWindow(L"BUTTON", L"Process", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            340, 530, 100, 30, hWnd, (HMENU)IDC_PROCESS_BUTTON, hInst, nullptr);
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_PROCESS_BUTTON) {
            ProcessInput(hwndInput, hwndOutput);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
