#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

int main() {

    // -------------------------
    // 1. Initialize GLFW
    // -------------------------

    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW"
                  << std::endl;

        return -1;
    }


    // -------------------------
    // 2. Request OpenGL 3.3
    // -------------------------

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MAJOR,
        3
    );

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MINOR,
        3
    );

    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );


    // -------------------------
    // 3. Create window
    // -------------------------

    GLFWwindow* window =
        glfwCreateWindow(
            1000,
            600,
            "DMET 502 Game",
            nullptr,
            nullptr
        );


    if (window == nullptr) {

        std::cout
            << "Failed to create GLFW window"
            << std::endl;

        glfwTerminate();

        return -1;
    }


    // -------------------------
    // 4. Make OpenGL context
    // -------------------------

    glfwMakeContextCurrent(window);


    // -------------------------
    // 5. Initialize GLAD
    // -------------------------

    if (!gladLoadGLLoader(
            (GLADloadproc)
            glfwGetProcAddress
        )) {

        std::cout
            << "Failed to initialize GLAD"
            << std::endl;

        return -1;
    }


    // -------------------------
    // 6. Print OpenGL version
    // -------------------------

    std::cout
        << "OpenGL Version: "
        << glGetString(GL_VERSION)
        << std::endl;


    // -------------------------
    // 7. Main game loop
    // -------------------------

    while (!glfwWindowShouldClose(window)) {

        glClearColor(
            0.1f,
            0.2f,
            0.3f,
            1.0f
        );

        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }


    // -------------------------
    // 8. Cleanup
    // -------------------------

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}