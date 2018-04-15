#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <stdio.h>

void glfwCallback(int error, const char* description)
{
	printf("Error: %s\n", description);
}

int main()
{
	if (!glfwInit())
	{
		return 1;
	}
	glfwSetErrorCallback(glfwCallback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(640, 480, "LGC", NULL, NULL);
	if (!window)
	{
		return 1;
	}
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwDestroyWindow(window);
	glfwTerminate();
    return 0;
}