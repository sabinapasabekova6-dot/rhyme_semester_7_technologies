#pragma once

#include <cstdint>
#include <string>
#include <vector>
using namespace std;

// Морфема и её вес (1..100).
struct MorphemeRule {
    string text; // текст морфемы
    uint8_t score = 0; // вес совпадения
};

struct MorphRules {
    // Окончания по группам
    vector<MorphemeRule> endingsInfinitive;
    vector<MorphemeRule> endingsReflexive;
    vector<MorphemeRule> endingsPast;
    vector<MorphemeRule> endingsPresent;
    vector<MorphemeRule> endingsImperative;

    // Суффиксы
    vector<MorphemeRule> suffixesVerbal; // глагольные
    vector<string> suffixesNominal; // именные

    // Списки исключений и неглагольных форм (причастия и деепричастия)
    vector<string> blacklist;
    vector<string> gerundSuffixes;
    vector<string> participleSuffixes;

    // Читает все .txt файлы из каталога "lib/"
    // Возвращаемые значения:
	//		true - если был успешно прочитан хотя бы один файл
	// 		false - иначе
    bool load_from_lib();

    static const MorphRules& instance();
};