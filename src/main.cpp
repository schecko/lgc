#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <vector>

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

void printError(u32 glError)
{
	switch (glError)
	{
		case GL_NO_ERROR:
		{
		} break;
		case GL_INVALID_ENUM:
		{
			printf("GL_INVALID_ENUM\n");
		} break;
		case GL_INVALID_VALUE:
		{
			printf("GL_INVALID_VALUE\n");
		} break;
		case GL_INVALID_OPERATION:
		{
			printf("GL_INVALID_OPERATION\n");
		} break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
		{
			printf("GL_INVALID_FRAMEBUFFER_OPERATION\n");
		} break;
		case GL_OUT_OF_MEMORY:
		{
			printf("GL_OUT_OF_MEMORY\n");
		} break;
		case GL_STACK_UNDERFLOW:
		{
			printf("GL_STACK_UNDERFLOW\n");
		} break;
		case GL_STACK_OVERFLOW:
		{
			printf("GL_STACK_OVERFLOW\n");
		} break;
		default:
		{
			printf("other gl error occurred!\n");
		}
	}
}

bool checkShaderCompile(u32 shader)
{
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		printf(&errorLog[0]);
		printf("\n");

		// Exit with failure.
		glDeleteShader(shader); // Don't leak the shader.
		return false;
	}

	return true;
}


#define GLE {u32 error = glGetError(); printError(error); assert(error == GL_NO_ERROR); }

static const u32 NUM_NEIGHTBOURS = 9;
static const u32 WIDTH = 480;
static const u32 HEIGHT = 640;

void APIENTRY oglDebugCallback(GLenum source​,
							   GLenum type​,
							   GLuint id​,
							   GLenum severity​,
							   GLsizei length​,
							   const GLchar* message​,
							   const void* userParam​)
{
	printf(message​);
}

static char logBuffer[512] = {};

int main(int argc, char** argv)
{
	if (!glfwInit())
	{
		return 1;
	}
	glfwSetErrorCallback(glfwCallback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LGC", NULL, NULL);
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
	assert(width == WIDTH);
	assert(height == HEIGHT);

	u32 currentCellBuffer = 0;

	typedef u32 Ages[WIDTH][HEIGHT];
	static Ages cellAges[NUM_CELL_BUFFERS] = {};

	u32 ageBuffers[NUM_CELL_BUFFERS];
	glGenBuffers(NUM_CELL_BUFFERS, ageBuffers);
	GLE;
	for (u32 i = 0; i < NUM_CELL_BUFFERS; i++)
	{
		Ages* currentCellAges = &cellAges[i];

		memset(currentCellAges, 0, sizeof(Ages));
		glBindBuffer(GL_TEXTURE_BUFFER, ageBuffers[i]);
		GLE;
		glBufferData(GL_TEXTURE_BUFFER,
					 sizeof(Ages),
					 currentCellAges,
					 GL_DYNAMIC_DRAW);
		GLE;
	}

	u32 ageTextures[NUM_CELL_BUFFERS];
	glGenTextures(NUM_CELL_BUFFERS, ageTextures);

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

	// setup the static quad data
	u32 vbo;
	glGenBuffers(1, &vbo);
	GLE;
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GLE;
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	GLE;

	// setup static index data
	u32 ibo;
	glGenBuffers(1, &ibo);
	GLE;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	GLE;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	GLE;

	// prepare the vertex array object
	// create vao
	u32 vao;
	glGenVertexArrays(1, &vao);
	GLE;
	glBindVertexArray(vao);
	GLE;
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)offsetof(Vert, pos));
	GLE;
	glEnableVertexAttribArray(0);
	GLE;
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)offsetof(Vert, uv));
	GLE;
	glEnableVertexAttribArray(1);
	GLE;

	const char rawVertexCode[] = 
	R"END(
		#version 400
		in vec2 vPos;
		void main()
		{
		    gl_Position = vec4(vPos, 0.0, 1.0);
		}
	)END";

	const u32 vertexBufferSize = sizeof(rawVertexCode) * 2;
	char vertexCode[vertexBufferSize] = {};
	u32 vertexWritten = sprintf_s(vertexCode, sizeof(rawVertexCode) * 2, rawVertexCode);
	assert(vertexWritten < vertexBufferSize);

	const char rawFragmentCode[] =
	R"END(
		#version 400

		const int width = %i;
		const int height = %i;
		uniform samplerBuffer cellAges;

		layout(location = 0) out vec4 color;

		void main()
		{
		    int x = int(gl_FragCoord.x * width);
		    int y = int(gl_FragCoord.y * height);
			int cellAge = life.ages[x + y * width];
			if(cellAge > 0)
			{
				if(cellAge > 100)
				{
					color = vec4(1.0, 1.0, 1.0, 1.0);
				} else if(cellAge > 10)
				{
					color = vec4(0.75, 0.75, 0.75, 1.0);
				} else if(cellAge > 1)
				{
					color = vec4(0.5, 0.5, 0.5, 1.0);
				} else
				{
					color = vec4(0.25, 0.25, 0.25, 1.0);
				}
			} else
			{
				color = vec4(0.0, 0.0, 0.0, 1.0);
			}
			 
		}
	)END";
	const u32 fragmentBufferSize = sizeof(rawFragmentCode) * 2;
	char fragmentCode[fragmentBufferSize] = {};
	u32 fragmentWritten = sprintf_s(fragmentCode, fragmentBufferSize, rawFragmentCode, WIDTH, HEIGHT);
	assert(fragmentWritten < fragmentBufferSize);

	u32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char* vertexCode2 = vertexCode;
	glShaderSource(vertexShader, 1, &vertexCode2, NULL);
	GLE;
	glCompileShader(vertexShader);
	assert(checkShaderCompile(vertexShader));
	u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* fragmentCode2 = fragmentCode;
	glShaderSource(fragmentShader, 1, &fragmentCode2, NULL);
	GLE;
	glCompileShader(fragmentShader);
	assert(checkShaderCompile(fragmentShader));
	GLE;

	u32 pipeline = glCreateProgram();
	GLE;
	glAttachShader(pipeline, vertexShader);
	GLE;
	glAttachShader(pipeline, fragmentShader);
	GLE;
	glLinkProgram(pipeline);
	GLE;

	int success = false;
	glGetProgramiv(pipeline, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(pipeline, sizeof(logBuffer), NULL, logBuffer);
		printf("SHADER LINK FAILED\n%s\n", logBuffer);
		assert(false);
	}

	while (!glfwWindowShouldClose(window))
	{
		u32 nextCellBufferIndex = (currentCellBuffer + 1) % NUM_CELL_BUFFERS;
		// input
		glfwPollEvents();

		glBindBuffer(GL_TEXTURE_BUFFER, ageBuffers[currentCellBuffer]);
		GLE;
		// update the host buffer
		for (u32 i = 1; i < WIDTH - 1; i++)
		{
			for (u32 j = 1; j < HEIGHT - 1; j++)
			{
				Ages* cellBuffer = &cellAges[currentCellBuffer];
				Ages* nextCellBuffer = &cellAges[nextCellBufferIndex];

				u32 numAliveNeighbours = 0;

				// top row
				numAliveNeighbours += (*cellBuffer)[i - 1][j + 1] ? 1 : 0;
				numAliveNeighbours += (*cellBuffer)[i][j + 1] ? 1 : 0;
				numAliveNeighbours += (*cellBuffer)[i + 1][j + 1] ? 1 : 0;

				// middle row
				numAliveNeighbours += (*cellBuffer)[i - 1][j] ? 1 : 0;
				//numAliveNeighbours += (*cellBuffer)[i][j].alive? 1 : 0;
				numAliveNeighbours += (*cellBuffer)[i + 1][j] ? 1 : 0;

				// bottom row
				numAliveNeighbours += (*cellBuffer)[i - 1][j - 1] ? 1 : 0;
				numAliveNeighbours += (*cellBuffer)[i][j - 1] ? 1 : 0;
				numAliveNeighbours += (*cellBuffer)[i + 1][j - 1] ? 1 : 0;

				switch (numAliveNeighbours)
				{
					case 0:
					case 1:
					{
						(*nextCellBuffer)[i][j] = 0;
					} break;
					case 2:
					case 3:
					{
						if ((*cellBuffer)[i][j])
						{
							(*nextCellBuffer)[i][j] = (*cellBuffer)[i][j] + 1;
						}
						else
						{
							(*nextCellBuffer)[i][j] = 1;
						}
					} break;
					default:
					{
						(*nextCellBuffer)[i][j] = 0;
					}
				}
			}
		}

		// update the device buffer
		glBufferSubData(GL_TEXTURE_BUFFER, 
						0, 
						sizeof(Ages),
						&cellAges[nextCellBufferIndex]);
		GLE;

		// clear and start drawing
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		GLE;
		glClear(GL_COLOR_BUFFER_BIT);
		GLE;
		glUseProgram(pipeline);
		GLE;
		//glBindVertexArray(vao);
		GLE;
		/*glBindBufferBase(GL_UNIFORM_BUFFER, 
						 0, 
						 cellUniforms[currentCellBuffer]);*/
		GLE;
		// glUniformBlockBinding(pipeline, cellsLocation, 0);
		// GLE;
		//glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_INT, 0);
		GLE;
		currentCellBuffer = nextCellBufferIndex;

		// swap
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
    return 0;
}