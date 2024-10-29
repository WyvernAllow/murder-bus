cmake_minimum_required(VERSION 3.5.0)
project(murder-bus VERSION 0.1.0 LANGUAGES C)

set(SDL_STATIC ON)
set(SDL_SHARED OFF)
add_subdirectory(deps/SDL)

add_executable(${PROJECT_NAME}
    src/main.c
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3-static
)