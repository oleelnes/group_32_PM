/******************************************************************************\
| GRAPHICS PROGRAMMING				~ PROG2002 ~				OBLIG 1		   |
|	By Anders Brunsberg Mariendal											   |
|	and Ole Kristian Elnæs													   |
|******************************************************************************|
| ACKNOWLEDGEMENTS															   |
|	- The code suffers from a lack of modularity. Modularity was on the agenda |
|	but we struggled with some aspects of the project and weren't able to have |
|	enough time to focus on it. 											   |
\******************************************************************************//**/

#include <iostream>
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include<iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>

using namespace std;

const int N = 1009;
const float wall_width = 1.0f / 18.0f;
const float wall_height = (2.0f / 36.0f);
float xStart = -1.0f;
float yStart = 1.0f;
int wallCounter = 0;
int level[N]; //array that stores the numbers read from the "level0.txt" file
const float yAspectRatio = 7.0f / 9.0f;

GLuint createShader(const std::string& vs_source, const std::string& fs_source);

GLuint load_opengl_texture(const std::string& filepath, GLuint slot);

void loadLevel();

/*ALL SHADERS HARDCODED HERE*/
const char* wall_vs_source =
"#version 410\n"
"in layout(location = 1) vec3 wall_position;\n"
"in layout(location = 2) float offset_x; \n"
"in layout(location = 3) float offset_y; \n"
"void main()\n"
"{\n"
"   gl_Position = vec4(wall_position.x + offset_x, wall_position.y + offset_y, wall_position.z, 1.0);\n"
"}\0";

const char* wall_fs_source =
"#version 410\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(0.1f, 0.0f, 0.5f, 1.0f);\n" //walls are a medium dark blue
"}\n";

const char* dots_vs_source =
"#version 410\n"
"in layout(location = 4) vec3 dot_position;\n"
"in layout(location = 5) float dot_offset_x; \n"
"in layout(location = 6) float dot_offset_y; \n"
"void main()\n"
"{\n"
"   gl_Position = vec4(dot_position.x + dot_offset_x, dot_position.y + dot_offset_y, dot_position.z, 1.0);\n"
"}\0";

const char* dots_fs_source =
"#version 410\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n" //yellow dots for the pacman to eaty-eaty
"}\n";

const char* pacMan_vs_src =
"#version 410\n"
"in layout(location = 7) vec2 aPosition; \n"
"in layout(location = 8) vec3 aColor;"
"in layout(location = 9) vec2 aTexcoord;"
"out vec3 vsColor;"
"out vec2 vsTexcoord;"
"uniform float m_x;"
"uniform float m_y;"
"uniform float pacMan_x;"
"uniform float pacMan_y;"
"void main () {\n"
"	float w = 1.0/6.0;"
"	float h = 1.0/4.0;"
"	vsColor = aColor;"
"	vsTexcoord = vec2((aTexcoord.x + (m_x * (w - 0.007))), (aTexcoord.y + (m_y * h)));"
"	gl_Position = vec4((aPosition.x/3.8) + pacMan_x, (((aPosition.y/3.8) ) * (7.0 / 9.0))+ pacMan_y, 0.0, 1.0 ); \n" // 7 / 9 is to account for the aspect ratio of the screen
"}\n";
//vec3=xyz-value

const char* pacMan_fs_src =
"#version 410\n"
"in vec3 vsColor;"
"in vec2 vsTexcoord;"
"out vec4 outColor;"
"uniform sampler2D uTexture;\n"
"void main () {"
"  outColor = texture(uTexture, vsTexcoord);"
"}";

const char* ghost_vs_src =
"#version 410\n"
"in layout(location = 10) vec2 aPosition; \n"
"in layout(location = 11) vec3 aColor;"
"in layout(location = 12) vec2 aTexcoord;"
"out vec3 vsColor;"
"out vec2 vsTexcoord;"
"uniform float g_rot_x;"
"uniform float g_rot_y;"
"uniform float ghost_x;"
"uniform float ghost_y;"
"void main () {\n"
"	float w = 1.0/6.0;"
"	float h = 1.0/4.0;"
"	vsColor = aColor;"
"	vsTexcoord = vec2((aTexcoord.x + (g_rot_x * (w - 0.002))), (aTexcoord.y + (g_rot_y * (h - 0.007))));"
"	gl_Position = vec4((aPosition.x / 3.8) + ghost_x, (((aPosition.y / 3.8) ) * (7.0 / 9.0))+ ghost_y, 0.0, 1.0 ); \n" // 7 / 9 is to account for the aspect ratio of the screen
"}\n";

const char* ghost_fs_src =
"#version 410\n"
"in vec3 vsColor;"
"in vec2 vsTexcoord;"
"out vec4 outColor;"
"uniform sampler2D uTexture;\n"
"void main () {"
"  outColor = texture(uTexture, vsTexcoord);"
"}";



/*Creates the window with the specified viewport*/
void windowMake(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWmonitor* a = glfwGetPrimaryMonitor();

	GLFWwindow* window = glfwCreateWindow(800, 800, "PacMan Game", NULL, NULL);
	glfwSetWindowAspectRatio(window, 28, 28);

	glfwMakeContextCurrent(window);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwSetFramebufferSizeCallback(window, windowMake);

	srand(time(NULL)); rand(); //throwing away first random value


	loadLevel();


	GLfloat wall_offsets_x[900], wall_offsets_y[900];
	GLfloat dots_offsets_x[900], dots_offsets_y[900];
	float pacMan_x_Start = 0, pacMan_y_Start = 0;
	wall_offsets_x[0] = 0.0f; wall_offsets_y[0] = 0.0f;
	int offset_index = 1, dot_offset_index = 0;

	/* HERE AFTER IT STOPS -- THE PROGRAM FAILS TO GO THROUGH THIS LOOP -- NUMBER OF WALLS IS 0*/

	/* This for-loop calculates the positions of the wall blocks, the yellow dots and the start position of the pacMan*/
	for (int y = 0; y < 36; y++) {
		for (int x = 1; x < 29; x++) {
			if (level[(28 * y) + x] == 1) {
				float position_x = ((2.0f / 36.0f) * (x - 1)); float position_y = -((2.0f / 36.0f) * (y)); //calculating the x- and y-position of the wall blocks
				wall_offsets_x[offset_index] = position_x; wall_offsets_y[offset_index] = position_y; //inserting the wall positions into their location in the array
				offset_index++;
			}
			else if (level[(28 * y) + x] == 0) {
				float dot_position_x = ((2.0f / 36.0f) * (x - 1)); float dot_position_y = -((2.0f / 36.0f) * (y)); //calculating the x- and y-position of the dots
				dots_offsets_x[dot_offset_index] = dot_position_x; dots_offsets_y[dot_offset_index] = dot_position_y; //inserting the dot positions into their location in the array
				dot_offset_index++;
			}
			if (level[(28 * y) + x] == 2) {
				float player_position_x = -0.75f * 3.8f;
				float player_position_y = 0.0f + (1.0f / 4.0f) + (1.0f / 80.0f);
				pacMan_x_Start = player_position_x;
				pacMan_y_Start = player_position_y;
			}
		}
	}
	//cout << "loc here" << endl;
	/*VERTICES AND INDICES FOR ALL ITEMS*/
	float wall_vertices[]
	{
		-1.0f + (5.0f / 18.0f),		1.0f,						1.0f,
		-1.0f + (2.0f / 9.0f),		1.0f,						1.0f,
		-1.0f + (2.0f / 9.0f),		1.0f - (2.0f / 36.0f),		1.0f,
		-1.0f + (5.0f / 18.0f),		1.0f - (2.0f / 36.0f),		1.0f
	};

	float dotStart_x = -1.0f + ((1.0f / 36.0f) - (1.0f / 144.0f));
	float dotStart_y = 1.0f - ((1.0f / 36.0f) - (1.0f / 144.0f));

	float dot_vertices[]{
		dotStart_x + (17.0f / 72.0f),	dotStart_y,							1.0f,
		dotStart_x + (2.0f / 9.0f),		dotStart_y,							1.0f,
		dotStart_x + (2.0f / 9.0f),		dotStart_y - (1.0f / 72.0f),		1.0f,
		dotStart_x + (17.0f / 72.0f),	dotStart_y - (1.0f / 72.0f),		1.0f
	};

	unsigned int wall_indices[]
	{
		0,1,2,
		0,2,3,
	};

	unsigned int dot_indices[]
	{
		0,1,2,
		0,2,3,
	};

	GLfloat ghost_offset_x = 0.02f;
	GLfloat ghost_tex_x = (1.0f / 6.0f);
	GLfloat ghost_tex_y = 1.0f / 4.0f;
	GLfloat ghost_x_Start = 0.0f;
	GLfloat ghost_y_Start = 1.09f;

	GLfloat ghost_vertices[4 * 7] =
	{
		/*
	  |position									|color				|texture coord*/
		0.0f,				1.09f,				1.0f, 1.0f, 1.0f,	0.0f,			0.0f,
		0.0f,				1.09f - ghost_tex_y,	1.0f, 1.0f, 1.0f,	0.0f,			ghost_tex_y,
		0.0f + ghost_tex_x,	1.09f - ghost_tex_y, 1.0f, 1.0f, 1.0f,	ghost_tex_x,	ghost_tex_y,
		0.0f + ghost_tex_x,	1.09f,				1.0f, 1.0f, 1.0f,	ghost_tex_x,	0.0f,
	};

	GLuint ghost_indices[6] = { 0,1,2,0,2,3 };

	GLfloat offset_x = 0.02f;
	GLfloat tex_x = (1.0f / 6.0f);
	GLfloat tex_y = (1.0f / 4.0f);
	GLfloat pacPos;

	GLfloat pacMan_vertices[] =
	{
		/*|position											|color				  |texture coord*/
		  pacMan_x_Start,			pacMan_y_Start,			1.0f, 1.0f, 1.0f,		0.0f,	tex_y,
		  pacMan_x_Start,			pacMan_y_Start - tex_y, 1.0f, 1.0f, 1.0f,		0.0f,	0.0f,
		  pacMan_x_Start + tex_x,	pacMan_y_Start - tex_y, 1.0f, 1.0f, 1.0f,		tex_x,	0.0f,
		  pacMan_x_Start + tex_x,	pacMan_y_Start,			1.0f, 1.0f, 1.0f,		tex_x,	tex_y,
	};

	GLuint pacMan_indices[6] = { 0,1,2,0,2,3 };

	/*pacMan hitbox
					   pacMan_y_max
							|
							V
						 _ _ _ _
						|		|
	pacMan_x_min->		|		|	<- pacMan_x_max
						|_ _ _ _|

							^
							|
					   pacMan_y_min
	*/
	float pacMan_min_y, pacMan_max_y, pacMan_min_x, pacMan_max_x; //these variables constitute the hitbox of the pacMan
	float hitBox_adjust_y = (1.0f / 10.0f) * (tex_y * yAspectRatio);
	float hitBox_adjust_x = (1.0f / 10.0f) * tex_x;
	pacMan_min_y = (((pacMan_y_Start - tex_y) * yAspectRatio) / 3.8f) + hitBox_adjust_y;
	pacMan_max_y = (((pacMan_y_Start)*yAspectRatio) / 3.8f) - hitBox_adjust_y;
	pacMan_min_x = ((pacMan_x_Start + tex_x) / 3.8f) - hitBox_adjust_x;
	pacMan_max_x = (pacMan_x_Start / 3.8f) + hitBox_adjust_x;

	/*GHOST HITBOX*/
	float ghost_min_y, ghost_max_y, ghost_min_x, ghost_max_x; //these variables constitute the hitbox of the pacMan
	float ghost_hitBox_adjust_y = (1.0f / 10.0f) * (ghost_tex_y * yAspectRatio);
	float ghost_hitBox_adjust_x = (1.0f / 10.0f) * ghost_tex_x;
	ghost_min_y = (((ghost_y_Start - ghost_tex_y) * yAspectRatio) / 3.8f) + ghost_hitBox_adjust_y;
	ghost_max_y = (((ghost_y_Start)*yAspectRatio) / 3.8f) - ghost_hitBox_adjust_y;
	ghost_min_x = ((ghost_x_Start + ghost_tex_x) / 3.8f) - ghost_hitBox_adjust_x;
	ghost_max_x = (ghost_x_Start / 3.8f) + ghost_hitBox_adjust_x;


	/* VAO CREATION */
	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/*WALLS*/
	/*VBO AND EBO FOR WALLS*/
	{
		unsigned int wall_vbo;
		glGenBuffers(1, &wall_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, wall_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(wall_vertices), wall_vertices, GL_STATIC_DRAW);

		unsigned int ebo;
		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(wall_indices), wall_indices, GL_STATIC_DRAW);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL); //endret siste par. fra (void*)0 til NULL
		glEnableVertexAttribArray(1);
	}

	/*OFFSET BUFFERS FOR WALLS*/
	{
		GLuint offsetsBufferID_x;
		glGenBuffers(1, &offsetsBufferID_x);
		glBindBuffer(GL_ARRAY_BUFFER, offsetsBufferID_x);
		glBufferData(GL_ARRAY_BUFFER, sizeof(wall_offsets_x), wall_offsets_x, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(2, 1);

		/*Buffers etc for offsets_y*/
		GLuint offsetsBufferID_y;
		glGenBuffers(1, &offsetsBufferID_y);
		glBindBuffer(GL_ARRAY_BUFFER, offsetsBufferID_y);
		glBufferData(GL_ARRAY_BUFFER, sizeof(wall_offsets_y), wall_offsets_y, GL_STATIC_DRAW);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(3, 1);
	}


	/*DOTS*/

	/*VBO AND EBO FOR DOTS*/
	{

		unsigned int dot_vbo;
		glGenBuffers(1, &dot_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, dot_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(dot_vertices), dot_vertices, GL_STATIC_DRAW);

		unsigned int dot_ebo;
		glGenBuffers(1, &dot_ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dot_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(dot_indices), dot_indices, GL_STATIC_DRAW);

		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
		glEnableVertexAttribArray(4);
	}

	/*OFFSET BUFFERS FOR DOTS*/
	GLuint dot_offsetsBufferID_x;
	glGenBuffers(1, &dot_offsetsBufferID_x);
	glBindBuffer(GL_ARRAY_BUFFER, dot_offsetsBufferID_x);
	glBufferData(GL_ARRAY_BUFFER, sizeof(dots_offsets_x), dots_offsets_x, GL_STATIC_DRAW);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(5, 1);

	/*Buffers etc for offsets_y*/
	GLuint dot_offsetsBufferID_y;
	glGenBuffers(1, &dot_offsetsBufferID_y);
	glBindBuffer(GL_ARRAY_BUFFER, dot_offsetsBufferID_y);
	glBufferData(GL_ARRAY_BUFFER, sizeof(dots_offsets_y), dots_offsets_y, GL_STATIC_DRAW);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(6, 1);

	/* Creating shader programs for walls, dots and TODO: THE REST*/
	GLuint wall_shaderprogram = createShader(wall_vs_source, wall_fs_source);
	GLuint dots_shaderprogram = createShader(dots_vs_source, dots_fs_source);


	/*PACMAN CREATION*/
	{

		GLuint pacMan_vbo;
		glGenBuffers(1, &pacMan_vbo);

		GLuint pacMan_ebo;
		glGenBuffers(1, &pacMan_ebo);

		glBindBuffer(GL_ARRAY_BUFFER, pacMan_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(pacMan_vertices), pacMan_vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pacMan_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pacMan_indices), pacMan_indices, GL_STATIC_DRAW);

		//auto vertexSrc = vertexShaderSrc.c_str();
		//auto fragmentSrc = fragmentShaderSrc.c_str();
	}
	unsigned int pacMan_vs, pacMan_fs, pacMan_sp;
	{
		pacMan_vs = glCreateShader(GL_VERTEX_SHADER);
		pacMan_fs = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(pacMan_vs, 1, &pacMan_vs_src, nullptr);
		glCompileShader(pacMan_vs);

		glShaderSource(pacMan_fs, 1, &pacMan_fs_src, nullptr);
		glCompileShader(pacMan_fs);

		pacMan_sp = glCreateProgram();

		glAttachShader(pacMan_sp, pacMan_vs);
		glAttachShader(pacMan_sp, pacMan_fs);

		glBindFragDataLocation(pacMan_sp, 0, "outColor");
		glLinkProgram(pacMan_sp);
		glUseProgram(pacMan_sp);

		glDeleteShader(pacMan_vs);
		glDeleteShader(pacMan_fs);
	}
	//PACMAN: attribute 7 = Position, attribute 8 = Color, attribute 9 = Texture
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

	glEnableVertexAttribArray(8);
	glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

	glEnableVertexAttribArray(9);
	glVertexAttribPointer(9, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));

	/*GHOST CREATION*/
	GLuint ghost_vbo;
	glGenBuffers(1, &ghost_vbo);

	GLuint ghost__ebo;
	glGenBuffers(1, &ghost__ebo);

	glBindBuffer(GL_ARRAY_BUFFER, ghost_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ghost_vertices), ghost_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ghost__ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ghost_indices), ghost_indices, GL_STATIC_DRAW);

	unsigned int ghost_vs, ghost_fs, ghost_sp;

	ghost_vs = glCreateShader(GL_VERTEX_SHADER);
	ghost_fs = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(ghost_vs, 1, &ghost_vs_src, nullptr);
	glCompileShader(ghost_vs);

	glShaderSource(ghost_fs, 1, &ghost_fs_src, nullptr);
	glCompileShader(ghost_fs);

	ghost_sp = glCreateProgram();

	glAttachShader(ghost_sp, ghost_vs);
	glAttachShader(ghost_sp, ghost_fs);

	glBindFragDataLocation(ghost_sp, 0, "outColor");
	glLinkProgram(ghost_sp);
	glUseProgram(ghost_sp);

	glDeleteShader(ghost_vs);
	glDeleteShader(ghost_fs);

	//GHOST: attribute 10 = Position, attribute 11 = Color, attribute 12 = Texture
	glEnableVertexAttribArray(10);
	glVertexAttribPointer(10, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

	glEnableVertexAttribArray(11);
	glVertexAttribPointer(11, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

	glEnableVertexAttribArray(12);
	glVertexAttribPointer(12, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));

	auto texture0 = load_opengl_texture("resources/pacman.png", 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glAlphaFunc(GL_GREATER, 0.1f);


	/**/
	GLint m_x_loc = glGetUniformLocation(pacMan_sp, "m_x");
	GLint m_y_loc = glGetUniformLocation(pacMan_sp, "m_y");
	GLint pacMan_x_loc = glGetUniformLocation(pacMan_sp, "pacMan_x");
	GLint pacMan_y_loc = glGetUniformLocation(pacMan_sp, "pacMan_y");
	GLfloat m_x = 0;
	GLfloat m_y = 3;
	GLfloat pacMan_x = 0;
	GLfloat pacMan_y = 0;
	GLfloat move_x = 0.005f;
	GLfloat move_y = 0.005f;
	GLint g_rot_x_loc = glGetUniformLocation(ghost_sp, "g_rot_x");
	GLint g_rot_y_loc = glGetUniformLocation(ghost_sp, "g_rot_y");
	GLint ghost_x_loc = glGetUniformLocation(ghost_sp, "ghost_x");
	GLint ghost_y_loc = glGetUniformLocation(ghost_sp, "ghost_y");
	GLfloat g_rot_x = 4;
	GLfloat g_rot_y = 0;
	GLfloat ghost_x = 0;
	GLfloat ghost_y = 0;
	GLfloat ghost_move = 0.005f;

	double timer = 0.075;
	double timer_2 = 0;
	double timer_4 = 0;
	double timer_3 = 0.075;
	double delay = 0.033;
	bool upBlock = false;
	bool downBlock = false;
	bool leftBlock = false;
	bool rightBlock = false;
	bool running = true;

	int ghostRightorLeft = (rand() % 10);
	int ghostUpOrDown = (rand() % 10);
	int verticalOrHorizontal = (rand() % 10);
	bool moving = false;
	bool movingUp = false;
	bool movingDown = false;
	bool movingLeft = false;
	bool movingRight = false;
	bool ghostUpBlock = false;
	bool ghostDownBlock = false;
	bool ghostLeftBlock = false;
	bool ghostRightBlock = false;



	//glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	/* G A M E L O O P */
	while (!glfwWindowShouldClose(window) && running)
	{
		double curr_s = glfwGetTime();
		
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f);//Background is dark grey
		glClear(GL_COLOR_BUFFER_BIT);

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, 1);
		};

		/*calculating pacMan's grid location*/
		int pacManGridLocX = (18 * pacMan_max_x) + 15;
		int pacManGridLocy = 18 * (1 - pacMan_max_y);
		int gridLoc = 28 * pacManGridLocy + pacManGridLocX;
		//cout << "x lox: " << pacManGridLocX << "\ty loc: " << pacManGridLocy << "\t loc: " << gridLoc  << "\tvalue: " << level[gridLoc] << endl;

		/*calculating ghost's grid location*/
		int ghostGridLocX = (18 * ghost_max_x) + 15;
		int ghostGridLocY = 18 * (1 - ghost_max_y);
		int ghostGridLoc = 28 * ghostGridLocY + ghostGridLocX;
		//cout << "x lox: " << ghostGridLocX << "\ty loc: " << ghostGridLocY << "\t loc: " << ghostGridLoc << "\tvalue: " << level[ghostGridLoc] << endl;

		bool freeze = true;
		for (int i = 0; i < 500; i++) {
			if (dots_offsets_x[i] > -1.5f) {
				freeze = false;
			}
		}
		if (ghostGridLoc == gridLoc) {
			freeze = true;
		}

		if (freeze) {
			move_y = 0; move_x = 0;
		}

		glBindVertexArray(vao);
		glUseProgram(wall_shaderprogram);

		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 1000);
		glUseProgram(dots_shaderprogram);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL, 500);

		glUseProgram(pacMan_sp);
		if (curr_s > timer && !freeze) {
			glUniform1f(m_x_loc, m_x);
			if (m_x >= 3) {
				m_x = 0;
			}
			else {
				m_x++;
			}
			timer += 0.075;
		}
		glUniform1f(m_y_loc, m_y);
		glUniform1f(pacMan_x_loc, pacMan_x);
		glUniform1f(pacMan_y_loc, pacMan_y);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void*)0);


		/*PacMan collision handler*/
		{
			//Moves dots that have been hit outside of viewport
			if (level[gridLoc] == 0) {
				int index = 0;
				for (int i = 0; i < gridLoc; i++) {
					if (level[i] == 0) {
						index++;
					}
				}

				dots_offsets_x[index] = -2.0f; //delete
				dots_offsets_y[index] = -2.0f; //delete

				glBindBuffer(GL_ARRAY_BUFFER, dot_offsetsBufferID_x); //these four are to be used in the dot intersects thing
				glBufferData(GL_ARRAY_BUFFER, sizeof(dots_offsets_x), dots_offsets_x, GL_STATIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, dot_offsetsBufferID_y);
				glBufferData(GL_ARRAY_BUFFER, sizeof(dots_offsets_y), dots_offsets_y, GL_STATIC_DRAW);
			}

			if (level[gridLoc - 28] == 1) {
				upBlock = true;
			}
			else {
				upBlock = false;
			}
			if (level[gridLoc + 28] == 1) {
				downBlock = true;
			}
			else {
				downBlock = false;
			}
			if (level[gridLoc - 1] == 1) {
				leftBlock = true;

			}
			else {
				leftBlock = false;
			}
			if (level[gridLoc + 1] == 1) {
				rightBlock = true;
			}
			else {
				rightBlock = false;

			}
		}

		/*PACMAN INPUT HANDLER*/
		{

			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			{
				if (curr_s > timer_2 && pacMan_max_y < 1.0f && pacMan_min_y > -1.1f && !upBlock) {
					pacMan_y += move_y; pacMan_min_y += move_y; pacMan_max_y += move_y;
					timer_2 = curr_s + delay;
				}
				else if (curr_s > timer_2 && pacMan_max_y < 1.0f && pacMan_min_y > -1.1f && upBlock) {
					if (level[gridLoc] == 0) {
						pacMan_y += move_y; pacMan_min_y += move_y; pacMan_max_y += move_y;
						timer_2 = curr_s + delay;
					}
				}
				m_y = 0.0f;
			}

			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			{
				if (curr_s > timer_2 && pacMan_max_y < 1.1f && pacMan_min_y > -1.0f && !downBlock) {
					pacMan_y -= move_y; pacMan_max_y -= move_y; pacMan_min_y -= move_y;
					timer_2 = curr_s + delay;
				}
				else if (curr_s > timer_2 && pacMan_max_y < 1.1f && pacMan_min_y > -1.0f && downBlock) {
					if (level[gridLoc] == 0) {
						pacMan_y -= move_y; pacMan_max_y -= move_y; pacMan_min_y -= move_y;
						timer_2 = curr_s + delay;
					}
				}
				m_y = 1.0f;
			}
			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			{
				if (curr_s > timer_2 && pacMan_min_x < 1.1f && pacMan_max_x > -1.0f && !leftBlock) {
					pacMan_x -= move_x; pacMan_max_x -= move_x; pacMan_min_x -= move_x;
					timer_2 = curr_s + delay;

				}
				else if (curr_s > timer_2 && pacMan_min_x < 1.1f && pacMan_max_x > -1.0f && leftBlock) {
					if (level[gridLoc] == 0) {
						pacMan_x -= move_x; pacMan_max_x -= move_x; pacMan_min_x -= move_x;
						timer_2 = curr_s + delay;
					}
				}
				m_y = 2.0f;
			}
			if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			{
				if (curr_s > timer_2 && pacMan_min_x < 1.0f && pacMan_max_x > -1.1f && !rightBlock) {
					pacMan_x += move_x; pacMan_min_x += move_x; pacMan_max_x += move_x;
					timer_2 = curr_s + delay;
				}
				else if (curr_s > timer_2 && pacMan_min_x < 1.0f && pacMan_max_x > -1.1f && rightBlock) {
					if (level[gridLoc] == 0) {
						pacMan_x += move_x; pacMan_min_x += move_x; pacMan_max_x += move_x;
						timer_2 = curr_s + delay;
					}
				}
				m_y = 3.0f;
			}
		}


		/*GHOST MOVEMENT ALGORITHM*/
		{
			if (!moving) {
				movingUp = false; movingDown = false; movingLeft = false; movingRight = false;
				ghostRightorLeft = (rand() % 10);
				ghostUpOrDown = (rand() % 10);
				verticalOrHorizontal = (rand() % 10);
				if (verticalOrHorizontal >= 5) {
					if (ghostUpOrDown >= 5) {
						movingUp = true;
					}
					else {
						movingDown = true;
					}
				}
				else {
					if (ghostRightorLeft >= 5) {
						movingRight = true;
					}
					else {
						movingLeft = true;
					}
				}
				moving = true;
			}

			if (moving) {

				if (movingUp) {
					if (level[ghostGridLoc - 28] == 1) {
						moving = false;  movingUp = false;
					}
					if (curr_s > timer_4 && ghost_max_y < 1.0f && ghost_min_y > -1.1f) {
						ghost_y += move_y; ghost_min_y += move_y; ghost_max_y += move_y;
						timer_4 = curr_s + delay; moving = true; movingUp = true;
					}
				}

				if (movingDown) {
					if (level[ghostGridLoc + 28] == 1) {
						moving = false;  movingDown = false;
					}
					if (curr_s > timer_4 && ghost_max_y < 1.1f && ghost_min_y > -1.0f) {
						ghost_y -= move_y; ghost_max_y -= move_y; ghost_min_y -= move_y;
						timer_4 = curr_s + delay; moving = true; movingDown = true;
					}
				}

				if (movingLeft) {
					if (level[ghostGridLoc - 1] == 1) {
						moving = false;  movingLeft = false;
					}
					if (curr_s > timer_4 && ghost_min_x < 1.1f && ghost_max_x > -1.0f) {
						ghost_x -= move_x; ghost_max_x -= move_x; ghost_min_x -= move_x;
						timer_4 = curr_s + delay; moving = true; movingLeft = true;
					}
				}

				if (movingRight) {
					if (level[ghostGridLoc + 1] == 1) {
						moving = false;  movingRight = false;
					}
					if (curr_s > timer_4 && ghost_min_x < 1.0f && ghost_max_x > -1.1f) {
						ghost_x += move_x; ghost_min_x += move_x; ghost_max_x += move_x;
						timer_4 = curr_s + delay;
						moving = true; movingRight = true;
					}
				}

			}
		}

		glUseProgram(ghost_sp);
		if (curr_s > timer_3 && !freeze) {
			glUniform1f(g_rot_x_loc, g_rot_x);
			if (g_rot_y >= 3) {
				g_rot_y = 0;
			}
			else {
				g_rot_y++;;
			}
			timer_3 += 0.075;
		}
		glUniform1f(g_rot_y_loc, g_rot_y);
		glUniform1f(ghost_x_loc, ghost_x);
		glUniform1f(ghost_y_loc, ghost_y);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void*)0);

		glfwSwapBuffers(window); //always at the bottom of render loop
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;

}

/*Shader creation program -- heavily inspired by lab04*/
GLuint createShader(const std::string& vs_source, const std::string& fs_source) {

	auto vertexSource = vs_source.c_str();
	auto fragmentSource = fs_source.c_str();

	auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertexSource, NULL);
	glCompileShader(vertex_shader);

	auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragmentSource, NULL);
	glCompileShader(fragment_shader);

	auto shader_program = glCreateProgram();
	glBindFragDataLocation(shader_program, 0, "FragColor");
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);


	glLinkProgram(shader_program);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	return shader_program;
}

/*Texture loader functuin -- heavily inspired by lab04*/
GLuint load_opengl_texture(const std::string& filepath, GLuint slot)
{

	/** Image width, height, bit depth */
	int w, h, bpp;

	/*loading the picture*/
	auto pixels = stbi_load(filepath.c_str(), &w, &h, &bpp, STBI_rgb_alpha);
	GLuint tex;
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	/** texture parameters */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/** deleting pixels */
	if (pixels) stbi_image_free(pixels);

	return tex;
}

/*This function loads the 0s, 1s and 2s from the textfile and into an array*/
void loadLevel() {
	/* READING THE FILE FOR THE MAP */
	std::ifstream levelFile;

	level[0] = 99; //first element in the array won't be used, setting it to 99
	int number;
	levelFile.open("resources/level0.txt");
	if (levelFile.is_open()) {
		/* For-loop that TODO: WRITE WHAT*/
		for (int i = 1; i <= 1008; i++) {
			levelFile >> number;
			level[i] = number;
		}
	}
	levelFile.close();

	for (int i = 0; i <= 1008; i++) {
		if (level[i] == 1) {
			wallCounter++;
		}
	}
	std::cout << "Number of walls is: " << wallCounter << "\n";
}


