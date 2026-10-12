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

	// Основной интерфейс
	// Метод анализирующий и возвращающий статистику по всем словам предложения 
	vector<VerbInfo> analize_all_words();
	VerbInfo analize_word(const string& src);
	// Метод возвращающий только слова с установленным порогом "похожести" н глагол
	// по умолчанию вес более 50 (от 1 до 100)
	vector<VerbInfo> ident_verbs(uint8_t min_score = 50); 

	// Доступ для тестов:
    const vector<string>& words() { return sentence_; }
// Вспомогательные функции
private:
// Для разбора входящего предложения на слова и его нормализации (приведение к нижнему регистру)
	// Функция проверки символа на знак пунктуации
	bool is_separator(unsigned char ch);
	// Функция возвращающая структуру слов, содержащихся в переданном предложении src без знаков пунктуации
	void parse_sentence(const string& src);
	// Функция нормализации одного слова
	void normalize_word(string& word);
	// Функция нормализации всех слов в структуре
	void normalize();
	
// Для анализа нормализованной структуры
	

// поля класса:
	vector<string> sentence_;

}; // IdentVerbs

} // namespace iverbs