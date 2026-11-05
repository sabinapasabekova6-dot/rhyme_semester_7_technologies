#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <windows.h>   
using namespace std;


//ƒЋя file_processing.cpp


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


//ƒЋя resilt_file.cpp


// —“–” “”–џ ƒјЌЌџ’


// структура которую заполн€ет серега
struct VerbStruct {
    string verb1;          // глагол1 
    string verb2;          // глагол2
    // а вместе глагол1 и глагол2 образуют рифму!
    uint64_t num_verb1;    // номер предложени€ где нашли глагол1 
    uint64_t num_verb2;    // номер предложени€ где нашли глагол2
};

// промежуточна€ структура дл€ того чтобы собрать все предложени€ в которых есть произвольный глагол 
struct WordGlobalStats {
    string word;           // произвольный глагол
    int totalCount;        // суммируем sentences 
    vector<int> sentences; // перечисл€ем в каких предложени€х встретилс€ этот глагол
};

// результат 
struct RhymeStats {
    string verb1;                   // глагол1 
    string verb2;                   // глагол2
    int totalRhymeCount;            // (countVerb1 * countVerb2)
    int countVerb1;                 // суммируем sentencesVerb1
    int countVerb2;                 // суммируем sentencesVerb2
    vector<int> sentencesVerb1;     // перечисл€ем в каких предложени€х встретилс€ этот глагол1
    vector<int> sentencesVerb2;     // перечисл€ем в каких предложени€х встретилс€ этот глагол2
};


//  ¬—ѕќћќ√ј“≈Ћ№Ќџ≈ ‘”Ќ ÷»»
 

// принимает строку по значению, возвращает строку в нижнем регистре
string toLowerStr(string s);

// идЄт ли слово a раньше слова b в словаре 
bool isLessAlpha(const string& a, const string& b);

// добавл€ет глагол в вектор промежуточной структуры. 
// ѕринимает вектор по ссылке (дл€ изменени€), глагол, номер предложени€ где встр.
void addWordToGlobalStats(vector<WordGlobalStats>& globalStats, const string& word, int sentenceNum);

// сортирует вектор целых чисел по возрастанию. ѕринимает вектор по ссылке.
void sortNumbers(vector<int>& vec);

// из прост вектора чисел в строку 1,2,3.
string vectorToString(const vector<int>& vec);

// шейкерна€ сортировка вектора структур. ѕринимает по ссылке.
void shakerSort(vector<RhymeStats>& stats);



// √Ћј¬Ќџ≈ ‘”Ќ ÷»»


// √Ћј¬Ќјя ‘”Ќ ÷»я ќЅ–јЅќ“ » (принимает ” ј«ј“≈Ћ№ на вектор)
// возвращает вектор структур с готовой статистикой
vector<RhymeStats> processRhymes(vector<VerbStruct>* verbs);

// ‘”Ќ ÷»я «јѕ»—» ¬ ‘ј…Ћ
// принимает вектор по константной ссылке и им€ файла по константной ссылке
void writeToFile(const vector<RhymeStats>& stats, const string& filename);




