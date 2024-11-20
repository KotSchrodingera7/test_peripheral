
cmake_minimum_required(VERSION 3.5)

project(expo_view)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_BUILD_TYPE Debug)

find_package(PkgConfig REQUIRED)

# pkg_search_module(GST REQUIRED gstreamer-1.0>=1.4
#     gstreamer-sdp-1.0>=1.4
#     gstreamer-video-1.0>=1.4
#     gstreamer-app-1.0>=1.4)
# pkg_search_module(GLIB REQUIRED glib-2.0) 
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GST_VIDEO REQUIRED gstreamer-video-1.0)
# pkg_check_modules(GSTREAMER gstreamer-1.0>=1.6 gstreamer-video-1.0>=1.6 REQUIRED)

find_package(Qt5 COMPONENTS Core Quick Qml Widgets REQUIRED)

link_directories(${GSTREAMER_LIBRARY_DIRS})
include_directories (${GSTREAMER_INCLUDE_DIRS})

# find_package(pthread REQUIRED)

# execute_process(
#   COMMAND git rev-parse --abbrev-ref HEAD
#   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#   OUTPUT_VARIABLE GIT_BRANCH
#   OUTPUT_STRIP_TRAILING_WHITESPACE
# )

set(GIT_HASH "unknown")
 
# find Git and if available set GIT_HASH variable
find_package(Git QUIET)
if(GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%h
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    )
endif()
 
message(STATUS "Git hash is ${GIT_HASH}")

set(NAME_PROJECT "expo_view")
add_executable(${NAME_PROJECT} main.cpp 
                        tester.cpp 
                        canbus.cpp
                        camera_gst.cpp 
                        can_thread.cpp 
                        check_cpu.cpp
                        spi_test.cpp 
                        test_uart.cpp
                        logger.cpp)

target_link_libraries(${NAME_PROJECT} -lm -ldl Qt5::Core Qt5::Quick Qt5::Qml Qt5::Widgets ${GST_APP_LIBRARIES} ${GST_VIDEO_LIBRARIES})
set_property(TARGET ${NAME_PROJECT} PROPERTY AUTOMOC ON)

