

#include <string>
#include <fstream>
#include <iostream>
#include <windows.h>   
#include "Total_Header.h"        // ваша часть: VerbStruct, RhymeStats, processRhymes...
#include "RhymesAnalysis.h"      // часть коллеги: find_rhymes, VerbInfo, IdentVerbs...

int main() {
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    cout << "Анализ рифмующихся глаголов\n\n";

    // запрос имени входного файла (проверки: не пусто, открывается, не пустой)
    string path = requestValidInputFile();

    // разбиение текста на предложения с нумерацией
    // Результат сохраняется в файл "<path>_numbered_sentences.txt"
    writeNumberedSentences(path);

    // читаем этот файл обратно в вектор строк
    // Каждая строка имеет вид "N) текст предложения"
    string numberedPath = buildNumberedSentencesFileName(path);
    vector<string> sentences = readNumberedSentences(numberedPath);

    if (sentences.empty()) {
        cout << "Не удалось прочитать предложения для анализа.\n";
        return 1;
    }

    cout << "\nПредложений для анализа: " << sentences.size() << "\n";
    cout << "Ищем пары рифмующихся глаголов (может занять время)...\n";

    //ищем пары рифмующихся глаголов (функция коллеги)
    //   min_verb_score = 50  — минимальный вес, чтобы слово считалось глаголом
    //   min_similarity = 0.5 — минимальный коэффициент фонетической схожести
    vector<VerbStruct> verbs = find_rhymes(sentences, 50, 0.5);

    cout << "Найдено пар-кандидатов: " << verbs.size() << "\n";

    if (verbs.empty()) {
        cout << "Рифмующихся пар не найдено. Завершение работы.\n";
        return 0;
    }

    //строим статистику по найденным парам (ваша функция)
    //   processRhymes принимает УКАЗАТЕЛЬ — передаём адрес вектора
    vector<RhymeStats> stats = processRhymes(&verbs);

    // пишем результат в файл
    //   имя: "<path>_result_file.txt" (исправлена опечатка "resilt")
    string outputPath = path + "_result_file.txt";
    writeToFile(stats, outputPath);

    cout << "Готово.\n";
    return 0;
}