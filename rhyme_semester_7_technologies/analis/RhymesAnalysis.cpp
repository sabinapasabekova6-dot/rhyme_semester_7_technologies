#include "RhymesAnalysis.h"


// Вспомогательная функция для проверки, является ли символ гласным
bool is_vowel(string_view ch) {
    static const string_view vowels[] = {
        "а","е","ё","и","о","у","ы","э","ю","я"
    };
    for (auto v : vowels) 
		if(ch == v) 
			return true;
    return false;
}

// Вспомогательная функция для проверки, является ли символ согласным
bool is_consonant(string_view ch) {
    static const string_view consonants[] = {
        "б","в","г","д","ж","з","й","к","л","м",
        "н","п","р","с","т","ф","х","ц","ч","ш","щ"
    };
    for (auto v : consonants) 
		if(ch == v) 
			return true;
    return false;
}

string lower_word(string_view word) {
    string result(word);
    size_t i = 0;
    while (i < result.size()) {
        unsigned char c = static_cast<unsigned char>(result[i]);
        if (c < 128) {
            if (c >= 'A' && c <= 'Z') result[i] = static_cast<char>(c + 0x20);
            ++i;
        } else if (c == 0xD0 && i + 1 < result.size()) {
            unsigned char b = static_cast<unsigned char>(result[i + 1]);
            if (b >= 0x90 && b <= 0x9F) {          // А..П
                result[i + 1] = static_cast<char>(b + 0x20);
            } else if (b >= 0xA0 && b <= 0xAF) {   // Р..Я
                result[i]     = static_cast<char>(0xD1);
                result[i + 1] = static_cast<char>(b - 0x20);
            } else if (b == 0x81) {                // Ё
                result[i]     = static_cast<char>(0xD1);
                result[i + 1] = static_cast<char>(0x91);
            }
            i += 2;
        } else {
            ++i;
        }
    }
    return result;
}

// Функция приведения слова к фонетической записи
string to_phonetic(string_view word){
	if (word.empty()) return "";
	string lword = lower_word(word);
	// TODO: реализовать фонетическую транскрипцию
    return lower_word(word);
}