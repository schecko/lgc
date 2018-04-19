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

	const u32 realTextureSize = (width + 1) * (height + 1) * sizeof(Cell);
	const u32 usableTextureSize = width * height * sizeof(Cell);

#define LAZY (Cell*)malloc(realTextureSize)
	Cell* realPtrs[NUM_CELL_BUFFERS] = { LAZY, LAZY };
	Cell* cells[NUM_CELL_BUFFERS] = { &cells[0][width], &cells[1][width] };

	u32 currentCellBuffer = 0;

	u32 cellUniforms[NUM_CELL_BUFFERS];
	glGenBuffers(NUM_CELL_BUFFERS, cellUniforms);
	for (u32 i = 0; i < NUM_CELL_BUFFERS; i++)
	{
		memset(realPtrs[i], 0, realTextureSize);
		glBindBuffer(GL_TEXTURE_BUFFER, cellUniforms[i]);
		glBufferData(GL_TEXTURE_BUFFER,
					 usableTextureSize,
					 cells[i],
					 GL_DYNAMIC_DRAW);
	}

	struct Vert
	{
		float pos[2];
		float uv[2];
	};

	Vert verts[] = 
	{
		// positions    // texture coords
		1.0f,  1.0f,	1.0f, 1.0f, // top right
		1.0f, -1.0f,	1.0f, 0.0f, // bottom right
		-1.0f, -1.0f,	0.0f, 0.0f, // bottom left
		-1.0f,  1.0f,	0.0f, 1.0f  // top left 
	};

	u32 indices[] = 
	{
		0, 1, 3,
		1, 2, 3
	};

	// prepare the vertex array object
	u32 vao;
	glGenVertexArrays(1, &vao);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)offsetof(Vert, pos));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)offsetof(Vert, uv));
	glEnableVertexAttribArray(1);

	// setup the static quad data
	u32 vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	// setup static index data
	u32 ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ARRAY_BUFFER, ibo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	static const char rawVertexCode[] = 
	R"END(
		in vec2 vPos;
		void main()
		{
		    gl_Position = vec4(vPos, 0.0, 1.0);
		}
	)END";

	char vertexCode[sizeof(rawVertexCode) * 2] = {};
	sprintf(vertexCode, rawVertexCode);

	static const char* rawFragmentCode =
	R"END(
		struct Cell
		{
			int age;
		};

		uniform Cell cells[%i];
		void main()
		{
		    gl_FragColor = ;
		}
	)END";
	char fragmentCode[sizeof(rawFragmentCode) * 2] = {};
	sprintf(fragmentCode, rawVertexCode, realTextureSize);


	u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &(const char*)vertexCode, NULL);
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

		glBindBuffer(GL_TEXTURE_BUFFER, generations[currentCellBuffer]);

		// update the host buffer
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

		// update the device buffer
		glBufferSubData(GL_TEXTURE_BUFFER, 0, width * height * sizeof(Cell), cells[currentCellBuffer]);

		currentCellBuffer = nextCellBuffer;

		// swap
		glfwSwapBuffers(window);
	}

	for (u32 i = 0; i < NUM_CELL_BUFFERS; i++)
	{
		free(realPtrs[i]);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
    return 0;
}