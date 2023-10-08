

cmake_minimum_required(VERSION 3.5)

project(tester)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Quick REQUIRED)
# find_package(pthread REQUIRED)

add_executable(tester main.cpp tester.cpp canbus.cpp can_thread.cpp spi_test.cpp)

target_link_libraries(tester -lm -ldl Qt5::Core Qt5::Quick)
set_property(TARGET tester PROPERTY AUTOMOC ON)

