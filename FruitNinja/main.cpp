#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "main.h"
#include <map>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

World* world;

// the program starts here
void AppMain()
{
    // initialise GLFW
    //glfwSetErrorCallback(OnError);
    if (!glfwInit())
        throw std::runtime_error("glfwInit failed");

    // open a window with GLFW
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "Fruit Ninja", NULL, NULL);
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(window, mode->width / 2.f - screen_width / 2.f, mode->height / 2.f - screen_height / 2.f);
	
	//if you want full-screen, uncomment the following line and comment the previous three
	//GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "Fruit Ninja", glfwGetPrimaryMonitor(), NULL);

    if (!window)
        throw std::runtime_error("glfwCreateWindow failed. Can your hardware handle OpenGL 3.2?");

    //glfwSetScrollCallback(window, OnScroll);
    glfwMakeContextCurrent(window);

    // initialise GLEW
    glewExperimental = GL_TRUE; //stops glew crashing on OSX :-/
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("glewInit failed");

    // GLEW throws some errors, so discard all the errors so far
    while (glGetError() != GL_NO_ERROR) {}

    // print out some info about the graphics drivers
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    // make sure OpenGL version 3.2 API is available
    if (!GLEW_VERSION_3_2)
        throw std::runtime_error("OpenGL 3.2 API is not available.");

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(0.f, 0.f, 0.f, 1.0f);

    world = new World();

    // Set Key Callback Function
    glfwSetKeyCallback(window, &World::key_callback);

    // Set Mouse Callback Function
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPos(window, 0, 0);
    glfwSetCursorPosCallback(window, &World::mouse_callback);

    glfwSetScrollCallback(window, &World::scroll_callback);

	glfwSetMouseButtonCallback(window, &World::mouse_button_callback);

	glfwSetWindowSizeCallback(window, &World::resize_window);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glfwSetTime(0.f);
	int i = 0;
	float frameTime = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

		world->update();
        world->draw();

		if (!(i % 50))
		{
			cout << "Frame Rate: " << frameTime << endl;
			frameTime = 0.0f;
		}
		frameTime += game_speed / actual_seconds_passed / 50.0f;
		i++;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
	
	delete world;

    glfwTerminate();
}

int main(int argc, char *argv[])
{
    try
    {
        AppMain();
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        system("PAUSE");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}