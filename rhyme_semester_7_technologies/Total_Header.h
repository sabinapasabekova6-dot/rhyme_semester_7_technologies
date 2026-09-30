#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <windows.h>   
using namespace std;
//запрашиваем имя входного файла с проверками:
//   1) имя не пустое;
//   2) файл существует и открывается для чтения;
//   3) файл не пустой.
string requestValidInputFile();

