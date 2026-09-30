
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
   
    return 0;
}




