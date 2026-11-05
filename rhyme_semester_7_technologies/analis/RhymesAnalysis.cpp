#include "RhymesAnalysis.h"


// Длина в байтах последнего UTF-8 символа в s, либо 0 если s пуста.
// Continuation-байты имеют вид 10xxxxxx (0x80..0xBF).
size_t last_char_bytes(string_view s) {
	if (s.empty()) return 0;
	size_t i = s.size() - 1;
	while (i > 0 && (static_cast<unsigned char>(s[i]) & 0xC0) == 0x80)
		--i;
	return s.size() - i;
}

// Количество UTF-8 символов (start-байты - всё, что НЕ 10xxxxxx).
size_t count_chars(string_view s) {
	size_t n = 0;
	for (unsigned char c : s)
		if ((c & 0xC0) != 0x80) ++n;
	return n;
}

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
string to_phonetic(string_view word) {
    if (word.empty()) return "";

    string lword = lower_word(word);
    string result;
    result.reserve(lword.size() + 4);
    size_t i = 0;
    while (i < lword.size()) {
        unsigned char c = static_cast<unsigned char>(lword[i]);
        // ASCII (латиница, цифры, символы) - без изменений
        if (c < 128) {
            result += static_cast<char>(c);
            ++i;
            continue;
        }
        // остальное копируем как есть
        if (i + 1 >= lword.size()) {
            result += lword[i];
            ++i;
            continue;
        }

        string_view ch(lword.data() + i, 2);

        if (ch == "я") result += "а";
        else if (ch == "ю") result += "у";
        else if (ch == "ё") result += "о";
        else if (ch == "е") result += "э";
        else if (ch == "ь" || ch == "ъ") {/* знаки не имеют звука */}
        else result.append(ch);
        i += 2;
    }

    // Оглушение звонкого согласного в конце слова
    if (result.size() >= 2) {
        size_t n = result.size();
        unsigned char b0 = static_cast<unsigned char>(result[n - 2]);
        unsigned char b1 = static_cast<unsigned char>(result[n - 1]);
        // Проверяем, что последний символ - корректный 2-байтный кириллический
        if ((b0 == 0xD0 || b0 == 0xD1) && b1 >= 0x80 && b1 <= 0xBF) {
            string_view last(result.data() + n - 2, 2);
            string_view repl;
            if (last == "б") repl = "п";
            else if (last == "в") repl = "ф";
            else if (last == "г") repl = "к";
            else if (last == "д") repl = "т";
            else if (last == "ж") repl = "ш";
            else if (last == "з") repl = "с";

            if (!repl.empty()) {
                result[n - 2] = repl[0];
                result[n - 1] = repl[1];
            }
        }
    }

    return result;
}


// Длина общего суффикса (с конца) в символах.
// Сравнение посимвольное (UTF-8-aware), счёт — в символах, не в байтах.
size_t common_suffix_len(string_view a, string_view b) {
    size_t count = 0;
    while (!a.empty() && !b.empty()) {
        size_t la = last_char_bytes(a);
        size_t lb = last_char_bytes(b);
        if (la != lb) break;                       // разная ширина → символы разные
        if (a.substr(a.size() - la) != b.substr(b.size() - lb)) break;
        a.remove_suffix(la);
        b.remove_suffix(lb);
        ++count;
    }
    return count;
}

// Коэффициент схожести 0...1.
// Значение = доля более короткой строки, покрытая общим суффиксом.
//   common == min_len  ->  1.0 (одна строка — суффикс другой)
//   common == 0        ->  0.0
double similarity(string_view phonetic_a, string_view phonetic_b) {
    if (phonetic_a.empty() || phonetic_b.empty()) return 0.0;

    size_t ca = count_chars(phonetic_a);
    size_t cb = count_chars(phonetic_b);
    size_t min_c = min(ca, cb);
    if (min_c == 0) return 0.0;

    size_t common = common_suffix_len(phonetic_a, phonetic_b);
	double res = static_cast<double>(common) / static_cast<double>(min_c);
    return res;
}