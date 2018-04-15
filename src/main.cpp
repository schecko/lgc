#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned int u32;

void glfwCallback(int error, const char* description)
{
	printf("Error: %s\n", description);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

struct Cell
{
	int age;
};

const u32 NUM_NEIGHTBOURS = 9;

inline Cell nextGeneration(Cell neighbourhood[NUM_NEIGHTBOURS])
{
	Cell nextGen;

	u32 age = 0;

	for (u32 i = 0; i < NUM_NEIGHTBOURS; i++)
	{
		age ^= neighbourhood[i].age;
	}

	nextGen.age = age;

	return nextGen;
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
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSetKeyCallback(window, keyCallback);
	glfwSwapInterval(1);

	const u32 NUM_CELL_BUFFERS = 2;
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);

#define LAZY (Cell*)malloc((width + 1) * (height + 1) * sizeof(Cell))
	Cell* cells[NUM_CELL_BUFFERS] = { LAZY, LAZY };
	cells[0] = &cells[0][width];
	cells[1] = &cells[1][width];
	u32 currentCellBuffer = 0;

	u32 vbos[NUM_CELL_BUFFERS];
	glGenBuffers(NUM_CELL_BUFFERS, vbos);

	static const char* vertexCode = 
	R"END(
		uniform mat4 MVP;
		attribute vec3 vCol;
		attribute vec2 vPos;
		varying vec3 color;
		void main()
		{
		    gl_Position = MVP * vec4(vPos, 0.0, 1.0);
		    color = vCol;
		}
	)END";

	static const char* fragmentCode =
	R"END(
		varying vec3 color;
		void main()
		{
		    gl_FragColor = vec4(color, 1.0);
		}
	)END";

	u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertexCode, NULL);
	glCompileShader(vertex_shader);
	u32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragmentCode, NULL);
	glCompileShader(fragment_shader);

	u32 pipeline = glCreateProgram();
	glAttachShader(pipeline, vertex_shader);
	glAttachShader(pipeline, fragment_shader);
	glLinkProgram(pipeline);

	while (!glfwWindowShouldClose(window))
	{
		u32 nextCellBuffer = (currentCellBuffer + 1) % NUM_CELL_BUFFERS;
		// input
		glfwPollEvents();

		glBindBuffer(GL_ARRAY_BUFFER, vbos[currentCellBuffer]);


		for (u32 i = 0; i < width - 1; i++)
		{
			for (u32 j = 0; j < height - 1; j++)
			{
				Cell* cellBuffer = cells[currentCellBuffer];
				Cell* nextCellBuffer = cells[currentCellBuffer];
				Cell neighbourhood[NUM_NEIGHTBOURS];

				u32 topLeft = (i - 1) + j * (width - 1);
				u32 left = (i - 1) + j * width;
				u32 bottomLeft = (i - 1) + j * (width + 1);

				u32 index = 0;
				for (u32 n = 0; n < 3; n++)
				{
					neighbourhood[index++] = cellBuffer[(topLeft + n) % (width * height)];
					neighbourhood[index++] = cellBuffer[(left + n) % (width * height)];
					neighbourhood[index++] = cellBuffer[(bottomLeft + n) % (width * height)];
				}

				nextCellBuffer[(left + 1) % (width * height)] = nextGeneration(neighbourhood);
			}
		}

		glBufferData(GL_ARRAY_BUFFER,
					 width * height * sizeof(Cell),
					 cells[currentCellBuffer],
					 GL_STATIC_DRAW);

		currentCellBuffer = nextCellBuffer;

		// swap
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
    return 0;
}