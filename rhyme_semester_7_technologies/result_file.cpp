#define _CRT_SECURE_NO_WARNINGS
#include "Total_Header.h"
using namespace std;

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <windows.h>
#include <locale.h>
#include <cstdio>


// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ

// принимает строку по значению, возвращает строку в нижнем регистре
string toLowerStr(string s) {
    string res = "";

    for (int i = 0; i < s.length(); i++) {
        char c = s[i];

        // если большая английская 
        if (c >= 'A' && c <= 'Z') {
            // делаем маленькую англ
            res += (char)(c - 'A' + 'a');
        }
        //если большая рус 
        else if ((unsigned char)c >= 192 && (unsigned char)c <= 223) {
            // делаем маленькую рус 
            res += (char)(c + 32);
        }
        // если  'Ё' 
        else if ((unsigned char)c == 168) {
            res += (char)184; //делаем  'ё'
        }
        // если символ маленькая буква или вообще не буква 
        else {
            res += c; // добавляем к результату 
        }
    }
    return res;
}

// идёт ли слово а раньше слова b  в словаре 
bool isLessAlpha(const string& a, const string& b) {

    return toLowerStr(a) < toLowerStr(b);
}

//  добавляет глагол в вектор промежуточной структуры . 
// Принимает вектор по ссылке (для изменения), глагол, номер предложения где встр.
void addWordToGlobalStats(vector<WordGlobalStats>& globalStats, const string& word, int sentenceNum) {
    int foundIndex = -1; // -1 означает "не найдено"

    for (int i = 0; i < globalStats.size(); i++) {
        // если какойто глагол из вектора такой же как и добавляемый глагол
        if (globalStats[i].word == word) {
            foundIndex = (int)i; // сохр его индекс
            break; //  дальше искать не нужно
        }
    }


    if (foundIndex != -1) { // Если слово уже есть в векторе
        globalStats[foundIndex].totalCount++; // увеличиваем кол-во сколько раз глагол встр в тексте 
        globalStats[foundIndex].sentences.push_back(sentenceNum); // добавляем номер предложения в конец вектора 
    }
    else { // Если слово еще не встречалось
        WordGlobalStats newStat; //создаём  локальный экземпляр структуры
        newStat.word = word;
        newStat.totalCount = 1; // тип ток 1 раз встр в тексте 
        newStat.sentences.push_back(sentenceNum); //  добавление первого номера предложения
        globalStats.push_back(newStat); // добавление новой структуры в конец вектора globalStats
    }
}


// сортирует вектор целых чисел по возрастанию. Принимает вектор по ссылке.
void sortNumbers(vector<int>& vec) {
    int n = (int)vec.size(); // сколько всего элементов 

    // пузырёк..
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            // если левый элемент больше правого, перестановка 
            if (vec[j] > vec[j + 1]) {
                int temp = vec[j];
                vec[j] = vec[j + 1];
                vec[j + 1] = temp;
            }
        }
    }
}

//  из прост вектора чисел в строку 1,2,3.
string vectorToString(const vector<int>& vec) {
    string result = "";

    for (int i = 0; i < vec.size(); i++) {
        // Метод: конкатенация строк. to_string() преобразует int в string
        result += to_string(vec[i]);

        // не является ли текущий элемент последним
        if (i < vec.size() - 1) {
            result += ","; //  добавление разделителя-запятой между числами
        }
    }

    // если результат заполнен и все чиса перенесены ставим точку 
    if (!result.empty()) {
        result += ".";
    }
    return result;
}

// шейкерная сортировка  вектора структур. Принимает по ссылке.
void shakerSort(vector<RhymeStats>& stats) {
    int n = (int)stats.size(); // скок всего элементов сортирвать 
    if (n <= 1) return; // на всякий

    int left = 0;
    int right = n - 1;

    // пока границы не сошлись
    while (left < right) {

        //  СЛЕВА НАПРАВО (всплытие максимального элемента вправо)
        for (int i = left; i < right; i++) {
            bool needSwap = false; // флаг необходимости обмена

            //сравнение 
            if (isLessAlpha(stats[i + 1].verb1, stats[i].verb1)) {// тру если не в том порядке 
                needSwap = true; // нужно поменять 
            }
            //  если первые слова равны , сравниваем вторые слова
            else if (toLowerStr(stats[i].verb1) == toLowerStr(stats[i + 1].verb1)) {
                if (isLessAlpha(stats[i + 1].verb2, stats[i].verb2)) {
                    needSwap = true; // установка флага, порядок вторых слов нарушен
                }
            }

            //  обмен 
            if (needSwap) {
                RhymeStats temp = stats[i];
                stats[i] = stats[i + 1];
                stats[i + 1] = temp;
            }
        }
        right--; //  сужение правой границы, так как максимальный элемент встал на свое место

        //  СПРАВА НАЛЕВО (всплытие минимального элемента влево)
        for (int i = right; i > left; i--) {
            bool needSwap = false; //  сброс флага для нового прохода

            // сравнение текущих элементов с предыдущими
            if (isLessAlpha(stats[i].verb1, stats[i - 1].verb1)) {
                needSwap = true;
            }
            else if (toLowerStr(stats[i - 1].verb1) == toLowerStr(stats[i].verb1)) {
                if (isLessAlpha(stats[i].verb2, stats[i - 1].verb2)) {
                    needSwap = true;
                }
            }

            //  обмен
            if (needSwap) {
                RhymeStats temp = stats[i];
                stats[i] = stats[i - 1];
                stats[i - 1] = temp;
            }
        }
        left++; // сужение левой границы, так как минимальный элемент встал на свое место
    }
}


// ГЛАВНАЯ ФУНКЦИЯ ОБРАБОТКИ (принимает УКАЗАТЕЛЬ на вектор)
vector<RhymeStats> processRhymes(vector<VerbStruct>* verbs) {

    //  проверка указателя на нулевое значение  для предотвращения падения программы
    if (verbs == nullptr) {
        cout << "ОШИБКА: передан пустой указатель !" << endl;
        vector<RhymeStats> empty;
        return empty;
    }

    //  проверка наличия элементов
    if (verbs->empty()) {
        cout << "вектор пустой, нечего обрабатывать!" << endl;
        vector<RhymeStats> empty;
        return empty;
    }

    cout << "Получено структур для обработки: " << verbs->size() << endl;

    vector<WordGlobalStats> globalStats; // объявление локального вектора для промежуточной статистики

    //  перебор всех элементов вектора по указателю
    for (int i = 0; i < verbs->size(); i++) {
        // сбор всех глаголов в вектор структур 
        addWordToGlobalStats(globalStats, verbs->at(i).verb1, (int)verbs->at(i).num_verb1);
        addWordToGlobalStats(globalStats, verbs->at(i).verb2, (int)verbs->at(i).num_verb2);
    }

    // объявление локальной структуры для временного хранения уникальных пар
    struct SimplePair { string w1; string w2; };
    vector<SimplePair> uniquePairs;

    // формируем список уникальных пар рифм
    for (int i = 0; i < verbs->size(); i++) {
        string w1 = verbs->at(i).verb1;
        string w2 = verbs->at(i).verb2;

        //  нормализация порядка слов (чтобы "Брить-пить" и "пить-Брить" стали одной парой)
        if (isLessAlpha(w2, w1)) {
            string temp = w1;
            w1 = w2;
            w2 = temp;
        }

        bool exists = false;// флаг добавлять ли пару? 

        //  линейный поиск пары в векторе uniquePairs
        for (int j = 0; j < uniquePairs.size(); j++) {
            if (uniquePairs[j].w1 == w1 && uniquePairs[j].w2 == w2) {
                exists = true;
                break;
            }
        }

        //  добавление пары только если она еще не встречалась
        if (!exists) {
            SimplePair sp;
            sp.w1 = w1;
            sp.w2 = w2;
            uniquePairs.push_back(sp);
        }
    }

    vector<RhymeStats> finalStats; // объявление итогового вектора результатов

    //  цикл for для формирования итоговой статистики по каждой уникальной паре
    for (int i = 0; i < uniquePairs.size(); i++) {
        RhymeStats rs;
        rs.verb1 = uniquePairs[i].w1;
        rs.verb2 = uniquePairs[i].w2;
        rs.countVerb1 = 0;
        rs.countVerb2 = 0;

        // линейный поиск статистики 1-го слова в глобальном векторе
        for (int j = 0; j < globalStats.size(); j++) {
            if (globalStats[j].word == rs.verb1) {
                rs.countVerb1 = globalStats[j].totalCount;
                rs.sentencesVerb1 = globalStats[j].sentences; // Копируем ВСЕ номера, включая дубликаты!
                break;
            }
        }

        //  линейный поиск статистики 20го слова в глобальном векторе
        for (int j = 0; j < globalStats.size(); j++) {
            if (globalStats[j].word == rs.verb2) {
                rs.countVerb2 = globalStats[j].totalCount;
                rs.sentencesVerb2 = globalStats[j].sentences; // Копируем ВСЕ номера, включая дубликаты!
                break;
            }
        }

        // скок раз рифма встр в предлежении 
        rs.totalRhymeCount = rs.countVerb1 * rs.countVerb2;

        // сортируем вектор, чтобы номера шли по возрастанию (1, 3, 5, 5)
        sortNumbers(rs.sentencesVerb1);
        sortNumbers(rs.sentencesVerb2);

        finalStats.push_back(rs); //  добавление готовой структуры в итоговый вектор
    }

    // вызов функции шейкерной сортировки для упорядочивания итогового вектора по алфавиту
    shakerSort(finalStats);

    return finalStats; // возврат готового вектора результатов
}



// ФУНКЦИЯ ЗАПИСИ В ФАЙЛ
//  принимает вектор по константной ссылке и имя файла по константной ссылке
void writeToFile(const vector<RhymeStats>& stats, const string& filename) {

    FILE* out = fopen(filename.c_str(), "w");

    if (!out) {
        cout << "Ошибка: не удалось открыть файл: " << filename << endl;
        return;
    }

    //печатаем  4 строки так как заголовки длинные 
    fprintf(out, "%-30s | %-25s | %-20s | %-30s | %-20s | %-30s\n",
        "Рифмы", "Сколько раз рифма", "Сколько раз", "В каких предложениях", "Сколько раз", "В каких предложениях");

    fprintf(out, "%-30s | %-25s | %-20s | %-30s | %-20s | %-30s\n",
        "", "встретилась в тексте", "в тексте", "встретилось 1-е слово", "в тексте", "встретилось 2-е слово");

    fprintf(out, "%-30s | %-25s | %-20s | %-30s | %-20s | %-30s\n",
        "", "", "встретилось", "", "встретилось", "");

    fprintf(out, "%-30s | %-25s | %-20s | %-30s | %-20s | %-30s\n",
        "", "", "1-е слово", "", "2-е слово", "");

    fprintf(out, "-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

    //  построчная запись данных из вектора
    for (int i = 0; i < stats.size(); i++) {
        //  номера строк и скобка + глагол1, глагол2
        string rhymeStr = to_string(i + 1) + ") " + stats[i].verb1 + ", " + stats[i].verb2;

        // вывод
        fprintf(out, "%-30s | %-25d | %-20d | %-30s | %-20d | %-30s\n",
            rhymeStr.c_str(),
            stats[i].totalRhymeCount,
            stats[i].countVerb1,
            vectorToString(stats[i].sentencesVerb1).c_str(),
            stats[i].countVerb2,
            vectorToString(stats[i].sentencesVerb2).c_str()
        );
    }

    fclose(out);
    cout << "Результаты записаны в файл: " << filename << endl; //  информационный вывод в консоль
}