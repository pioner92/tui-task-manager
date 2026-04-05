cmake_minimum_required(VERSION 4.1)
project(task_manager)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)


include(FetchContent)

FetchContent_Declare(
        ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
        GIT_TAG main
)

FetchContent_MakeAvailable(ftxui)


add_library(sqlite3 STATIC
        app/third_party/sqlite/sqlite3.c
        app/core/storage/storage.cpp
        app/ui/timesheet_list/list_item.h
        app/ui/utils.h
        app/ui/task_info_block/description.h
)


target_include_directories(sqlite3 PUBLIC
        app/third_party/sqlite
)

add_executable(task_manager main.cpp)

target_include_directories(task_manager PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/app
)

target_link_libraries(task_manager PRIVATE
        ftxui::screen
        ftxui::dom
        ftxui::component
        sqlite3
)
