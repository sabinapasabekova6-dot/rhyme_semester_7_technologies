#include "IdentVerbs.h"
#include <iterator> // для size()

namespace iverbs {
	using std::string_view, std::move, std::string, std::size;

	// анонимный namespace для хранение словарей морфем и прочего
	namespace {

		struct EndingRule { 
			string_view ending; 
			uint8_t score;
		};
		struct SuffixRule { 
			string_view suffix; 
			uint8_t score; 
		};
		
		// ----------------------ОКОНЧАНИЯ----------------------------
		// Окончания инфинитивов
		constexpr EndingRule kEndingsInfinitive[] = {
			{"ться", 100}, {"тись", 100}, {"ать", 100},
			{"ять", 99}, {"ить", 100}, {"еть", 98},
			{"оть", 78}, {"уть", 99}, {"ыть", 94},
			{"ть", 78},	{"ти", 13}, {"чь", 90},
		};

		// Окончания возвратных форм глаголов
		constexpr EndingRule kEndingsReflexive[] = {
			{"лись", 100}, {"лась", 100}, {"лось", 100},
			{"лся", 100}, {"ешься", 100}, {"ишься", 100},
			{"етесь", 100},	{"итесь", 100}, {"ется", 100},
			{"ится", 100}, {"емся", 100}, {"имся", 100},
			{"утся", 100}, {"ются", 100}, {"атся", 100},
			{"ятся", 100}, {"усь", 99}, {"юсь", 100},
			{"йся", 98}, {"ись", 100}, {"ся", 99}, 
			{"сь", 100},
		};

		// Окончание глаголов прошедшего времени
		constexpr EndingRule kEndingsPast[] = {
			{"или", 99}, {"ила", 97}, {"ило", 99}, 
			{"ил", 96}, {"али", 99}, {"ала", 97}, 
			{"ало", 100}, {"ал", 96}, {"яли", 100},
			{"яла", 100}, {"яло", 99}, {"ял", 99},
			{"ели", 53}, {"ела", 90}, {"ело", 96},
			{"ел", 85}, {"ули", 94}, {"ула", 86},
			{"уло", 99},{"ул", 87}, {"ли", 93}, 
			{"ла", 93}, {"ло", 98}, {"л", 90}
		};

		// Окончания глаголов настоящего и прошедшего времени
		constexpr EndingRule kEndingsPresent[] = {
			{"ешь", 100}, {"ишь", 100}, {"ете", 97},
			{"ите", 88}, {"ет", 95}, {"ит", 83},
			{"ем", 72}, {"им", 77}, {"ут", 94},
			{"ют", 100}, {"ат", 41}, {"ят", 97},
			{"ю", 46}, {"у", 37} 
		};

		// Окончания повелительного наклонения глаголов
		constexpr EndingRule kEndingsImperative[] = {
			{"йте", 100}, {"ите", 88}, {"ай", 98}, 
			{"ей", 68}, {"уй", 98}, {"ой", 24}, 
			{"й", 48}, {"и", 37}, 
		};

		// -------------------СУФФИКСЫ-----------------------------
		constexpr SuffixRule kSuffixes[] = {
			{"изирова", 75}, {"изова",   70},   
			{"ирова",   70}, {"ствова",  65}, 
			{"ествова", 65}, {"нича",    55},   			
			{"ича",     50}, {"ива",     50},
			{"ыва",     50}, {"ова",     45},   
			{"ева",     45}, {"ну",      35},   
			{"ва",      25},   
		};

		// Черный список частых существительных/прилагательных, которые маскируются под глаголы
		bool is_in_blacklist(string_view s) {
			constexpr string_view blacklist[] = {
				"зал", "бал", "вал", "нал",	"мил", "бил", "лил", "нос", "род", "вид", "лад", "сон",
				"свет", "гранит", "банкет", "билет", "скелет",
				"молод", "стар", "холод", "голод", "тело", "зеркало", "дело", "мыло", "крыло", "весло", "полотно",
        		"чуть", "двадцать", "двадцати", "тридцать",
        		"бел", "зол", "стол", "пять", "шесть", "девять", "десять", "крем", "причем"
			};
			for (const string_view& w : blacklist) {
				if (s == w) return true;
			}
			return false;
		}

		bool is_participle_or_gerund(string_view word) {
			// Деепричастия совершенного и несовершенного вида
			constexpr string_view gerund_suffixes[] = {
				"вшись", "вшись", "вши", "явшись", "яв", "ав", "ив", "ыв",
				"вшись", "ясь", "авшись", "ившись"
			};
			constexpr string_view participle_markers[] = {
				"ущ", "ющ", "ащ", "ящ", "ем", "им",
				"нн", "т", "вш", "ш", "ом", "енн"
			};
			
			for (const auto& suf : gerund_suffixes) {
				if (word.size() > suf.size() && word.compare(word.size() - suf.size(), suf.size(), suf) == 0) {
					return true;
				}
			}
			for (const auto& mark : participle_markers) {
				if (word.find(mark) != string_view::npos) {
					break; 
				}
			}
			constexpr string_view part_endings[] = {
				"ущий", "ющий", "ащий", "ящий", "ущая", "ющая", "ущее", "ящее", 
				"ущие", "ющие", "ащие", "ящие", "емый", "имый", "енный", "нный", "тый"
			};
			for (const auto& end : part_endings) {
				if (word.size() >= end.size() && word.compare(word.size() - end.size(), end.size(), end) == 0) {
					return true;
				}
			}
			
			return false;
		}

		const EndingRule* try_ending_table(string_view word, const EndingRule* table,
                                    		size_t n, const EndingRule* best) {
			size_t word_s = word.size(); // длина слова в байтах
			for (size_t i = 0; i < n; ++i) {
				const EndingRule& rule = table[i];
				size_t rule_end_s = rule.ending.size(); // длина окончания из словаря в байтах

				// если длина слова <= длине окончания из словаря
				if (word_s <= rule_end_s) 
					continue;
				// если окончания не совпадают
				if (word.compare(word_s - rule_end_s, rule_end_s, rule.ending) != 0)
					continue;
				// если лучший не определен или длина найденного окончания больше или
				// если длина совпадает, но score больше
				if (!best || rule_end_s > best->ending.size() ||
					(rule_end_s == best->ending.size() && rule.score > best->score)) {
					best = &rule;
				}
			}
			return best;
		}

		// Функция поиска суффикса (аналог try_ending_table)
		const SuffixRule* try_suffix_table(string_view word, const SuffixRule* table,
										size_t n, const SuffixRule* best) {
			size_t word_s = word.size();
			for (size_t i = 0; i < n; ++i) {
				const SuffixRule& rule = table[i];
				size_t rule_suf_s = rule.suffix.size();
				if (word_s <= rule_suf_s) continue;
				
				// если суффикс не совпадает с концом слова
				if (word.compare(word_s - rule_suf_s, rule_suf_s, rule.suffix) != 0) continue;
				
				// берем самый длинный суффикс (или с большим score, если длина равна)
				if (!best || rule_suf_s > best->suffix.size() ||
					(rule_suf_s == best->suffix.size() && rule.score > best->score)) {
					best = &rule;
				}
			}
			return best;
		}

	}

	IdentVerbs::IdentVerbs(const string& src) {
		parse_sentence(src);
		normalize();
	}

	// Метод анализирующий одно слово и возвращающий по нему статистику 
	// Возвращаемые значения:
	// 		VerbInfo - информация по глаголу, где:
	// 			score = 0 - слово не глагол
	//				  = 5 - идентифицировать не вышло, вероятно не глагол
	//                = <число> - найдено некоторое совпадение
	VerbInfo IdentVerbs::analysis_word(const string& word) {
		VerbInfo info;
		info.verb = word;
		if (!is_cyrillic_word(word) || word.size() < 4) {
			info.score = 0;
			return info;
		}
		// Проверка по черному списку
		if (is_in_blacklist(word)) {
			info.score = 0;
			return info;
		}
		
		string_view sv = word; // Будем "обрезать" слово справа налево
		uint32_t total_score = 0;
		string found_ending = "";
		string found_suffix = "";

		if (is_participle_or_gerund(sv)) {
			info.score = 0;
			return info;
		}

		// Ищем самое длинное окончание таблицах
		const EndingRule* best_end = nullptr;
		best_end = try_ending_table(sv, kEndingsInfinitive, size(kEndingsInfinitive), best_end);
		best_end = try_ending_table(sv, kEndingsReflexive,  size(kEndingsReflexive),  best_end);
		best_end = try_ending_table(sv, kEndingsPast,       size(kEndingsPast),       best_end);
		best_end = try_ending_table(sv, kEndingsPresent,    size(kEndingsPresent),    best_end);
		best_end = try_ending_table(sv, kEndingsImperative, size(kEndingsImperative), best_end);
		if (!best_end) {
			info.score = 5; // не нашли окончания
			return info;
		}

		// Отрезаем найденное окончание
		sv.remove_suffix(best_end->ending.size());
		found_ending = string(best_end->ending);
		total_score += best_end->score;

		const SuffixRule* best_suf = try_suffix_table(sv, kSuffixes, size(kSuffixes), nullptr);

		if (best_suf) {
			sv.remove_suffix(best_suf->suffix.size());
			found_suffix = string(best_suf->suffix);
			total_score += best_suf->score;
		}
		
		info.stem = string(sv);
		info.ending = found_suffix + found_ending;
		info.score = static_cast<uint8_t>(std::min(total_score, 100u));

		return info;
	}

	// Метод анализирующий и возвращающий статистику по всем словам предложения 
	vector<VerbInfo> IdentVerbs::analysis_all_words() {
		vector<VerbInfo> out;
		for (const string& w : sentence_)
			out.push_back(analysis_word(w));
		return out;
	}

	// Метод возвращающий только слова с установленным порогом "похожести" на глагол
	// по умолчанию вес более 50 (от 1 до 100)
	vector<VerbInfo> IdentVerbs::ident_verbs(uint8_t min_score) {
		vector<VerbInfo> out;
		for (const auto& w : sentence_) {
			VerbInfo info = analysis_word(w);
			if (info.score >= min_score)
				out.push_back(move(info));
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

	// Метод проверки принадлежности символа к кириллице
	bool IdentVerbs::is_cyrillic_word(const string& w) {
		if (w.empty()) return false;
		bool has_cyr = false;
		size_t i = 0;
		// проходим по байтам
		while (i < w.size()) {
			unsigned char c = static_cast<unsigned char>(w[i]);
			if (c < 128) {
				// ASCII-символ внутри слова, значит не чисто кириллическое
				if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return false;
				++i;
				continue;
			}
			// кириллица: D0/D1 + второй байт
			if ((c == 0xD0 || c == 0xD1) && i + 1 < w.size()) {
				has_cyr = true;
				i += 2;
				continue;
			}
			// прочий символ пропускаем
			return false;
		}
		return has_cyr;
	}

}