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
    if (phonetic_a.empty() || phonetic_b.empty()) {
        return 0.0;
    }

    size_t ca = count_chars(phonetic_a);
    size_t cb = count_chars(phonetic_b);
    size_t min_c = ca;
    if (cb < min_c) {
        min_c = cb;
    }
    if (min_c == 0) {
        return 0.0;
    }
    size_t common = common_suffix_len(phonetic_a, phonetic_b);
    double res = static_cast<double>(common) / static_cast<double>(min_c);
    return res;
}


// Проверяет, есть ли уже такая пара в результате (для дедупликации).
// Ключ нормализуется: меньший номер и меньшее слово идут первыми.
bool is_duplicate(const vector<VerbStruct>& result,
					uint64_t kn1, uint64_t kn2,
					const string& kw1, const string& kw2){
	for (size_t k = 0; k < result.size(); ++k) {
		uint64_t rn1 = result[k].num_verb1;
		uint64_t rn2 = result[k].num_verb2;
		if (rn1 > rn2) {
			uint64_t t = rn1;
			rn1 = rn2;
			rn2 = t;
		}
		string rw1 = result[k].verb1;
		string rw2 = result[k].verb2;
		if (rw1 > rw2) {
			string t = rw1;
			rw1 = rw2;
			rw2 = t;
		}
		if (rn1 == kn1 && rn2 == kn2 && rw1 == kw1 && rw2 == kw2) {
			return true;
		}
	}
	return false;
}

uint64_t extract_sentence_num(string_view& body) {
	size_t i = 0;
	while (i < body.size()) {
		unsigned char c = static_cast<unsigned char>(body[i]);
		if (!isspace(c)) {
			break;
		}
		++i;
	}

	size_t start = i;
	while (i < body.size()) {
		if (body[i] < '0' || body[i] > '9') {
			break;
		}
		++i;
	}

	if (i == start) {
		return 0;
	}

	// Если после цифр идёт буква, это не номер
	if (i < body.size()) {
		unsigned char c = static_cast<unsigned char>(body[i]);
		if (isalpha(c)) {
			return 0;
		}
	}

	uint64_t val = 0;
	for (size_t k = start; k < i; ++k) {
		val = val * 10 + static_cast<uint64_t>(body[k] - '0');
	}

	// Пропускаем пунктуацию сразу после номера
	while (i < body.size()) {
		char c = body[i];
		if (c == '.' || c == ')' || c == ':' || c == ',' || c == '-') {
			++i;
		} else {
			break;
		}
	}

	body = body.substr(i);
	return val;
}

double rhyme_similarity(const VerbInfo& w1, const VerbInfo& w2) {
	if (w1.verb.empty() || w2.verb.empty()) {
		return 0.0;
	}
	string p1 = to_phonetic(w1.verb);
	string p2 = to_phonetic(w2.verb);
	double res = similarity(p1, p2);
	return res;
}

bool is_rhyme(const VerbInfo& w1, const VerbInfo& w2,
				double min_similarity)
{
	double sim = rhyme_similarity(w1, w2);
	if (sim >= min_similarity) {
		return true;
	}
	return false;
}


// Ищет пары рифмующихся глаголов в списке предложений.
vector<VerbStruct> find_rhymes(
    const vector<string>& sentences,
    uint8_t min_verb_score,
    double min_similarity)
{
    vector<VerbStruct> result;
    if (sentences.empty()) {
        return result;
    }

    // Собираем все глаголы из всех предложений
    vector<VerbInfo> all_verbs;
    for (size_t i = 0; i < sentences.size(); ++i) {
        string_view body = sentences[i];
        uint64_t num = extract_sentence_num(body);
        string body_str(body);
        if (body_str.empty()) {
            continue;
        }

        IdentVerbs iv(body_str);
        vector<VerbInfo> verbs = iv.ident_verbs(min_verb_score);
        for (size_t k = 0; k < verbs.size(); ++k) {
            verbs[k].num = num;
            all_verbs.push_back(verbs[k]);
        }
    }

    if (all_verbs.size() < 2) {
        return result;
    }

    // Кэшируем фонетические формы, чтобы не считать их повторно
    vector<string> phon(all_verbs.size());
    for (size_t i = 0; i < all_verbs.size(); ++i) {
        phon[i] = to_phonetic(all_verbs[i].verb);
    }

    // Перебираем пары (i < j) и добавляем без дублей
    for (size_t i = 0; i < all_verbs.size(); ++i) {
        for (size_t j = i + 1; j < all_verbs.size(); ++j) {
            double sim = similarity(phon[i], phon[j]);
            if (sim < min_similarity) {
                continue;
            }

            uint64_t n1 = all_verbs[i].num;
            uint64_t n2 = all_verbs[j].num;
            string w1 = all_verbs[i].verb;
            string w2 = all_verbs[j].verb;

            // Нормализованный ключ для проверки дубля
            uint64_t kn1 = n1;
            uint64_t kn2 = n2;
            if (kn1 > kn2) {
                uint64_t t = kn1;
                kn1 = kn2;
                kn2 = t;
            }
            string kw1 = w1;
            string kw2 = w2;
            if (kw1 > kw2) {
                string t = kw1;
                kw1 = kw2;
                kw2 = t;
            }

            bool dup = is_duplicate(result, kn1, kn2, kw1, kw2);
            if (dup) {
                continue;
            }

            VerbStruct vs;
            vs.verb1 = w1;
            vs.verb2 = w2;
            vs.num_verb1 = n1;
            vs.num_verb2 = n2;
            result.push_back(vs);
        }
    }
    return result;
}