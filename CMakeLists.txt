cmake_minimum_required(VERSION 3.21)
project(Guardians_of_the_Sun_Temple LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_path(STB_INCLUDE_DIR stb_image.h REQUIRED)

add_executable(Guardians_of_the_Sun_Temple
        main.cpp
        src/core/Game.cpp
        src/core/InputManager.cpp
        src/rendering/Camera2D.cpp
        src/rendering/Renderer2D.cpp
        src/rendering/ResourceManager.cpp
        src/rendering/Shader.cpp
        src/rendering/SpriteRenderer.cpp
        src/rendering/Texture.cpp
)

target_include_directories(Guardians_of_the_Sun_Temple PRIVATE src "${STB_INCLUDE_DIR}")
target_compile_definitions(Guardians_of_the_Sun_Temple PRIVATE
        GLFW_INCLUDE_NONE
        GOST_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)
target_link_libraries(Guardians_of_the_Sun_Temple PRIVATE
        OpenGL::GL
        glfw
        glad::glad
        glm::glm
)

if (WIN32)
    add_custom_command(TARGET Guardians_of_the_Sun_Temple POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:Guardians_of_the_Sun_Temple>
            $<TARGET_FILE_DIR:Guardians_of_the_Sun_Temple>
            COMMAND_EXPAND_LISTS
    )
endif ()
