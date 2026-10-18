#include "composition/AppContainer.h"
#include "console/ConsoleApp.h"

#include <iostream>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    AppContainer container;
    ConsoleApp app(container.getFleetService(), std::cin, std::cout);
    app.run();
    return 0;
}