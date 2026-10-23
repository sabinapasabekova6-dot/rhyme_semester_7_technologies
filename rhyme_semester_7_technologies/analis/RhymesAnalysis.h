#pragma once
#include "IdentVerbs.h"

using namespace std;

// Привести слово к фонетической записи
string to_phonetic(string_view word);

// Длина общего суффикса (с конца) в байтах/фонемах
size_t common_suffix_len(string_view a, string_view b);

// Коэффициент схожести 0...1 (на фонетических строках)
double similarity(string_view phonetic_a, string_view phonetic_b);

// -------------- Вспомогательные функции --------------------------

// Функция для проверки, является ли символ гласным
bool is_vowel(string_view c);
// Функция для проверки, является ли символ согласным
bool is_consonant(string_view c);
// Функция приведения к нижнему регистру
string lower_word(string_view word);



// struct RhymesInfo
// {
// 	VerbInfo verb1;
// 	VerbInfo verb2;
// 	double sim_coeff;
// };


// class RhymesAnalysis{
// public:
// 	RhymesAnalysis();
// 	~RhymesAnalysis() = default;

// 	// Метод определяющий пару на
// 	static bool is_rhyme(VerbInfo word1, VerbInfo word2);

// private:

// };
