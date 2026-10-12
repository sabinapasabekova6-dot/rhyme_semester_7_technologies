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

	// Доступ для тестов:
    const vector<string>& words() const { return sentence_; }
// Вспомогательные функции
private:
// Для разбора предложения на слова и его нормализации (приведение к нижнему регистру)
	// Функция проверки символа на знак пунктуации
	bool is_separator(unsigned char ch);
	// Функция возвращающая структуру слов, содержащихся в переданном предложении src без знаков пунктуации
	void parse_sentence(const string& src);
	// Функция нормализации одного слова
	void normalize_word(string& word);
	// Функция нормализации всех слов в структуре
	void normalize();

// поля класса:
	vector<string> sentence_;

}; // IdentVerbs

} // namespace iverbs