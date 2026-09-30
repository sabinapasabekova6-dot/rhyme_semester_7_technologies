#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <windows.h>   
using namespace std;
//запрашиваем им€ входного файла с проверками:
//   1) им€ не пустое;
//   2) файл существует и открываетс€ дл€ чтени€;
//   3) файл не пустой.
string requestValidInputFile();

//строит им€ выходного файла на основе имени входного :
//   "input.txt" -> "input_numbered_sentences.txt"
string buildNumberedSentencesFileName(const string & inputFileName);

//читает входной файл, разбивает его на предложени€ (по точке, "!" и "?"),
//нумерует их и записывает каждое предложение с новой строки в файл
//»м€¬ходного‘айла_numbered_sentences.txt
void writeNumberedSentences(const string& inputFileName);
