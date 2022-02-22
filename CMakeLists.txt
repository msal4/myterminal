cmake_minimum_required(VERSION 3.20)
project(myterminal C)

set(CMAKE_C_STANDARD 99)

include_directories(./include)
link_directories(./lib)
find_package(Freetype REQUIRED)

add_executable(myterminal main.c glad.c stb_image.c pty.c pty.h shader.c stb_image.c)

target_link_libraries(myterminal Freetype::Freetype glfw3 cglm "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreFoundation -framework CoreVideo")
