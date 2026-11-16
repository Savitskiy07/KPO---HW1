cmake_minimum_required(VERSION 3.20)
project(pos_journal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8 /W4)
endif()

add_library(pos_core
    include/pos/domain/journal.hpp
    include/pos/domain/grading.hpp
    include/pos/application/journal_app.hpp
    src/domain/journal.cpp
    src/domain/grading.cpp
    src/application/journal_app.cpp
)
target_include_directories(pos_core PUBLIC include)

add_executable(pos
    include/pos/adapters/console.hpp
    include/pos/adapters/files.hpp
    include/pos/bootstrap/container.hpp
    src/main.cpp
    src/adapters/console.cpp
    src/adapters/files.cpp
    src/adapters/group_lock.cpp
)
target_link_libraries(pos PRIVATE pos_core)

include(CTest)
if(BUILD_TESTING)
    add_executable(pos_tests tests/tests.cpp)
    target_link_libraries(pos_tests PRIVATE pos_core)
    add_test(NAME journal_rules COMMAND pos_tests)

    find_package(Python3 QUIET COMPONENTS Interpreter)
    if(Python3_Interpreter_FOUND)
        add_test(NAME console_workflow
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/console.py"
                    "$<TARGET_FILE:pos>" "${CMAKE_CURRENT_SOURCE_DIR}/examples/demo.txt")
    endif()
endif()
