#include <string>
#include <fstream>
#include <iostream>
#include <windows.h>
#include "Total_Header.h"
#include "analis/RhymesAnalysis.h"

int main() {
    // UTF-8 ввод/вывод консоли
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    // setlocale НЕ трогаем — оставляем "C", чтобы isalpha/isspace
    // работали только с ASCII, а UTF-8-байты >= 128 они не трогали

    cout << "Анализ рифмующихся глаголов\n\n";

    string path = requestValidInputFile();
    writeNumberedSentences(path);

    string numberedPath = buildNumberedSentencesFileName(path);
    vector<string> sentences = readNumberedSentences(numberedPath);

    vector<RhymeStats> stats;

    if (sentences.empty()) {
        cout << "Не удалось прочитать предложения для анализа.\n";
        // НЕ выходим — ниже создадим пустой файл статистики
    } else {
        cout << "\nПредложений для анализа: " << sentences.size() << "\n";
        cout << "Ищем пары рифмующихся глаголов (может занять время)...\n";

        vector<VerbStruct> verbs = find_rhymes(sentences, 50, 0.5);
        cout << "Найдено пар-кандидатов: " << verbs.size() << "\n";

        if (!verbs.empty()) {
            stats = processRhymes(&verbs);
        } else {
            cout << "Рифмующихся пар не найдено — файл статистики будет пустым "
                    "(только заголовок).\n";
        }
    }

    // ВСЕГДА пишем файл — даже если статистика пустая
    string outputPath = path + "_result_file.txt";
    writeToFile(stats, outputPath);

    cout << "Готово.\n";
    return 0;
}