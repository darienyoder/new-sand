#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <chrono>
#include <math.h>

#include <iostream>

#include "sandsim.hpp"
#include "input.hpp"

GLFWwindow* window;

bool game_is_running = false;
float time_since_last_frame = 0.0;
int target_fps = 60;
int tps = 60;
auto t = std::chrono::high_resolution_clock::now();
auto last_sim_update = t;

int window_size[2];

SandSim sim(200, 120);
InputManager* input = InputManager::getInstance();

int mouse_action = 1;

bool run_sim = true;

int tile_size = 8;

const int window_margin = 10;
const int button_size = 50;
const int button_count = 13;
const int button_rows = 10;

float camera_position[2] = { -10, -10 };
float camera_zoom = 1.0;

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

uniform int tile_size;
uniform vec2 window_size;

uniform vec2 camera_position;
uniform float camera_zoom;

int get_material(vec2 coords)
{
	if (coords.x < 0.0 || coords.y < 0.0 || coords.x >= window_size.x / tile_size || coords.y >= window_size.y / tile_size)
		return -1;
	return texture(materialTexture, coords.yx / window_size.yx * tile_size).r;
}

vec3 get_material_color(int material, float value)
{
	if (material == -1)
		return vec3(0.1);
	if (material == 0)
		return vec3(0.0);
	else if (material == 1) // SAND
		return vec3(1.0, 0.9, 0.5) * value;
	else if (material == 2) // WATER
		return vec3(0.0, 0.2, 1.0);
	else if (material == 3) // ICE
		return vec3(0.2, 0.6, 1.0);
	else if (material == 4) // STEAM
		return vec3(0.9, 0.9, 0.9) * 0.0;
	else if (material == 5) // DIRT
		return vec3(0.5, 0.25, 0.0) * value;
	else if (material == 6) // STONE
		return vec3(0.2, 0.2, 0.2) * value;
	else if (material == 7) // LAVA
		return vec3(0.9, 0.6, 0.2) * value;
	else if (material == 8) // OIL
		return vec3(0.6, 0.4, 0.1);
	else if (material == 9) // ACID
		return vec3(0.4, 0.7, 0.0);
	else if (material == 10) // FIRE
		return vec3(0.9, 0.4, 0.1);
	else if (material == 11) // SMOKE
		return vec3(0.3, 0.3, 0.3) * 0.0;
	else if (material == 12) // WOOD
		return vec3(0.427, 0.275, 0.012) * value;

	return vec3(1.0, 0.2, 1.0);
}

vec2 screen_to_game(vec2 coords)
{
	return ivec2( vec2(-window_size.xy / 2.0 + coords.xy) / tile_size / camera_zoom + camera_position.xy);
}

vec3 first_pass(vec2 screen_coords, float value)
{
	vec2 coords = screen_to_game(screen_coords.xy);
	int material = get_material(coords);
	vec3 clr = get_material_color(material, value);

	if (material == -1)
		return clr;

	for (int x = -5; x < 6; x++)
	for (int y = -5; y < 6; y++)
	{
		if (x*x + y*y <= 25)
		{
			int neighbor = get_material(coords + vec2(x, y));
			
			float scale = 0.0; // Steam
			if ((material == 0 || material == 4 || material == 11) && neighbor == 4)
				scale += 0.015;
			clr = vec3(0.9) * scale + clr * (1.0 - scale);

			scale = 0.0; // Smoke
			if ((true || material == 0 || material == 11 || material == 4) && neighbor == 11)
				scale += 0.03;
			clr = vec3(0.3) * scale + clr * (1.0 - scale);

			if (neighbor == 7) // Lava
				clr += vec3(0.02, 0.0, 0.0);

			if (neighbor == 10) // Fire
				clr += vec3(0.01, 0.0, 0.0);// * (1.0 - (x*x + y*y) / 25.0);

			if (material == 2 && neighbor == 0 && x == 0 && y == -1)
				clr = vec3(1.0, 1.0, 1.0);
		}
	}
	return clr;
}

void main()
{

	vec2 coords = screen_to_game(gl_FragCoord.xy);

	int material = get_material(coords);

	float value = ( int(pow(int(coords.x) % 50 + 50, (int(coords.y) % 20 + 20) * 0.1)) % 100) / 100.0;
	value = value * 0.25 + 0.75;

	color.rgb = first_pass(gl_FragCoord.xy, value);
	return;

	// Second Pass
	
	bool surrounded = (
		get_material(coords + vec2(1, 0)) == material
		&& get_material(coords + vec2(-1, 0)) == material
		&& get_material(coords + vec2(0, 1)) == material
		&& get_material(coords + vec2(0, -1)) == material
		&& (material != 2 || get_material(coords + vec2(0, -2)) == material)
	);
	
	if (surrounded)
		color.rgb = first_pass(gl_FragCoord.xy, value);
	else
	{
		int count = 0;
		float hardness = (material == 1) ? 2.0 : 4.0;
		for (int x = -2; x < 3; x++)
		for (int y = -2; y < 3; y++)
		//	if (x*x + y*y <= 4)
		{
			color.rgb += first_pass(gl_FragCoord.xy + vec2(x, y) * tile_size / hardness, value);
			count++;
		}
		color.rgb /= count;
	}
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

	window_size[0] = sim.x_size * tile_size + window_margin * 2;// +(button_size + window_margin) * (((button_count - 1) / button_rows) + 1);
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

void setup()
{
	input->setup(window);

	camera_position[0] = sim.x_size * 0.5;
	camera_position[1] = sim.y_size * 0.5;
	camera_zoom = 0.95;

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

	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
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
	output[0] = (-window_size[0] / 2.0 + x) / tile_size / camera_zoom + camera_position[0];
	output[1] = (-window_size[1] / 2.0 + y) / tile_size / camera_zoom + camera_position[1];
}

void place_tile(int new_tile, int state = 0)
{
	int tile[2];
	screen_to_sim(input->mouse_x, input->mouse_y, tile);

	const int radius = 5;

	for (int x = -radius; x < radius; x++)
	for (int y = -radius; y < radius; y++)
		if (x*x + y*y < radius*radius)
		{
			sim.set_tile(tile[0] + x, tile[1] + y, new_tile);
			sim.make_active(tile[0] + x, tile[1] + y);
		}
}

void on_click(void);

void get_input()
{
	input->update();

	if (input->mouse_down)
		on_click();
	else if (input->right_mouse_down)
		place_tile(EMPTY);

	if (input->is_pressed(GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(window, 1);

	
	if (input->is_pressed(GLFW_KEY_LEFT))
	{
		camera_position[0] -= 3 / camera_zoom;
	}
	if (input->is_pressed(GLFW_KEY_RIGHT))
	{
		camera_position[0] += 3 / camera_zoom;
	}
	if (input->is_pressed(GLFW_KEY_UP))
	{
		camera_position[1] -= 3 / camera_zoom;
	}
	if (input->is_pressed(GLFW_KEY_DOWN))
	{
		camera_position[1] += 3 / camera_zoom;
	}
	if (input->is_pressed(GLFW_KEY_EQUAL))
	{
		camera_zoom *= 1.01;
	}
	if (input->is_pressed(GLFW_KEY_MINUS))
	{
		camera_zoom /= 1.01;
	}


	if (input->is_pressed(GLFW_KEY_GRAVE_ACCENT))
	{
		mouse_action = (mouse_action + 1) % button_count;
		if (mouse_action == 0)
			mouse_action++;
	}

	if (input->is_pressed(GLFW_KEY_1))
	{
		mouse_action = 1;
	}

	if (input->is_pressed(GLFW_KEY_2))
	{
		mouse_action = 2;
	}

	if (input->is_pressed(GLFW_KEY_3))
	{
		mouse_action = 4;
	}

	if (input->is_pressed(GLFW_KEY_4))
	{
		mouse_action = 5;
	}

	if (input->is_pressed(GLFW_KEY_5))
	{
		mouse_action = 6;
	}

	if (input->is_pressed(GLFW_KEY_6))
	{
		mouse_action = 7;
	}

	if (input->is_pressed(GLFW_KEY_7))
	{
		mouse_action = 10;
	}

	if (input->is_pressed(GLFW_KEY_8))
	{
		mouse_action = 11;
	}

	if (input->is_pressed(GLFW_KEY_9))
	{
		mouse_action = 12;
	}

	if (input->is_pressed(GLFW_KEY_0))
	{
		mouse_action = 13;
	}
}

void on_click()
{
	if (input->mouse_x > window_margin && input->mouse_x < window_margin + sim.x_size * tile_size && input->mouse_y > window_margin && input->mouse_y < window_margin + sim.y_size * tile_size)
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
					sim.heat_tile(input->mouse_x / sim.tile_size + x, input->mouse_y / sim.tile_size + y, 5);
			break;
		case 9:
			for (int x = -5; x < 6; ++x)
				for (int y = -5; y < 6; ++y)
					sim.heat_tile(input->mouse_x / sim.tile_size + x, input->mouse_y / sim.tile_size + y, -5);
			break;
			*/

			// COLUMN 2

		case 10:
			place_tile(OIL);
			break;

		case 11:
			place_tile(ACID);
			break;

		case 12:
			place_tile(FIRE);
			break;

		case 13:
			place_tile(WOOD);
			break;
				
		}
	return;
	// else if (input->mouse_x > window_margin * 2 + sim.x_size * sim.tile_size && input->mouse_x < window_margin * 2 + sim.x_size * sim.tile_size + button_size)
	{
		for (int i = 0; i < button_count; ++i)
			if (input->mouse_y > window_margin + (window_margin + button_size) * (i % button_rows) && input->mouse_y < window_margin + (window_margin + button_size) * (i % button_rows) + button_size
				&& input->mouse_x > window_margin * 2 + sim.x_size * tile_size + (button_size + window_margin) * (i / button_rows) && input->mouse_x < window_margin * 2 + sim.x_size * tile_size + (button_size + window_margin) * (i / button_rows) + button_size)
			{
				if (i == 0)
					sim.clear();
				else
					mouse_action = i;
				break;
			}
	}

}

float compare_times(std::chrono::high_resolution_clock::time_point t1, std::chrono::high_resolution_clock::time_point t2)
{
	return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000000.0;
}

void update()
{
	auto new_time = std::chrono::high_resolution_clock::now();
	float delta = compare_times(t, new_time);
	t = new_time;

	glfwGetWindowSize(window, &window_size[0], &window_size[1]);

	if (run_sim && compare_times(last_sim_update, new_time) > 1.0 / tps)
	{
		last_sim_update = new_time;
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

void draw_sim2()
{
	auto materialMatrix = sim.get_texture_data();

	//GLuint textureID;
	//glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Upload the texture data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32I, sim.y_size, sim.x_size, 0, GL_RG_INTEGER, GL_INT, materialMatrix.data());

	// Generate mipmaps (optional, depending on your use case)
	glGenerateMipmap(GL_TEXTURE_2D);

	// Unbind the texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Use the shader program
	glUseProgram(shaderProgram);

	// Bind the texture to a texture unit
	glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, textureID);

	// Set the sampler uniform in the shader
	glUniform1i(glGetUniformLocation(shaderProgram, "materialTexture"), 0);
	
	float fl_winsize[2] = { float(sim.x_size * tile_size), float(sim.y_size * tile_size) };
	glUniform2fv(glGetUniformLocation(shaderProgram, "window_size"), 1, fl_winsize);
	glUniform2fv(glGetUniformLocation(shaderProgram, "camera_position"), 1, camera_position);
	glUniform1i(glGetUniformLocation(shaderProgram, "tile_size"), tile_size);
	glUniform1f(glGetUniformLocation(shaderProgram, "camera_zoom"), camera_zoom);

	////////////////////////////////////////////////////////////////////////

	// Draw the triangle
	//glUseProgram(shaderProgram);
	glBindVertexArray(VAO); // Bind the VAO
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0); // Unbind the VAO

	materialMatrix.~vector();
	//textureID.~GLuint();
}

void draw()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f); // Set clear color to a dark teal
	glClear(GL_COLOR_BUFFER_BIT);

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