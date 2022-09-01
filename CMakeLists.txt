cmake_minimum_required(VERSION 3.10)

project(Snake)

add_executable(snake main.c)

target_link_libraries(snake PUBLIC ncurses)
