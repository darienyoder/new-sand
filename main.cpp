
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <math.h>

#include "sandsim.hpp"
#include "input.hpp"

//SDL_Window* window;
//SDL_Renderer* renderer;
GLFWwindow* window;

bool game_is_running = false;
float time_since_last_frame = 0.0;
int target_fps = 60;
int tps = 60;
int t = 0;

int window_size[2];
int mouse_position[2] { 0, 0 };

SandSim sim(200, 120);
InputManager input;

int mouse_action = 1;

bool run_sim = true;

int tile_size = 5;

const int window_margin = 10;
const int button_size = 50;
const int button_count = 13;
const int button_rows = 10;

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
}
)";

// Fragment Shader source code
const char* fragmentShaderSource = R"(
#version 330 core

layout(origin_upper_left) in vec4 gl_FragCoord;
out vec4 color;

uniform isampler2D materialTexture;

uniform vec2 window_size;

void main() {

	ivec2 coords = (ivec2(gl_FragCoord.xy) - ivec2(10, 10)) / 5;

	color = texture(materialTexture, coords.yx / vec2(200, 120));

/* ////////////////////
	if (coords.x >= 0 && coords.x < 200 && coords.y >= 0 && coords.y < 120)
	{
		if ( (coords.x + coords.y) % 2 == 0 )
			color = vec4(0, 1, 0, 1);
		else
			color = vec4(1, 0, 0, 1);
	}
	else
		color = vec4(1, 0, 1, 1);
*/ ///////////////////

}
)";

unsigned int shaderProgram;
unsigned int VAO, VBO;

void initialize_window()
{
	// Initialize GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return;
	}

	window_size[0] = sim.x_size * tile_size + window_margin * 2 + (button_size + window_margin) * (((button_count - 1) / button_rows) + 1);
	window_size[1] = sim.y_size * tile_size + window_margin * 2;
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(
		window_size[0],
		window_size[1],
		"Sand",
		nullptr,
		nullptr
	);

	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = GL_TRUE; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return;
	}

	game_is_running = true;
}

void cleanup()
{
	
}

void key_callback(GLFWwindow* window_, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window_, 1);
}

static void cursor_position_callback(GLFWwindow* window_, double xpos, double ypos)
{
	mouse_position[0] = xpos;
	mouse_position[1] = ypos;
}

void on_click(void);

void mouse_button_callback(GLFWwindow* window_, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		on_click();
}

void setup()
{
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	input.set_monitering(SDLK_ESCAPE);

	// Compile and link shaders
	unsigned int vertexShader, fragmentShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
	glCompileShader(vertexShader);

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
	glCompileShader(fragmentShader);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Delete the shaders as they're linked into our program now and no longer necessary
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);



	float vertices[] = {
		-1.0f + float(window_margin + sim.x_size * tile_size) / window_size[0] * 2.0f, -1.0f + float(window_margin) / window_size[1] * 2.0f, 0.0f,  // top right
		-1.0f + float(window_margin + sim.x_size * tile_size) / window_size[0] * 2.0f,  1.0f - float(window_margin) / window_size[1] * 2.0f, 0.0f,  // bottom right
		-1.0f + float(window_margin) / window_size[0] * 2.0f,  1.0f - float(window_margin) / window_size[1] * 2.0f, 0.0f,  // bottom left
		-1.0f + float(window_margin) / window_size[0] * 2.0f, -1.0f + float(window_margin) / window_size[1] * 2.0f, 0.0f   // top left 
	};

	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,  // first Triangle
		1, 2, 3   // second Triangle
	};
	unsigned int EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void screen_to_sim(int x, int y, int output[])
{
	output[0] = (x - window_margin) / tile_size;
	output[1] = (y - window_margin) / tile_size;
}

void place_tile(int new_tile, int state = 0)
{
	int tile[2];
	screen_to_sim(mouse_position[0], mouse_position[1], tile);

	const int radius = 10;

	for (int x = -radius; x < radius; x++)
	for (int y = -radius; y < radius; y++)
		if (x*x + y*y < radius*radius)
		{
			sim.set_tile(tile[0] + x, tile[1] + y, new_tile);
			sim.make_active(tile[0] + x, tile[1] + y);
		}
}

void get_input()
{
	glfwPollEvents();

	input.update();
}

//	if (input.mouse_down)
//	{

void on_click()
{
	if (mouse_position[0] > window_margin && mouse_position[0] < window_margin + sim.x_size * tile_size && mouse_position[1] > window_margin && mouse_position[1] < window_margin + sim.y_size * tile_size)
		switch (mouse_action)
		{
		case 1:
			place_tile(SAND);
			break;
				
		case 2:
			place_tile(WATER);
			break;

		case 3:
			place_tile(ICE);
			break;

		case 4:
			place_tile(STEAM);
			break;
				
		case 5:
			place_tile(DIRT);
			break;

		case 6:
			place_tile(STONE);
			break;

		case 7:
			place_tile(LAVA);
			break;
			/*
		case 8:
			for (int x = -5; x < 6; ++x)
				for (int y = -5; y < 6; ++y)
					sim.heat_tile(input.mouse_x / sim.tile_size + x, input.mouse_y / sim.tile_size + y, 5);
			break;
		case 9:
			for (int x = -5; x < 6; ++x)
				for (int y = -5; y < 6; ++y)
					sim.heat_tile(input.mouse_x / sim.tile_size + x, input.mouse_y / sim.tile_size + y, -5);
			break;
			*/

			// COLUMN 2

		case 10:
			place_tile(OIL);
			break;

		case 11:
			place_tile(ACID);
			break;
				
		}
	else// if (input.mouse_x > window_margin * 2 + sim.x_size * sim.tile_size && input.mouse_x < window_margin * 2 + sim.x_size * sim.tile_size + button_size)
	{
		for (int i = 0; i < button_count; ++i)
			if (input.mouse_y > window_margin + (window_margin + button_size) * (i % button_rows) && input.mouse_y < window_margin + (window_margin + button_size) * (i % button_rows) + button_size
				&& input.mouse_x > window_margin * 2 + sim.x_size * tile_size + (button_size + window_margin) * (i / button_rows) && input.mouse_x < window_margin * 2 + sim.x_size * tile_size + (button_size + window_margin) * (i / button_rows) + button_size)
			{
				if (i == 0)
					sim.clear();
				else
					mouse_action = i;
				break;
			}
	}

}
	//else if (input.right_mouse_down)
	//{
	//	place_tile(EMPTY);
	//}
	

	//if (input.is_pressed(SDLK_ESCAPE))
	//	game_is_running = false;

void update()
{
	++t;

	int time_to_wait = (1000.0f / target_fps) - (SDL_GetTicks() - time_since_last_frame);

	if (time_to_wait > 0 && time_to_wait <= (1000.0f / target_fps))
		SDL_Delay(time_to_wait);


	float delta = (SDL_GetTicks() - time_since_last_frame) / 1000.0f;
	time_since_last_frame = SDL_GetTicks();

	glfwGetWindowSize(window, &window_size[0], &window_size[1]);

	if (run_sim && t % (target_fps / tps) == 0)
	{
		sim.update();
	}
}

void draw_buttons()
{
	/*
	const int r[13] = { 255, 255,   0,  50, 200, 128, 50, 252, 255,  50,
								  192,   0, 175, };

	const int g[13] = { 0, 255,   0, 150, 200,  64, 50, 157,   0, 150,
								  192, 175,   0, };

	const int b[13] = { 0,   0, 255, 255, 200,   0, 50,  47,   0, 255,
								  192,   0,   0, };

	for (int i = 0; i < button_count; ++i)
	{
		SDL_Rect button_rect;
		button_rect.x = sim.x_size * tile_size + window_margin * 2 + (button_size + window_margin) * (i / button_rows);
		button_rect.y = window_margin + (button_size + window_margin) * (i % button_rows);
		button_rect.w = button_size;
		button_rect.h = button_size;

		if (mouse_action == i)
			SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
		else
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(renderer, &button_rect);

		button_rect.x += 5;
		button_rect.y += 5;
		button_rect.w -= 10;
		button_rect.h -= 10;

		SDL_SetRenderDrawColor(renderer, r[i], g[i], b[i], 255);
		SDL_RenderFillRect(renderer, &button_rect);

		if (i == 0)
		{
			const int scale = 3;
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderSetScale(renderer, scale, scale);
			SDL_RenderDrawLine(renderer, (button_rect.x + 5) / scale, (button_rect.y + 5) / scale, (button_rect.x + button_rect.w - 5) / scale, (button_rect.y + button_rect.h - 5) / scale);
			SDL_RenderDrawLine(renderer, (button_rect.x + 5) / scale, (button_rect.y + button_rect.h - 5) / scale, (button_rect.x + button_rect.w - 5) / scale, (button_rect.y + 5) / scale);
			SDL_RenderSetScale(renderer, 1, 1);
		}
		else if (i == 8)
		{
			const int scale = 3;
			SDL_SetRenderDrawColor(renderer, 252, 157, 47, 255);
			SDL_RenderSetScale(renderer, scale, scale);
			SDL_RenderDrawLine(renderer, (button_rect.x + button_rect.w / 2) / scale, (button_rect.y + 5) / scale, (button_rect.x + button_rect.w / 2) / scale, (button_rect.y + button_rect.h - 5) / scale);
			SDL_RenderDrawLine(renderer, (button_rect.x + button_rect.w / 2) / scale, (button_rect.y + 5) / scale, (button_rect.x + 5) / scale, (button_rect.y + button_rect.h / 2) / scale);
			SDL_RenderDrawLine(renderer, (button_rect.x + button_rect.w / 2) / scale, (button_rect.y + 5) / scale, (button_rect.x + button_rect.w - 5) / scale, (button_rect.y + button_rect.h / 2) / scale);
			SDL_RenderSetScale(renderer, 1, 1);
		}
		else if (i == 9)
		{
			const int scale = 3;
			SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
			SDL_RenderSetScale(renderer, scale, scale);
			SDL_RenderDrawLine(renderer, (button_rect.x + button_rect.w / 2) / scale, (button_rect.y + 5) / scale, (button_rect.x + button_rect.w / 2) / scale, (button_rect.y + button_rect.h - 5) / scale);
			SDL_RenderDrawLine(renderer, (button_rect.x + button_rect.w / 2) / scale, (button_rect.y + button_rect.h - 5) / scale, (button_rect.x + 5) / scale, (button_rect.y + button_rect.h / 2) / scale);
			SDL_RenderDrawLine(renderer, (button_rect.x + button_rect.w / 2) / scale, (button_rect.y + button_rect.h - 5) / scale, (button_rect.x + button_rect.w - 5) / scale, (button_rect.y + button_rect.h / 2) / scale);
			SDL_RenderSetScale(renderer, 1, 1);
		}
	}
	*/
}

/*
void draw_sim()
{

	// Black background
	SDL_Rect background = { 10, 10, tile_size * sim.x_size, tile_size * sim.y_size };
	SDL_SetRenderDrawColor(renderer, 00, 00, 00, 255);
	SDL_RenderFillRect(renderer, &background);

	int r = 0, g = 0, b = 0;
	float value = 1.0f;

	for (int x = 0; x < sim.x_size; ++x)
	{
		for (int y = 0; y < sim.y_size; ++y)
		{
			if (sim.tiles[x][y]->material == EMPTY)
				continue;

			//r = sim.tiles[x][y]->r;
			//g = sim.tiles[x][y]->g;
			//b = sim.tiles[x][y]->b;

			float value = (((x + 12) * (x + 12) + x^y + (y + 35) * (y + 35) * 2) % 100) / 100.0;
			value = value * 0.25 + 0.75;

			switch (sim.tiles[x][y]->material)
			{
			case EMPTY:
				r = 255;
				g = 255;
				b = 255;
				break;

			case SAND:
				r = 255 * value;
				g = 230 * value;
				b = 128 * value;
				break;

			case WATER:
				if (!sim.is_tile_empty(x, y + 1) && ((sim.is_tile_empty(x + 1, y) && sim.is_tile_empty(x - 1, y)) || (sim.is_tile_empty(x + 2, y) && sim.is_tile_empty(x - 2, y))))
					continue;

				r = 0;
				g = 50;
				b = 255;
				// White border
				if (sim.get_tile(x, y - 1).material == EMPTY && sim.get_tile(x, y + 1).material != EMPTY)
				{
					r = 200;
					g = 200;
					b = 255;
				}
				break;

			case ICE:
				r = 50;
				g = 150;
				b = 255;

				if (
					//(x + y) % 2 == 0 && 
					x % 5 == y % 5
					&& ((x / 7) + (y / 7)) % 2 == 0
					)
				{
					r = 255;
					g = 255;
					b = 255;
				}

				break;

			case STEAM:
				r = 255 * (value * 0.5 + 0.5);
				g = 255 * (value * 0.5 + 0.5);
				b = 255 * (value * 0.5 + 0.5);
				break;

			case DIRT:
				r = 128 * value;
				g = 64 * value;
				b = 0;
				break;

			case STONE:
				r = 50 * value;
				g = 50 * value;
				b = 50 * value;
				break;

			case LAVA:
				r = 252 * value;
				g = 157 * value;
				b = 47 * value;
				break;

			case OIL:
				r = 155;
				g = 118;
				b = 17;
				break;

			case ACID:
				r = 50;
				g = 175;
				b = 0;
				break;
			}
			

			SDL_Rect box = { x * tile_size + 10, y * tile_size + 10, tile_size, tile_size };
			SDL_SetRenderDrawColor(renderer, r * 1, g * 1, b * 1, 255);
			SDL_RenderFillRect(renderer, &box);
		}
	}
}
*/

void draw_sim2()
{
	auto materialMatrix = sim.get_texture_data();

	std::vector<int> flatData;
	for (const auto& row : materialMatrix) {
		flatData.insert(flatData.end(), row.begin(), row.end());
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Upload the texture data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, sim.x_size, sim.y_size, 0, GL_RED_INTEGER, GL_INT, flatData.data());

	// Generate mipmaps (optional, depending on your use case)
	glGenerateMipmap(GL_TEXTURE_2D);

	// Unbind the texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Use the shader program
	glUseProgram(shaderProgram);

	// Bind the texture to a texture unit
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Set the sampler uniform in the shader
	glUniform1i(glGetUniformLocation(shaderProgram, "materialTexture"), 0);
	
	float fl_winsize[2] = { float(sim.x_size * tile_size), float(sim.y_size * tile_size) };
	glUniform2fv(glGetUniformLocation(shaderProgram, "window_size"), 1, fl_winsize);

	////////////////////////////////////////////////////////////////////////

	// Draw the triangle
	//glUseProgram(shaderProgram);
	glBindVertexArray(VAO); // Bind the VAO
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0); // Unbind the VAO
}

void draw()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f); // Set clear color to a dark teal
	glClear(GL_COLOR_BUFFER_BIT);

	/*
	int size[2];
	SDL_GetWindowSize(window, &size[0], &size[1]);
	if ((float)size[0] / (float)size[1] < (float)sim.x_size / (float)sim.y_size)
		tile_size = size[0] / sim.x_size;
	else
		tile_size = size[1] / sim.y_size;
	*/

	draw_sim2();

	draw_buttons();

	// Swap front and back buffers
	glfwSwapBuffers(window);
	// Poll for and process events
	glfwPollEvents();
}

int main(int argc, char* argv[])
{
	initialize_window();

	setup();

	while (!glfwWindowShouldClose(window))
	{
		get_input();
		update();
		draw();
	}

	cleanup();

	return 0;
}