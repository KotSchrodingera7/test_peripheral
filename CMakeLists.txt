
cmake_minimum_required(VERSION 3.5)

set(CMAKE_TOOLCHAIN_FILE /home/alex/Projects/macro_group/bsp/output/ds-rk3568-evb/host/share/buildroot/toolchainfile.cmake)

project(tester)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Quick REQUIRED)
# find_package(pthread REQUIRED)

add_executable(tester main.cpp 
                        tester.cpp 
                        canbus.cpp 
                        can_thread.cpp 
                        check_cpu.cpp
                        spi_test.cpp 
                        test_uart.cpp
                        logger.cpp)

target_link_libraries(tester -lm -ldl Qt5::Core Qt5::Quick)
set_property(TARGET tester PROPERTY AUTOMOC ON)

