#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
using namespace std;

// Возможные способы анализа слов на часть речи:
// 1. Анализ морфем и анализ контекста руками
// 2. XMorphy (построена на словарях OpenCorpora)


// Функция проверки, является ли переданный символ (ch), знаком
// пунктуации или специальным символом
bool is_separator(unsigned char ch) {
	// все символы лежат в диапазоне значений <128, поэтому все что больше нас не интересует
	if(ch >= 128) return false;
    if (isspace(ch)) return true;
    constexpr string_view symbols = ".,;:!?\"'()[]{}<>/\\|*&^%#@$~`";
	// приведение к char допустимо, интересующие буквы с >=128 а символы < 128
	bool flag = (symbols.find(static_cast<char>(ch)) != string_view::npos);
	return flag;
	// npos максимально возможное значение для size_t 
	// как некое спец значение для идентификации значения "не найдено" 
}

// Функция возвращающая структуру слов, содержащихся в переданном
// предложении src без знаков пунктуации
vector<string>  parse_sentence(const string& src){
	vector<string> words; // вектор слов из предложения без знаков пунктуации
    string current; // промежуточная строка для "накопления" букв слова
	for (size_t i = 0; i < src.size(); ++i){
        unsigned char c = static_cast<unsigned char>(src[i]);

        if (is_separator(c)){
            if (!current.empty()){
                words.push_back(current); // кладем слово в массив
                current.clear(); // очищаем для нового слова
            }
            continue;
        }	

        if (c == '-' || c == '\''){
            bool has_prev = !current.empty();
            bool has_next = (i + 1 < src.size()) && !is_separator(static_cast<unsigned char>(src[i + 1]));
			// если предыдущая часть слова есть и следующая часть не символ пунктуации 
            if (has_prev && has_next) {
                current += static_cast<char>(c);
                continue;
            } else {
                if (!current.empty()) {
                    words.push_back(current);
                    current.clear();
                }
                continue;
            }
        }
        current += static_cast<char>(c);
    }

    if (!current.empty()){
        words.push_back(current);
    }
	return words;
}



vector<string> tests = {
        "Жил да был черный кот за углом.",
        "И кота ненавидел: весь дом (кто-то)!",
        "Hello, world!?",
        "Стекло стекло на пол.", 
        "Печь пироги - это печь нужна.",
        "  ...тут многоточие...  и   пробелы  ",
        "Д'Артаньян и кто-то ещё",
        "" 
    };

int main() {

	for (size_t i = 0; i < tests.size(); ++i) {
        cout << "> Тест " << i << ": [" << tests[i] << "]\n";
        vector<string> words = parse_sentence(tests[i]);
        cout << "Слов: " << words.size() << "\n";
        for (const auto& w : words) {
            cout << "- " << w << "\n";
        }
        cout << "\n";
    }

    return 0;
}
