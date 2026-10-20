#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace iverbs{
using std::string;
using std::vector;

struct VerbInfo{
	string verb;   // сам глагол
	string stem;   // корень глагола (предположительный)
	string ending; // окончание глагола
	uint8_t score; // "вес" определения того, глагол ли это (от 1 до 100)
}; 

// Структура для разбора всех слов
// struct WordInfo{
// 	string word;
// 	string ending;
// 	int score;
// } WordInfo;

class IdentVerbs {
public:
	explicit IdentVerbs(const string& src);
	~IdentVerbs() = default;

	// -------------- Основной интерфейс -------------------------------------
	// Метод анализирующий одно слово и возвращающий по нему статистику  
	static VerbInfo analysis_word(const string& word);
	// Метод анализирующий и возвращающий статистику по всем словам предложения 
	vector<VerbInfo> analysis_all_words();
	// Метод возвращающий только слова с установленным порогом "похожести" на глагол
	// по умолчанию вес более 50 (от 1 до 100)
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
	// Метод приведения одного слова к нижнему регистру
	void normalize_word(string& word);
	// Метод нормализации всех слов в структуре класса
	void normalize();
	// Метод проверки принадлежности символа к кириллице
	static bool is_cyrillic_word(const string& w);
	
// Для анализа нормализованной структуры
	

// поля класса:
	vector<string> sentence_;

}; // IdentVerbs

} // namespace iverbs