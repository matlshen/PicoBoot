project(bootutil)

# Compile library
file(GLOB SOURCES "src/*.c")
add_library(bootlib STATIC ${SOURCES})
target_include_directories(bootlib PUBLIC inc)