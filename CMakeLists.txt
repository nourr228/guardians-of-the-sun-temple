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
        src/demo/CharacterShowcase.cpp
        src/demo/GuardianPractice.cpp
        src/demo/WavePractice.cpp
        src/entities/Enemy.cpp
        src/entities/Guardian.cpp
        src/rendering/Animator.cpp
        src/rendering/Camera2D.cpp
        src/rendering/CharacterRenderer.cpp
        src/rendering/Renderer2D.cpp
        src/rendering/ResourceManager.cpp
        src/rendering/Shader.cpp
        src/rendering/SpriteRenderer.cpp
        src/rendering/Texture.cpp
        src/systems/GuardianManager.cpp
        src/systems/EnemyManager.cpp
        src/systems/EnemySpawner.cpp
        src/systems/WaveManager.cpp
        src/world/Battlefield.cpp
        src/world/HomeBase.cpp
        src/world/TempleEnvironment.cpp
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
    if (MINGW)
        get_filename_component(COMPILER_RUNTIME_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        foreach (RUNTIME_DLL IN ITEMS
                libgcc_s_seh-1.dll
                libstdc++-6.dll
                libwinpthread-1.dll)
            add_custom_command(TARGET Guardians_of_the_Sun_Temple POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${COMPILER_RUNTIME_DIR}/${RUNTIME_DLL}"
                    $<TARGET_FILE_DIR:Guardians_of_the_Sun_Temple>
            )
        endforeach ()
    endif ()
endif ()

include(CTest)
if (BUILD_TESTING)
    add_executable(animator_tests
            tests/animator_tests.cpp
            src/rendering/Animator.cpp
    )
    target_include_directories(animator_tests PRIVATE src)
    add_test(NAME animator_tests COMMAND animator_tests)
    add_executable(world_geometry_tests
            tests/world_geometry_tests.cpp
            src/world/Battlefield.cpp
            src/rendering/Camera2D.cpp
    )
    target_include_directories(world_geometry_tests PRIVATE src)
    target_link_libraries(world_geometry_tests PRIVATE glm::glm)
    add_test(NAME world_geometry_tests COMMAND world_geometry_tests)
    add_executable(guardian_tests
            tests/guardian_tests.cpp
            src/entities/Guardian.cpp
            src/systems/GuardianManager.cpp
            src/rendering/Animator.cpp
            src/world/Battlefield.cpp
    )
    target_include_directories(guardian_tests PRIVATE src)
    target_link_libraries(guardian_tests PRIVATE glm::glm)
    add_test(NAME guardian_tests COMMAND guardian_tests)
    add_executable(enemy_wave_tests
            tests/enemy_wave_tests.cpp
            src/entities/Enemy.cpp
            src/entities/Guardian.cpp
            src/rendering/Animator.cpp
            src/systems/EnemyManager.cpp
            src/systems/EnemySpawner.cpp
            src/systems/GuardianManager.cpp
            src/systems/WaveManager.cpp
            src/world/Battlefield.cpp
    )
    target_include_directories(enemy_wave_tests PRIVATE src)
    target_link_libraries(enemy_wave_tests PRIVATE glm::glm)
    add_test(NAME enemy_wave_tests COMMAND enemy_wave_tests)
    if (WIN32 AND MINGW)
        set_tests_properties(world_geometry_tests PROPERTIES
                ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${COMPILER_RUNTIME_DIR}")
        set_tests_properties(animator_tests PROPERTIES
                ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${COMPILER_RUNTIME_DIR}")
        set_tests_properties(guardian_tests PROPERTIES
                ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${COMPILER_RUNTIME_DIR}")
        set_tests_properties(enemy_wave_tests PROPERTIES
                ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${COMPILER_RUNTIME_DIR}")
    endif ()
endif ()
