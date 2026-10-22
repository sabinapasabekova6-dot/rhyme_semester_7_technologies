

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
    //получаем название входного файла
    string path = requestValidInputFile();

    //создаём файл с пронумерованными предложениями
    writeNumberedSentences(path);
   

    //чтото вроде заглушки 
    vector<VerbStruct>* verbs = new vector<VerbStruct>();
    verbs->push_back({ "Косить", "пить", 1, 1 });
    verbs->push_back({ "Брить", "пить", 4, 4 });
    verbs->push_back({ "Брить", "пить", 4, 5 });
    verbs->push_back({ "Брить", "стелить", 1, 1 });
    verbs->push_back({ "Орать", "кричать ", 1, 1 });
    verbs->push_back({ "Бесить", "месить", 4, 4 });
    verbs->push_back({ "Бесить", "месить", 5, 5 });

    vector<RhymeStats> stats = processRhymes(verbs);
    path = path + "_resilt_file";
    writeToFile(stats, path);
    verbs = nullptr;


    // создаем выходной файл с информацией об рифмах 
    return 0;
}




