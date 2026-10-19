cmake_minimum_required(VERSION 4.2)
project(Guardians_of_the_Sun_Temple LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(Guardians_of_the_Sun_Temple main.cpp)


target_link_libraries(Guardians_of_the_Sun_Temple PRIVATE
        OpenGL::GL
        glfw
        glad::glad
        glm::glm
)
