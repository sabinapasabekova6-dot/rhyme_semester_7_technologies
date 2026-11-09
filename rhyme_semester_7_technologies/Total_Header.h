#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <vector>
#include <windows.h>   
using namespace std;


//ДЛЯ file_processing.cpp


//запрашиваем имя входного файла с проверками:
//   1) имя не пустое;
//   2) файл существует и открывается для чтения;
//   3) файл не пустой.
string requestValidInputFile();

//строит имя выходного файла на основе имени входного :
//   "input.txt" -> "input_numbered_sentences.txt"
string buildNumberedSentencesFileName(const string & inputFileName);

//проверяет, является ли символ русской или английской буквой
bool isLetterChar(char ch);

//проверяет, является ли символ закрывающей скобкой или закрывающей кавычкой
bool isClosingMark(char ch);

//убирает пробелы, табуляцию и переводы строк с конца строки
string trimTrailingWhitespace(string text);

//печатает currentSentence как очередное пронумерованное предложение
//(если в нём что-то есть кроме знаков препинания) и очищает currentSentence
void flushSentence(ofstream& outputFile, string& currentSentence, int& sentenceNumber);

//читает входной файл, разбивает его на предложения (по точке, "!", "?",
//а также по переводу строки после не буквенного знака), нумерует их и
//записывает каждое предложение с новой строки в файл
//ИмяВходногоФайла_numbered_sentences.txt
void writeNumberedSentences(const string& inputFileName);



vector<string> readNumberedSentences(const string& filename);
//ДЛЯ resilt_file.cpp


// СТРУКТУРЫ ДАННЫХ

#ifndef VERBSTRUCT_DEFINED
#define VERBSTRUCT_DEFINED

// структура которую заполняет серега
struct VerbStruct {
    string verb1;          // глагол1 
    string verb2;          // глагол2
    // а вместе глагол1 и глагол2 образуют рифму!
    uint64_t num_verb1;    // номер предложения где нашли глагол1 
    uint64_t num_verb2;    // номер предложения где нашли глагол2
};

// промежуточная структура для того чтобы собрать все предложения в которых есть произвольный глагол 
struct WordGlobalStats {
    string word;           // произвольный глагол
    int totalCount;        // суммируем sentences 
    vector<int> sentences; // перечисляем в каких предложениях встретился этот глагол
};

// результат 
struct RhymeStats {
    string verb1;                   // глагол1 
    string verb2;                   // глагол2
    int totalRhymeCount;            // (countVerb1 * countVerb2)
    int countVerb1;                 // суммируем sentencesVerb1
    int countVerb2;                 // суммируем sentencesVerb2
    vector<int> sentencesVerb1;     // перечисляем в каких предложениях встретился этот глагол1
    vector<int> sentencesVerb2;     // перечисляем в каких предложениях встретился этот глагол2
};


//  ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
 

// принимает строку по значению, возвращает строку в нижнем регистре
string toLowerStr(string s);

// идёт ли слово a раньше слова b в словаре 
bool isLessAlpha(const string& a, const string& b);

// добавляет глагол в вектор промежуточной структуры. 
// Принимает вектор по ссылке (для изменения), глагол, номер предложения где встр.
void addWordToGlobalStats(vector<WordGlobalStats>& globalStats, const string& word, int sentenceNum);

// сортирует вектор целых чисел по возрастанию. Принимает вектор по ссылке.
void sortNumbers(vector<int>& vec);

// из прост вектора чисел в строку 1,2,3.
string vectorToString(const vector<int>& vec);

// шейкерная сортировка вектора структур. Принимает по ссылке.
void shakerSort(vector<RhymeStats>& stats);



// ГЛАВНЫЕ ФУНКЦИИ


// ГЛАВНАЯ ФУНКЦИЯ ОБРАБОТКИ (принимает УКАЗАТЕЛЬ на вектор)
// возвращает вектор структур с готовой статистикой
vector<RhymeStats> processRhymes(vector<VerbStruct>* verbs);

// ФУНКЦИЯ ЗАПИСИ В ФАЙЛ
// принимает вектор по константной ссылке и имя файла по константной ссылке
void writeToFile(const vector<RhymeStats>& stats, const string& filename);




