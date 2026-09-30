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

