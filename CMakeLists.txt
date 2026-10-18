cmake_minimum_required(VERSION 3.20)
project(HW1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8 /Zc:__cplusplus)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

add_library(HW1_domain
        src/domain/Vehicle.cpp
        src/domain/Item.cpp
        src/domain/ElectricScooter.cpp
        src/domain/ChargingStation.cpp
        src/services/FleetService.cpp
)
target_include_directories(HW1_domain PUBLIC src)

add_library(HW1_composition
        src/composition/AppContainer.cpp
)
target_link_libraries(HW1_composition PUBLIC HW1_domain)

add_library(HW1_console
        src/console/ConsoleApp.cpp
)
target_link_libraries(HW1_console PUBLIC HW1_domain)

add_executable(HW1 src/main.cpp)
target_link_libraries(HW1 PRIVATE HW1_composition HW1_console)

include(CTest)
if(BUILD_TESTING)
    add_executable(HW1_tests
            tests/setup_test.cpp
            tests/vehicle_test.cpp
            tests/service_center_test.cpp
            tests/fleet_service_test.cpp
            tests/items_test.cpp
            tests/energy_test.cpp
            tests/lists_test.cpp
            tests/app_container_test.cpp
            tests/console_app_test.cpp
    )
    target_include_directories(HW1_tests SYSTEM PRIVATE third_party/doctest)
    target_link_libraries(HW1_tests PRIVATE HW1_composition HW1_console)
    add_test(NAME setup_test COMMAND HW1_tests)
endif()