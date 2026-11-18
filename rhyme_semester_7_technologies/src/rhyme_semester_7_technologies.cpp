#include <string>
#include <filesystem>
#include <iostream>
#include <ctime>
#include <windows.h>
#include "Total_Header.h"
#include "MorphRules.h"
#include "RhymesAnalysis.h"

// Настройки анализа (меняются здесь в одном месте)
const int MIN_VERB_SCORE = 50; // минимальный вес слова, чтобы считать его глаголом (0..100)
const double MIN_SIMILARITY = 0.5; // минимальная похожесть окончаний для рифмы (0.0..1.0)

static bool outputExists(const string& path) {
    std::error_code error;
    return std::filesystem::exists(path, error);
}

static void analyzeFile(const string& path) {
    const string numberedPath = buildNumberedSentencesFileName(path);
    const string outputPath = path + "_result_file.txt";
    const string debugPath = path + "_verbs_debug.txt";

    if (outputExists(numberedPath) || outputExists(outputPath) || outputExists(debugPath)) {
        cout << "Выходные файлы для этого текста уже существуют и будут перезаписаны при записи.\n";
    }

    writeNumberedSentences(path);
    vector<string> sentences = readNumberedSentences(numberedPath);

    vector<RhymeStats> stats;

    if (sentences.empty()) {
        cout << "Не удалось прочитать предложения для анализа.\n";
        // НЕ выходим — ниже создадим пустой файл статистики
    } else {
        cout << "\nПредложений для анализа: " << sentences.size() << "\n";

        clock_t startTime = clock(); // для замера времени

        // 1) все глаголы текста, по одной записи на каждое вхождение
        vector<VerbInfo> verbs = collect_verbs(sentences, MIN_VERB_SCORE);
        cout << "Найдено глаголов в тексте (с повторами): " << verbs.size() << "\n";

        // отладочный файл: какие слова приняты за глаголы и с каким весом
        // (если он не нужен - закомментируйте следующую строку)
        writeVerbsDebug(verbs, debugPath);

        // 2) статистика по парам рифм
        if (!verbs.empty()) {
            stats = buildRhymeStats(verbs, MIN_SIMILARITY);
        } else {
            cout << "Глаголов не найдено — файл статистики будет пустым "
                    "(только заголовок).\n";
        }
        cout << "Найдено пар рифм: " << stats.size() << "\n";

        double seconds = (double)(clock() - startTime) / CLOCKS_PER_SEC;
        cout << "Время анализа, секунд: " << seconds << "\n";
    }

    // ВСЕГДА пишем файл — даже если статистика пустая
    writeToFile(stats, outputPath);

    cout << "Готово.\n\n";
}

int main() {
    // UTF-8 ввод/вывод консоли
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    // setlocale НЕ трогаем — оставляем "C", чтобы isalpha/isspace
    // работали только с ASCII, а UTF-8-байты >= 128 они не трогали

    cout << "Анализ рифмующихся глаголов\n\n";
    const MorphRules& rules = MorphRules::instance();
    if (rules.endingsInfinitive.empty() && rules.endingsPast.empty() && rules.endingsPresent.empty()) {
        cout << "Ошибка: не найдены файлы словаря в папке lib. "
                "Проверьте рабочую папку программы.\n";
        cin.get();
        return 1;
    }
    while (true) {
        const string path = requestValidInputFile();
        if (path.empty()) {
            cout << "Программа завершена.\n";
            return 0;
        }

        analyzeFile(path);
    }
}
