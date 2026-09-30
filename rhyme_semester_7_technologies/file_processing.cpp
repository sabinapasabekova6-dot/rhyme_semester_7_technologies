#include "Total_Header.h"  
using namespace std;

//запрашиваем имя входного файла с проверками:
//   1) имя не пустое;
//   2) файл существует и открывается для чтения;
//   3) файл не пустой.
string requestValidInputFile() {
    while (true) {
        cout << "Введите имя входного файла: ";
        string filename;
        getline(cin, filename);

        // Проверка 1 не пустое  ли название  файла 
        if (filename.empty()) {
            cout << "Ошибка: имя файла не может быть пустым. Попробуйте снова.\n\n";
            continue;
        }

        // Проверка 2: открывается ли файл для чтения
        ifstream file(filename.c_str(), ios::binary);
        if (!file.is_open()) {
            cout << "Ошибка: не удалось открыть файл \"" << filename
                << "\". Проверьте названиe и попробуйте снова.\n\n";
            continue;
        }

        // Проверка 3: файл не пустой.
        file.seekg(0, ios::end);
        streampos size = file.tellg();
        file.close();

        if (size <= 0) {
            cout << "Ошибка: файл \"" << filename << "\" пустой. Введите другой файл.\n\n";
            continue;
        }

        // Все проверки пройдены 
        cout << "Файл \"" << filename << "\" успешно открыт \n";
        return filename;
    }
}

// строит имя выходного файла на основе имени входного:
// "input.txt" -> "input_numbered_sentences.txt"
string buildNumberedSentencesFileName(const string& inputFileName) {
    // ищем последнюю точку в имени файла, чтобы отбросить расширение (.txt)
    int dotPosition = -1;
    for (int i = (int)inputFileName.size() - 1; i >= 0; i--) {
        if (inputFileName[i] == '.') {
            dotPosition = i;
            break;
        }
    }

    string nameWithoutExtension = (dotPosition == -1) ? inputFileName : inputFileName.substr(0, dotPosition);
    return nameWithoutExtension + "_numbered_sentences.txt";
}

// читает входной файл, разбивает его на предложения и записывает их
// пронумерованными в новый файл, каждое предложение - с новой строки
void writeNumberedSentences(const string& inputFileName) {
    ifstream inputFile(inputFileName.c_str());
    if (!inputFile.is_open()) {
        cout << "Ошибка: не удалось открыть файл для чтения предложений.\n";
        return;
    }

    // считываем содержимое файла целиком в одну строку
    string text((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    inputFile.close();

    string outputFileName = buildNumberedSentencesFileName(inputFileName);
    ofstream outputFile(outputFileName.c_str());
    if (!outputFile.is_open()) {
        cout << "Ошибка: не удалось создать файл \"" << outputFileName << "\".\n";
        return;
    }

    outputFile << "Пронумерованные предложения:\n\n";

    string currentSentence = "";
    int sentenceNumber = 0;

    // идём по тексту символ за символом и собираем предложения
    for (int i = 0; i < (int)text.size(); i++) {
        char ch = text[i];

        // пока новое предложение ещё не началось - пропускаем пробелы,
        // переводы строк и пустые строки между абзацами
        if (currentSentence.empty() && (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t')) {
            continue;
        }

        currentSentence += ch;

        // конец предложения - точка, восклицательный или вопросительный знак
        if (ch == '.' || ch == '!' || ch == '?') {
            // если это не одинокий знак препинания подряд (например, второй "." из "..")
            if (currentSentence.size() > 1) {
                sentenceNumber++;
                outputFile << sentenceNumber << ") " << currentSentence << "\n";
            }
            currentSentence = "";
        }
    }

    // если текст закончился без знака препинания в конце - тоже записываем остаток
    while (!currentSentence.empty() &&
        (currentSentence[currentSentence.size() - 1] == ' ' ||
            currentSentence[currentSentence.size() - 1] == '\n' ||
            currentSentence[currentSentence.size() - 1] == '\r' ||
            currentSentence[currentSentence.size() - 1] == '\t')) {
        currentSentence.erase(currentSentence.size() - 1);
    }
    if (!currentSentence.empty()) {
        sentenceNumber++;
        outputFile << sentenceNumber << ") " << currentSentence << "\n";
    }

    outputFile.close();

    cout << "Файл \"" << outputFileName << "\" создан. Найдено предложений: " << sentenceNumber << "\n";
}