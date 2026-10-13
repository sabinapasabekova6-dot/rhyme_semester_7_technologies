#include "IdentVerbs.h"

namespace iverbs {
	using std::string_view;

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
			{"ться", 95}, {"тись", 90}, {"ать", 85}, {"ять", 85}, 
			{"ить",  85}, {"еть",  80}, {"оть", 75}, {"уть", 75},
			{"ыть",  75}, {"ть",   80}, {"ти",  85}, {"чь",  60}
		};

		// Окончания возвратных форм глаголов
		constexpr EndingRule kEndingsReflexive[] = {
			{"лись",  85}, {"лась",  85}, {"лось",  80}, {"лся", 85},
			{"ешься", 85}, {"ишься", 85}, {"етесь", 80}, {"итесь", 80},
			{"ется",  80}, {"ится",  80}, {"емся",  75}, {"имся", 75},
			{"утся",  75}, {"ются",  75}, {"атся",  75}, {"ятся", 75},
			{"усь",   60}, {"юсь",   60}, {"йся",   65}, {"ись", 65},
			{"ся",    40}, {"сь",    30}
		};

		// Окончание глаголов прошедшего времени (без -ся)
		constexpr EndingRule kEndingsPast[] = {
			{"или", 60}, {"ила", 60}, {"ило", 55}, {"ил", 55},
			{"али", 55}, {"ала", 55}, {"ало", 50}, {"ал", 50},
			{"яли", 55}, {"яла", 55}, {"яло", 50}, {"ял", 50},
			{"ели", 55}, {"ела", 55}, {"ело", 50}, {"ел", 50},
			{"ули", 50}, {"ула", 50}, {"уло", 45}, {"ул", 45},
			{"ли", 40}, {"ла", 35}, {"ло", 30}, {"л", 25}
		};

		// Окончания глаголов настоящего и прошедшего времени
		constexpr EndingRule kEndingsPresent[] = {
			{"ешь", 70}, {"ишь", 70}, {"ете", 60}, {"ите", 60},
			{"ет",  45}, {"ит",  45}, {"ем",  45}, {"им",  45},
			{"ут",  40}, {"ют",  40}, {"ат", 35}, {"ят", 40},
			{"ю",   20}, {"у",   15} 
		};

		// Окончания повелительного наклонения глаголов
		constexpr EndingRule kEndingsImperative[] = {
			{"йте", 40}, {"ите", 40}, {"ай", 30}, {"ей", 25}, 
			{"уй",  25}, {"ой",  20}, {"й",  15}, {"и",  10}
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
	}

	IdentVerbs::IdentVerbs(const string& src){
		parse_sentence(src);
		normalize();
	}

	vector<VerbInfo> IdentVerbs::analize_all_words(){
		// В цикле вызов проверки каждого слова в структуре
	}

	VerbInfo IdentVerbs::analize_word(const string& src){

	}

	// Функция проверки, является ли переданный символ (ch), знаком
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

	// Приведение одного слова к нижнему регистру.
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
				if (b >= 0x90 && b <= 0x9F) {           // А..П
					word[i + 1] = static_cast<char>(b + 0x20);
				} else if (b >= 0xA0 && b <= 0xAF) {    // Р..Я
					word[i]     = static_cast<char>(0xD1);
					word[i + 1] = static_cast<char>(b - 0x20);
				} else if (b == 0x81) {                 // Ё
					word[i]     = static_cast<char>(0xD1);
					word[i + 1] = static_cast<char>(0x91);
				}
				i += 2;
			} else {
				++i; // прочие многобайтовые символы - не трогаем
			}
		}
	}

	void IdentVerbs::normalize() {
		for (string& w : sentence_) normalize_word(w);
	}

}