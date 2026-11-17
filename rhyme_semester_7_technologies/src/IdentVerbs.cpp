#include "IdentVerbs.h"
#include "MorphRules.h"
#include <cctype>
#include <algorithm>
#include <utility>
#include <string_view>

using std::string_view, std::move, std::string;

// Универсальный поиск самого длинного (а при равной длине - самого
// весомого) правила, которым заканчивается слово
// Возвращает указатель на лучшее правило либо best (если ничего не нашли).
// Параметры:
//     word  - слово, в котором ищем окончание/суффикс;
//     table - таблица правил (окончания или суффиксы);
//     best  - текущий лучший результат (для последовательного вызова
//             по нескольким таблицам; при первом вызове - nullptr).
const MorphemeRule* try_rule_table(string_view word,
								const vector<MorphemeRule>& table,
								const MorphemeRule* best) {
	size_t word_s = word.size();
	for (const MorphemeRule& rule : table) {
		size_t rule_s = rule.text.size();
		if (word_s <= rule_s) continue; // слово не длиннее правила
		if (word.compare(word_s - rule_s, rule_s, rule.text) != 0) continue; // не совпало
		if (!best
			|| rule_s > best->text.size()
			|| (rule_s == best->text.size() && rule.score > best->score)) {
			best = &rule;
		}
	}
	return best;
}

// Проверка точного совпадения слова с элементом списка.
// Параметры: word - проверяемое слово; words - список слов.
// Возвращает true при совпадении, false - если слова в списке нет.
bool is_in_word_list(string_view word, const vector<string>& words) {
	for (const string& w : words)
		if (word == w) return true;
	return false;
}

// Проверяет, оканчивается ли word на suf
// Возвращает true, если word длиннее suf и последние suf.size()
// байт word совпадают с suf
bool ends_with(string_view word, string_view suf) {
	if (word.size() <= suf.size()) return false; // слово не длиннее суффикса
	return word.compare(word.size() - suf.size(), suf.size(), suf) == 0;
}

// Именной суффикс ищем перед падежной флексией или на конце слова.
bool has_nominal_suffix(string_view word, const vector<string>& suffixes) {
    const string_view flexions[] = {
        "", "а", "я", "у", "ю", "е", "и", "ы", "о",
        "ом", "ем", "ам", "ям", "ах", "ях", "ов", "ев", "ей", "ами", "ями"
    };
    for (string_view flexion : flexions) {
        if (!flexion.empty() && !ends_with(word, flexion)) continue;
        string_view stem = word;
        stem.remove_suffix(flexion.size());
        for (const string& suffix : suffixes) {
            if (ends_with(stem, suffix)) return true;
        }
    }
    return false;
}

struct VerbForm {
    string_view ending;
    const MorphemeRule* weight = nullptr;
    bool takes_suffix = false;
};

// Частотный хвост может захватывать часть основы (например, «кали»).
// Границу формы определяем отдельно по «ли».
void match_verb_form(string_view word, const vector<string>& forms,
                     const vector<MorphemeRule>& weights,
                     bool takes_suffix, VerbForm& best) {
    string_view ending;
    for (const string& form : forms) {
        if (form.size() > ending.size() && ends_with(word, form)) ending = form;
    }
    if (ending.empty()) return;

    const MorphemeRule* weight = try_rule_table(word, weights, nullptr);
    if (!weight || weight->text.size() < ending.size()) return;
    if (best.weight && weight->text.size() < best.weight->text.size()) return;
    if (best.weight && weight->text.size() == best.weight->text.size()
        && weight->score <= best.weight->score) return;

    best.ending = ending;
    best.weight = weight;
    best.takes_suffix = takes_suffix;
}

// Проверка "слово - деепричастие/причастие"
bool is_participle(string_view word, const vector<string>& gerunds, const vector<string>& participles) {
	for (const string& s : gerunds) 
		if (ends_with(word, s)) return true;
	for (const string& s : participles) 
		if (ends_with(word, s)) return true;
	return false;
}


IdentVerbs::IdentVerbs(const string& src) {
	parse_sentence(src);
	normalize();

}

// Метод анализирующий одно слово и возвращающий по нему статистику.
// Параметры: word - одно слово в UTF-8, регистр может быть любым.
// Исходная строка не изменяется; info.verb содержит слово в нижнем регистре.
// Возвращаемые значения:
// 		VerbInfo - информация по глаголу, где:
// 			score = 0 - слово не глагол
//				  = 5 - идентифицировать не вышло, вероятно не глагол
//                = <число> - найдено некоторое совпадение
VerbInfo IdentVerbs::analysis_word(const string& word) {
    // Нормализация нужна и при прямом вызове, без создания IdentVerbs.
    string normalized_word = word;
    normalize_word(normalized_word);
    // Один раз за вызов получаем ссылку на общий словарь.
    const MorphRules& rules = MorphRules::instance();

    VerbInfo info{};
    info.verb = normalized_word;

    // После проверки русских букв четыре байта соответствуют двум буквам.
    if (!is_cyrillic_word(normalized_word) || normalized_word.size() < 4) {
        info.score = 0;
        return info;
    }

    if (is_in_word_list(normalized_word, rules.blacklist)) return info;

    string_view sv = normalized_word;
    // Частые формы со слабым окончанием или нестандартной основой.
    // Омонимы «пила», «стали», «печь» требуют контекста.
    const string_view known_verbs[] = {
        "идти", "везти", "могу", "может", "хочу", "хочет", "иду", "еду",
        "буду", "будет", "будут", "был", "была", "было", "были",
        "живу", "жду", "знаю", "вижу", "слышу", "помню", "сплю", "любит",
        "имел", "имела", "имело", "имели",
        "шёл", "шел", "шла", "шло", "шли", "бил", "лил",
        "дал", "дало", "несла", "нёсся", "несся", "ушёл", "ушел"
    };
    bool known_verb = false;
    for (string_view known : known_verbs) {
        if (sv == known) {
            known_verb = true;
            break;
        }
    }

    string_view postfix;
    if (ends_with(sv, "ся") || ends_with(sv, "сь")) {
        postfix = sv.substr(sv.size() - 4);
        sv.remove_suffix(postfix.size());
    }

    if (!known_verb && is_participle(sv, rules.gerundSuffixes, rules.participleSuffixes)) {
        return info;
    }

    VerbForm form;
    match_verb_form(sv, rules.formsInfinitive, rules.endingsInfinitive, true, form);
    match_verb_form(sv, rules.formsPast, rules.endingsPast, true, form);
    match_verb_form(sv, rules.formsPresent, rules.endingsPresent, false, form);
    match_verb_form(sv, rules.formsImperative, rules.endingsImperative, false, form);

    if (!form.weight) {
        info.score = 5;
        if (known_verb) {
            info.stem = sv;
            info.ending = postfix;
            info.score = 100;
        }
        return info;
    }

    uint32_t total_score = form.weight->score;
    if (!postfix.empty()) {
        // «ся» присоединяется после согласной, «сь» — после гласной.
        // Подтверждение из таблицы возвратных форм учитываем только
        // при наличии подходящей формы перед постфиксом.
        const string_view vowels[] = {"а", "е", "ё", "и", "о", "у", "ы", "э", "ю", "я"};
        string_view last = sv.substr(sv.size() - 2);
        bool after_vowel = false;
        for (string_view vowel : vowels) {
            if (last == vowel) after_vowel = true;
        }
        if (after_vowel == (postfix == "сь")) {
            const MorphemeRule* reflexive = try_rule_table(normalized_word, rules.endingsReflexive, nullptr);
            if (reflexive && reflexive->text.size() > postfix.size()) {
                total_score = std::max<uint32_t>(total_score, reflexive->score);
            }
        } else {
            total_score = 5;
        }
    } else if (!known_verb && has_nominal_suffix(sv, rules.suffixesNominal)) {
        return info;
    }

    string_view stem = sv;
    stem.remove_suffix(form.ending.size());
    if (form.takes_suffix) {
        const MorphemeRule* suffix = try_rule_table(stem, rules.suffixesVerbal, nullptr);
        if (suffix) {
            stem.remove_suffix(suffix->text.size());
            total_score += suffix->score;
        } else if (form.ending == "ть" && postfix.empty() && stem.size() > 2) {
            // В продуктивных инфинитивах сохраняем тематическую гласную
            // в хвосте. Она уже учтена в частотном весе.
            string_view vowel = stem.substr(stem.size() - 2);
            if (vowel == "а" || vowel == "я" || vowel == "е" || vowel == "и") {
                stem.remove_suffix(vowel.size());
            }
        }
    }

    info.stem = stem;
    info.ending = normalized_word.substr(stem.size());
    if (known_verb) total_score = 100;
    info.score = static_cast<uint8_t>(std::min<uint32_t>(total_score, 100u));
    return info;
}

// Метод анализирующий и возвращающий статистику по всем словам предложения 
vector<VerbInfo> IdentVerbs::analysis_all_words() {
	vector<VerbInfo> out;
	for (const string& w : sentence_){
		VerbInfo info = analysis_word(w);
        info.num = num_;
        out.push_back(std::move(info));
	}
	return out;
}

// Метод возвращающий только слова с установленным порогом "похожести" на глагол
// по умолчанию вес более 50 (от 1 до 100)
vector<VerbInfo> IdentVerbs::ident_verbs(uint8_t min_score) {
	vector<VerbInfo> out;
	for (const auto& w : sentence_) {
		VerbInfo info = analysis_word(w);
		// Нулевой вес означает отказ: такой результат не включаем даже при пороге 0.
		if (info.score > 0 && info.score >= min_score){
			info.num = num_;
			out.push_back(move(info));
		}
	}
	return out;
}	

// Метод проверки, является ли переданный символ (ch), знаком
// пунктуации или специальным символом
bool IdentVerbs::is_separator(unsigned char ch) {
	// все символы лежат в диапазоне значений <128, поэтому все что больше нас не интересует
	if(ch >= 128) return false;
	if (isspace(ch)) return true;
	string_view symbols = ".,;:!?\"()[]{}<>/\\|*&^%#@$~";
	// приведение к char допустимо, интересующие буквы с >=128 а символы < 128
	bool flag = (symbols.find(static_cast<char>(ch)) != string_view::npos);
	// npos максимально возможное значение для size_t 
	// как некое спец значение для идентификации значения "не найдено" 
	return flag;
}

// Метод возвращающая структуру слов, содержащихся в переданном предложении src без знаков пунктуации
void IdentVerbs::parse_sentence(const string& src){
	string current; // промежуточная строка для "накопления" букв слова
	for (size_t i = 0; i < src.size(); ++i){
		unsigned char c = static_cast<unsigned char>(src[i]);
		if (is_separator(c)){
			if (!current.empty()){
				sentence_.push_back(current); // кладем слово в массив
				current.clear(); // очищаем для нового слова
			}
			continue;
		}
		if (c == 0xC2 || c == 0xE2) {
			if (!current.empty()) {
				sentence_.push_back(current);
				current.clear();
			}
			i = i + 1;
			while (i < src.size() && (static_cast<unsigned char>(src[i]) & 0xC0) == 0x80) {
				i = i + 1;
			}
			i = i - 1;
			continue;
		}
		if (c == '-' || c == '\'' || c == '`'){
			bool has_prev = !current.empty();
			bool has_next = (i + 1 < src.size()) && !is_separator(static_cast<unsigned char>(src[i + 1]));
			// если предыдущая часть слова есть и следующая часть не символ пунктуации 
			if (has_prev && has_next) {
				current += static_cast<char>(c);
				continue;
			} else {
				if (!current.empty()) {
					sentence_.push_back(current);
					current.clear();
				}
				continue;
			}
		}
		current += static_cast<char>(c);
	}
	if (!current.empty()){
		sentence_.push_back(current);
	}
}

void IdentVerbs::extract_sentence_num() {
    num_ = 0; // безопасное значение по умолчанию
    if (sentence_.empty()) return;

    const string& first = sentence_[0];
    if (first.empty()) return;

    uint64_t val = 0;
    for (char ch : first) {
        if (ch < '0' || ch > '9') return; // не число, значит оставляем 0, слово не трогаем
		val = val * 10 + static_cast<uint64_t>(ch - '0');
    }
    num_ = val;
    sentence_.erase(sentence_.begin());
}

// Метод приведения одного слова к нижнему регистру
void IdentVerbs::normalize_word(string& word) {
	// ASCII - сдвиг; кириллица в UTF-8 идёт двумя байтами - сдвиг второго байта
	size_t i = 0;
	while (i < word.size()) {
		unsigned char c = static_cast<unsigned char>(word[i]);
		if (c < 128) {
			if (c >= 'A' && c <= 'Z') word[i] = static_cast<char>(c + 0x20);
			++i;
		} else if (c == 0xD0 && i + 1 < word.size()) {
			// U+0410...U+042F -> D0 90...D0 AF (А...Я)
			// U+0430...U+044F -> D0 B0...D1 8F (а...я)
			// U+0401 (Ё) D0 81 -> U+0451 (ё) D1 91
			unsigned char b = static_cast<unsigned char>(word[i + 1]);
			if (b >= 0x90 && b <= 0x9F) {  // А..П
				word[i + 1] = static_cast<char>(b + 0x20);
			} else if (b >= 0xA0 && b <= 0xAF) {    // Р..Я
				word[i]     = static_cast<char>(0xD1);
				word[i + 1] = static_cast<char>(b - 0x20);
			} else if (b == 0x81) {   // Ё
				word[i]     = static_cast<char>(0xD1);
				word[i + 1] = static_cast<char>(0x91);
			}
			i += 2;
		} else {
			++i; // прочие многобайтовые символы - не трогаем
		}
	}
}

// Метод нормализации всех слов в структуре класса
void IdentVerbs::normalize() {
	for (string& w : sentence_) normalize_word(w);
}

// Проверка, что слово состоит только из русских букв в UTF-8.
// Параметры: w - проверяемая строка, допускается любой регистр.
// Возвращает true для непустого русского слова, false - иначе.
// Цифры, дефисы, латиница и повреждённые последовательности отвергаются.
bool IdentVerbs::is_cyrillic_word(const string& w) {
    if (w.empty()) return false;
    size_t i = 0;
    while (i < w.size()) {
        // Каждая русская буква занимает два байта; одиночного байта недостаточно.
        if (i + 1 >= w.size()) return false;
        unsigned char first = static_cast<unsigned char>(w[i]);
        unsigned char second = static_cast<unsigned char>(w[i + 1]);

        // D0 90..BF: А..Я и а..п; D1 80..8F: р..я.
        // Ё и ё расположены отдельно от основных диапазонов алфавита.
        bool first_range = first == 0xD0 && second >= 0x90 && second <= 0xBF;
        bool second_range = first == 0xD1 && second >= 0x80 && second <= 0x8F;
        bool letter_yo = (first == 0xD0 && second == 0x81)
                      || (first == 0xD1 && second == 0x91);
        if (!first_range && !second_range && !letter_yo) return false;
        i += 2;
    }
    return true;
}
