#include "pos/bootstrap/container.hpp"

#include <iostream>
#include <string_view>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

int main(int argc, char** argv) {
    using namespace pos;
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    if (argc > 2 || (argc == 2 && std::string_view(argv[1]) == "--help")) {
        std::cout << "Запуск: pos [папка-группы]\n"
                     "По умолчанию: pos-data. Для новой группы укажите другую папку.\n";
        return argc > 2 ? 2 : 0;
    }
    try {
        const std::filesystem::path directory = argc == 2 ? argv[1] : "pos-data";
        AppContainer app(directory, std::cin, std::cout);
        app.console().run();
    } catch (const std::exception& error) {
        std::cerr << "Не удалось открыть или сохранить группу: " << error.what() << '\n';
        return 1;
    }
}
