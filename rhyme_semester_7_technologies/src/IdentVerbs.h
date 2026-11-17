#pragma once
#include <string>
#include <cstdint>
#include <vector>

using std::string;
using std::vector;

struct VerbInfo{
	string verb;   // сам глагол
	string stem;   // корень глагола (предположительный)
	string ending; // окончание глагола
	uint8_t score = 0; // "вес" определения того, глагол ли это (от 1 до 100)
	uint64_t num = 0;  // номер предложения из которого взят глагол
}; 

class IdentVerbs {
public:
	explicit IdentVerbs(const string& src);
	~IdentVerbs() = default;

	// -------------- Основной интерфейс -------------------------------------
	// Анализ одного слова в UTF-8, регистр может быть любым.
    // Параметры: word - слово без пробелов и знаков пунктуации.
    // Возвращает VerbInfo: verb в нижнем регистре, score - вес 0..100.
    // Нулевой вес означает отказ, 100 может означать словарное подтверждение.
	static VerbInfo analysis_word(const string& word);
	// Метод анализирующий и возвращающий статистику по всем словам предложения 
	vector<VerbInfo> analysis_all_words();
	// Метод возвращающий только слова с установленным порогом "похожести" на глагол
	// Параметры: min_score - включительный порог веса (по умолчанию 50).
	// Возвращает слова с ненулевым весом не ниже min_score.
	vector<VerbInfo> ident_verbs(uint8_t min_score = 50); 
	// Доступ для тестов:
    const vector<string>& words() { return sentence_; }

// Вспомогательные методы
private:
// Для разбора входящего предложения на слова и его нормализации (приведение к нижнему регистру)
	// Метод проверки символа на знак пунктуации
	bool is_separator(unsigned char ch);
	// Метод возвращающая структуру слов, содержащихся в переданном предложении src без знаков пунктуации
	void parse_sentence(const string& src);
	// Метод определяющий номер предложения из разметки
	void extract_sentence_num();
	// Приведение одного слова к нижнему регистру.
	// Параметры: word - изменяемая строка в UTF-8.
	// Возвращаемого значения нет; результат сохраняется в word.
	static void normalize_word(string& word);
	// Метод нормализации всех слов в структуре класса
	void normalize();
	// Метод проверки принадлежности символа к кириллице
	static bool is_cyrillic_word(const string& w);
	
// Для анализа нормализованной структуры
	

// поля класса:
	vector<string> sentence_;
	uint64_t num_ = 0;
}; // IdentVerbs