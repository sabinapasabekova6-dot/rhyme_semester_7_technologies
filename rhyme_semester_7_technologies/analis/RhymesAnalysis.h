#pragma once
#include "IdentVerbs.h"
#include <string_view> 
#include <cctype> 

using namespace std;

// Привести слово к фонетической записи
string to_phonetic(string_view word);

// Длина общего суффикса (с конца) в символах
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
// Длина последнего UTF-8 символа в строке в байтах.
// Параметры: s - входная строка (0 для пустой).
size_t last_char_bytes(string_view s);
// Количество UTF-8 символов в строке.
// Параметры: s - входная строка.
size_t count_chars(string_view s);


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

#ifndef VERBSTRUCT_DEFINED
#define VERBSTRUCT_DEFINED

// Пара рифмующихся глаголов.
struct VerbStruct {
    string   verb1;       // первый глагол
    string   verb2;       // второй глагол
    uint64_t num_verb1;   // номер предложения первого глагола
    uint64_t num_verb2;   // номер предложения второго глагола
};

// Извлекает номер предложения из начала строки.
// Параметры:
//   body - строка, после вызова обрезается до текста без номера.
// Возвращает номер или 0, если номера нет.
uint64_t extract_sentence_num(string_view& body);

// Коэффициент рифмы двух глаголов (от 0.0 до 1.0).
// Параметры: w1, w2 - структуры VerbInfo (поле verb).
double rhyme_similarity(const VerbInfo& w1, const VerbInfo& w2);

// Проверяет, рифмуются ли два глагола.
// Параметры:
//   w1, w2 - глаголы.
//   min_similarity - порог коэффициента рифмы (по умолчанию 0.5).
bool is_rhyme(const VerbInfo& w1, const VerbInfo& w2,
				double min_similarity = 0.5);


// Ищет пары рифмующихся глаголов в списке предложений.
// Параметры:
//   sentences      - предложения, каждое с разметкой "N. ..." (N - номер);
//   min_verb_score - минимальный вес глагола (0..100);
//   min_similarity - минимальный коэффициент рифмы (0.0..1.0).
// Возвращает список пар без дублей.
vector<VerbStruct> find_rhymes(
    const vector<string>& sentences,
    uint8_t min_verb_score,
    double min_similarity);