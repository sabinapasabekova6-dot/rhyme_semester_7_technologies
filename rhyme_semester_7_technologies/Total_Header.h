#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
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

//провер€ет, €вл€етс€ ли символ русской или английской буквой
bool isLetterChar(char ch);

//провер€ет, €вл€етс€ ли символ закрывающей скобкой или закрывающей кавычкой
bool isClosingMark(char ch);

//убирает пробелы, табул€цию и переводы строк с конца строки
string trimTrailingWhitespace(string text);

//печатает currentSentence как очередное пронумерованное предложение
//(если в нЄм что-то есть кроме знаков препинани€) и очищает currentSentence
void flushSentence(ofstream& outputFile, string& currentSentence, int& sentenceNumber);

//читает входной файл, разбивает его на предложени€ (по точке, "!", "?",
//а также по переводу строки после не буквенного знака), нумерует их и
//записывает каждое предложение с новой строки в файл
//»м€¬ходного‘айла_numbered_sentences.txt
void writeNumberedSentences(const string& inputFileName);