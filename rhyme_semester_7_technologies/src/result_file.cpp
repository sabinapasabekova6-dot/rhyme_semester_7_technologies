#define _CRT_SECURE_NO_WARNINGS
#include "Total_Header.h"
using namespace std;

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <map>
#include <algorithm>
#include "RhymesAnalysis.h"


// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ

// для std::sort: идет ли слово a раньше слова b по алфавиту
bool wordLess(const WordGlobalStats& a, const WordGlobalStats& b) {
    return a.word < b.word;
}

// принимает строку по значению, возвращает строку в нижнем регистре
string toLowerStr(string s) {
    return lower_word(s);
}

// идёт ли слово а раньше слова b  в словаре 
bool isLessAlpha(const string& a, const string& b) {

    return toLowerStr(a) < toLowerStr(b);
}

// добавляет ОДНО вхождение глагола в таблицу слов.
// Параметры: таблица слов и индекс для быстрого поиска (оба по ссылке, изменяются),
// слово, его основа, номер предложения, где слово встретилось.
void addWordToGlobalStats(vector<WordGlobalStats>& globalStats, map<string, int>& indexOfWord,
                          const string& word, const string& stem, int sentenceNum) {
    // если слова еще нет в таблице - создаем для него пустую запись
    if (indexOfWord.count(word) == 0) {
        WordGlobalStats newStat;
        newStat.word = word;
        newStat.stem = stem;
        newStat.totalCount = 0;
        globalStats.push_back(newStat);
        indexOfWord[word] = (int)globalStats.size() - 1; // запоминаем, где запись лежит
    }

    // учитываем текущее вхождение слова
    int index = indexOfWord[word];
    globalStats[index].totalCount++;
    globalStats[index].sentences.push_back(sentenceNum);
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

    for (size_t i = 0; i < vec.size(); i++) {
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
        bool wasSwap = false; // был ли в этом круге хотя бы один обмен

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
                wasSwap = true;
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
                wasSwap = true;
            }
        }
        left++; // сужение левой границы, так как минимальный элемент встал на свое место

        // если за целый круг не было ни одного обмена - массив уже отсортирован
        if (!wasSwap) {
            break;
        }
    }
}


// дополняет строку пробелами справа до width СИМВОЛОВ (а не байт)
string padRight(const string& s, int width) {
    int len = 0;
    for (int i = 0; i < (int)s.size(); i++) {
        unsigned char c = (unsigned char)s[i];
        if ((c & 0xC0) != 0x80) {
            len++;
        }
    }
    string result = s;
    while (len < width) {
        result += ' ';
        len++;
    }
    return result;
}

void writeRow(FILE* out, const string& c1, const string& c2, const string& c3,
              const string& c4, const string& c5, const string& c6) {
    fprintf(out, "%s | %s | %s | %s | %s | %s\n",
        padRight(c1, 30).c_str(),
        padRight(c2, 25).c_str(),
        padRight(c3, 20).c_str(),
        padRight(c4, 30).c_str(),
        padRight(c5, 20).c_str(),
        padRight(c6, 30).c_str());
}

// ГЛАВНАЯ ФУНКЦИЯ ОБРАБОТКИ
// Принимает все найденные глаголы (по одной записи на каждое вхождение в текст)
// и порог похожести. Возвращает статистику по парам рифм.
vector<RhymeStats> buildRhymeStats(const vector<VerbInfo>& verbs, double minSimilarity) {
    vector<RhymeStats> result; // итоговый вектор (пока пустой)

    if (verbs.empty()) {
        cout << "Глаголов нет, нечего обрабатывать!" << endl;
        return result;
    }

    // ШАГ 1. Таблица уникальных слов. Каждое вхождение слова учитывается
    // ровно один раз, поэтому счетчики и номера предложений получаются точными.
    vector<WordGlobalStats> words;
    map<string, int> indexOfWord;
    for (int i = 0; i < (int)verbs.size(); i++) {
        addWordToGlobalStats(words, indexOfWord, verbs[i].verb, verbs[i].stem, (int)verbs[i].num);
    }

    // номера предложений по возрастанию (1, 3, 5, 5)
    for (int i = 0; i < (int)words.size(); i++) {
        sortNumbers(words[i].sentences);
    }

    // слова по алфавиту: тогда пары ниже получатся сразу в алфавитном порядке
    sort(words.begin(), words.end(), wordLess);

    cout << "Уникальных глаголов: " << words.size() << endl;

    // ШАГ 2. Фонетическая запись считается один раз для каждого уникального слова
    vector<string> phonetic(words.size());
    for (int i = 0; i < (int)words.size(); i++) {
        phonetic[i] = to_phonetic(words[i].word);
    }

    // ШАГ 3. Перебираем пары УНИКАЛЬНЫХ слов (i < j), поэтому пара
    // "слово - то же слово" и повторы пар возникнуть не могут
    for (int i = 0; i < (int)words.size(); i++) {
        for (int j = i + 1; j < (int)words.size(); j++) {

            // окончания недостаточно похожи - не рифма
            if (similarity(phonetic[i], phonetic[j]) < minSimilarity) {
                continue;
            }
            // TODO: Подумать над этим
            // // однокоренные слова (арестовали - арестовать) - не рифма
            // if (is_same_root(words[i].word, words[i].stem, words[j].word, words[j].stem)) {
            //     continue;
            // }

            RhymeStats rs;
            rs.verb1 = words[i].word;
            rs.verb2 = words[j].word;
            rs.countVerb1 = words[i].totalCount;
            rs.countVerb2 = words[j].totalCount;
            rs.sentencesVerb1 = words[i].sentences;
            rs.sentencesVerb2 = words[j].sentences;
            rs.totalRhymeCount = rs.countVerb1 * rs.countVerb2;
            result.push_back(rs);
        }
    }

    // ШАГ 4. Шейкерная сортировка по алфавиту (пары и так почти упорядочены,
    // поэтому благодаря выходу при отсутствии обменов она отработает быстро)
    shakerSort(result);

    return result;
}

// ========================== ОТЛАДКА ===================================
// строка таблицы для файла отладки
struct DebugRow {
    VerbInfo info; // слово, основа, окончание, вес
    int count;     // сколько раз слово встретилось в тексте
};
// сначала слова с меньшим весом (самые сомнительные), при равном весе - по алфавиту
bool debugRowLess(const DebugRow& a, const DebugRow& b) {
    if (a.info.score != b.info.score) {
        return a.info.score < b.info.score;
    }
    return a.info.verb < b.info.verb;
}
// записывает в файл все найденные слова: вес, основа, окончание, сколько раз встретилось
void writeVerbsDebug(const vector<VerbInfo>& verbs, const string& filename) {
    vector<DebugRow> rows;
    map<string, int> indexOfWord;

    for (int i = 0; i < (int)verbs.size(); i++) {
        if (indexOfWord.count(verbs[i].verb) == 0) {
            DebugRow row;
            row.info = verbs[i];
            row.count = 0;
            rows.push_back(row);
            indexOfWord[verbs[i].verb] = (int)rows.size() - 1;
        }
        rows[indexOfWord[verbs[i].verb]].count++;
    }

    sort(rows.begin(), rows.end(), debugRowLess);

    FILE* out = fopen(filename.c_str(), "w");
    if (!out) {
        cout << "Ошибка: не удалось открыть файл: " << filename << endl;
        return;
    }
    fputs("\xEF\xBB\xBF", out);

    fprintf(out, "%s | %s | %s | %s | %s\n",
        padRight("Слово", 25).c_str(), padRight("Вес", 5).c_str(),
        padRight("Основа", 20).c_str(), padRight("Окончание", 12).c_str(),
        "Раз в тексте");

    for (int i = 0; i < (int)rows.size(); i++) {
        fprintf(out, "%s | %s | %s | %s | %d\n",
            padRight(rows[i].info.verb, 25).c_str(),
            padRight(to_string((int)rows[i].info.score), 5).c_str(),
            padRight(rows[i].info.stem, 20).c_str(),
            padRight(rows[i].info.ending, 12).c_str(),
            rows[i].count);
    }

    fclose(out);
    cout << "Отладочный файл записан: " << filename << endl;
}

// =====================================================================================

// ФУНКЦИЯ ЗАПИСИ В ФАЙЛ
//  принимает вектор по константной ссылке и имя файла по константной ссылке
void writeToFile(const vector<RhymeStats>& stats, const string& filename) {
    FILE* out = fopen(filename.c_str(), "w");
    if (!out) {
        cout << "Ошибка: не удалось открыть файл: " << filename << endl;
        return;
    }

    // BOM, чтобы Блокнот и Excel узнали UTF-8
    fputs("\xEF\xBB\xBF", out);

    writeRow(out, "Рифмы", "Сколько раз рифма", "Сколько раз", "В каких предложениях", "Сколько раз", "В каких предложениях");
    writeRow(out, "", "встретилась в тексте", "в тексте", "встретилось 1-е слово", "в тексте", "встретилось 2-е слово");
    writeRow(out, "", "", "встретилось", "", "встретилось", "");
    writeRow(out, "", "", "1-е слово", "", "2-е слово", "");

    string line(170, '-');
    fprintf(out, "%s\n", line.c_str());

    for (int i = 0; i < (int)stats.size(); i++) {
        string rhymeStr = to_string(i + 1) + ") " + stats[i].verb1 + ", " + stats[i].verb2;
        writeRow(out,
            rhymeStr,
            to_string(stats[i].totalRhymeCount),
            to_string(stats[i].countVerb1),
            vectorToString(stats[i].sentencesVerb1),
            to_string(stats[i].countVerb2),
            vectorToString(stats[i].sentencesVerb2));
    }

    fclose(out);
    cout << "Результаты записаны в файл: " << filename << endl;
}