cmake_minimum_required(VERSION 3.10)
project(maze)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Найти SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Добавить исходные файлы
add_executable(maze
    main.cpp
)

# Подключить заголовочные файлы
target_include_directories(maze PRIVATE src)

# Подключить библиотеки SFML
target_link_libraries(maze PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)
