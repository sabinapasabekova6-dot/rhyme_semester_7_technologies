

#include <string>
#include <fstream>
#include <iostream>
#include <windows.h>   
#include "Total_Header.h"  
using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    string path;  // переменная для имени входного файла

    do {
        // получаем название входного файла (с проверками и предупреждением о перезаписи)
        path = requestValidInputFile();

        // создаём файл с пронумерованными предложениями
        writeNumberedSentences(path);

        // чтото вроде заглушки
        vector<VerbStruct>* verbs = new vector<VerbStruct>();
        verbs->push_back({ "Косить", "пить", 1, 1 });
        verbs->push_back({ "Брить", "пить", 4, 4 });
        verbs->push_back({ "Брить", "пить", 4, 5 });
        verbs->push_back({ "Брить", "стелить", 1, 1 });
        verbs->push_back({ "Орать", "кричать ", 1, 1 });
        verbs->push_back({ "Бесить", "месить", 4, 4 });
        verbs->push_back({ "Бесить", "месить", 5, 5 });

        // считаем статистику рифм
        vector<RhymeStats> stats = processRhymes(verbs);

        // формируем имя выходного файла и пишем результат
        string resultFile = path + "_resilt_file";
        writeToFile(stats, resultFile);

        // освобождаем память, выделенную под вектор структур
        delete verbs;
        verbs = nullptr;

        cout << endl; 

    } while (!path.empty());  // пока функция вернула валидное имя - крутим цикл

    // создаем выходной файл с информацией об рифмах
    return 0;
}



    


    

