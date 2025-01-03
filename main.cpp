#include <chrono>
#include <math.h>

#include <iostream>

#include "drawing.hpp"
#include "sandsim.hpp"
#include "input.hpp"

bool game_is_running = false;
float time_since_last_frame = 0.0;
int target_fps = 60;
int tps = 60;
auto t = std::chrono::high_resolution_clock::now();
auto last_sim_update = t;
auto last_draw = t;

SandSim sim(200, 120);
InputManager* input = InputManager::getInstance();
Canvas* canvas;

int mouse_action = 1;

bool run_sim = true;

int tile_size = 1;

const int window_margin = 10;
const int button_min_size = 70;
int button_size = 70;
const int button_count = 20;
int button_rows = 10;

float camera_position[2] = { -10, -10 };
float camera_zoom = 1.0;

unsigned int VAO, VBO, EBO;

void cleanup()
{
	
}

void setup()
{
	canvas = Canvas::getInstance();

	input->setup(canvas->window);

	camera_position[0] = sim.x_size * 0.5;
	camera_position[1] = sim.y_size * 0.5;
	camera_zoom = std::min(canvas->size.x / sim.x_size, canvas->size.y / sim.y_size);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Bind the VAO
	glBindVertexArray(VAO);

	// Bind and set vertex buffer(s)
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 3, nullptr, GL_DYNAMIC_DRAW); // Allocate space for vertices

	// Bind and set index buffer(s)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, nullptr, GL_DYNAMIC_DRAW); // Allocate space for indices

	// Set vertex attribute pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Unbind the VAO
	glBindVertexArray(0);
}

void screen_to_sim(float x, float y, float output[])
{
	output[0] = float(-canvas->size.x / 2.0 + x) / camera_zoom + camera_position[0];
	output[1] = float(-canvas->size.y / 2.0 + y) / camera_zoom + camera_position[1];
}

void place_tile(int new_tile, int state = 0)
{
	float tile[2];
	screen_to_sim(input->mouse_x, input->mouse_y, tile);

	int radius = 3;
	if (mouse_action == 15) radius = 1;

	for (int x = -radius; x < radius; x++)
	for (int y = -radius; y < radius; y++)
		if (x*x + y*y < radius*radius)
		{
			sim.set_tile(tile[0] + x, tile[1] + y, new_tile);
			sim.make_active(tile[0] + x, tile[1] + y);
			sim.make_chunk_active((tile[0] + x) / sim.chunk_size, (tile[1] + y) / sim.chunk_size);
			//if (Powder* pow = dynamic_cast<Powder*>(&sim.get_tile(tile[0] + x, tile[1] + y)))
			//	sim.launch(tile[0] + x, tile[1] + y, 0, 2);
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
		glfwSetWindowShouldClose(canvas->window, 1);

	if (input->is_pressed(GLFW_KEY_SPACE))
	{
		run_sim = !run_sim;
	}

	
	if (input->is_pressed(GLFW_KEY_LEFT))
	{
		camera_position[0] -= 0.01 / camera_zoom;
	}
	if (input->is_pressed(GLFW_KEY_RIGHT))
	{
		camera_position[0] += 0.01 / camera_zoom;
	}
	if (input->is_pressed(GLFW_KEY_UP))
	{
		camera_position[1] -= 0.01 / camera_zoom;
	}
	if (input->is_pressed(GLFW_KEY_DOWN))
	{
		camera_position[1] += 0.01 / camera_zoom;
	}
	if (input->is_pressed(GLFW_KEY_EQUAL))
	{
		camera_zoom *= 1.0001;
	}
	if (input->is_pressed(GLFW_KEY_MINUS))
	{
		camera_zoom /= 1.0001;
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
	if (input->mouse_x < canvas->size.x - window_margin - (button_size + window_margin) * (((button_count - 1) / button_rows) + 1))//(input->mouse_x > window_margin && input->mouse_x < canvas->size.x - (button_size + window_margin) * (((button_count - 1) / button_rows) + 1) && input->mouse_y > window_margin && input->mouse_y < window_margin + sim.y_size * tile_size)
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

		case 14:
			if (input->just_click())
			{
				float mouse_coords[2];
				screen_to_sim(input->mouse_x, input->mouse_y, mouse_coords);
				sim.explode(mouse_coords[0], mouse_coords[1], 65);
			}
			break;

		case 15:
			if (input->just_click())
				place_tile(FIREWORK);
			break;
				
		}
	
	// if (input->mouse_x > window_margin * 2 + sim.x_size * tile_size && input->mouse_x < window_margin * 2 + sim.x_size * tile_size + button_size)
	if (input->just_click())
	{
		for (int i = 0; i < button_count; ++i)
			if (input->mouse_y > window_margin + (window_margin + button_size) * (i % button_rows) && input->mouse_y < window_margin + (window_margin + button_size) * (i % button_rows) + button_size
				&& input->mouse_x > canvas->size.x - (button_size + window_margin) * (((button_count - 1) / button_rows) + 1) + (button_size + window_margin) * (i / button_rows) && input->mouse_x < canvas->size.x - (button_size + window_margin) * (((button_count - 1) / button_rows) + 1) + (button_size + window_margin) * (i / button_rows) + button_size)
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

	glfwGetWindowSize(canvas->window, &canvas->size.x, &canvas->size.y);
	glViewport(0, 0, canvas->size.x, canvas->size.y);
	glScissor(0, 0, canvas->size.x, canvas->size.y);

	if (run_sim && compare_times(last_sim_update, new_time) > 1.0 / tps)
	{
		last_sim_update = new_time;
		sim.update();
	}
}

void draw_buttons()
{
	const int r[13] = { 255, 255,   0,  50, 200, 128, 50, 252, 255,  50,
								  192,   0, 175, };

	const int g[13] = { 0, 255,   0, 150, 200,  64, 50, 157,   0, 150,
								  192, 175,   0, };

	const int b[13] = { 0,   0, 255, 255, 200,   0, 50,  47,   0, 255,
								  192,   0,   0, };

	if (canvas->size.y)
	{
		button_rows = (canvas->size.y - window_margin) / (button_min_size + window_margin);
		button_size = (canvas->size.y - window_margin) / button_rows - window_margin;
	}

	for (int i = 0; i < button_count; ++i)
	{
		int rect[4] = {
			canvas->size.x - (button_size + window_margin) * (((button_count - 1) / button_rows) + 1) + (button_size + window_margin) * (i / button_rows), //sim.x_size * tile_size + window_margin * 2 + (button_size + window_margin) * (i / button_rows),
			window_margin + (button_size + window_margin) * (i % button_rows),
			button_size,
			button_size,
		};
		
		canvas->draw_rect(rect[0], rect[1], rect[2], rect[3], 200 * int(i == mouse_action), 200 * int(i == mouse_action), 200 * int(i == mouse_action), VAO, VBO, EBO);

		rect[0] += 5;
		rect[1] += 5;
		rect[2] -= 10;
		rect[3] -= 10;

		canvas->draw_rect(rect[0], rect[1], rect[2], rect[3], r[i] / 255.0, g[i] / 255.0, b[i] / 255.0, VAO, VBO, EBO);

		/*
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
		*/
	}
}

void draw_sim2()
{
	float vertices[] = {
		 1.0f - float((button_size + window_margin) * (((button_count - 1) / button_rows) + 1) + window_margin) / canvas->size.x * 2.0f, -1.0f + float(window_margin) / canvas->size.y * 2.0f, 0.0f,  // top right
		 1.0f - float((button_size + window_margin) * (((button_count - 1) / button_rows) + 1) + window_margin) / canvas->size.x * 2.0f,  1.0f - float(window_margin) / canvas->size.y * 2.0f, 0.0f,  // bottom right
		-1.0f + float(window_margin) / canvas->size.x * 2.0f,  1.0f - float(window_margin) / canvas->size.y * 2.0f, 0.0f,  // bottom left
		-1.0f + float(window_margin) / canvas->size.x * 2.0f, -1.0f + float(window_margin) / canvas->size.y * 2.0f, 0.0f   // top left 
	};

	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

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
	glUseProgram(TILE_SHADER);

	// Bind the texture to a texture unit
	glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, textureID);

	// Set the sampler uniform in the shader
	glUniform1i(glGetUniformLocation(TILE_SHADER, "materialTexture"), 0);
	
	float fl_winsize[2] = { float(canvas->size.x), float(canvas->size.y) };
	glUniform2fv(glGetUniformLocation(TILE_SHADER, "window_size"), 1, fl_winsize);
	glUniform2f(glGetUniformLocation(TILE_SHADER, "sim_size"), sim.x_size, sim.y_size);
	glUniform2fv(glGetUniformLocation(TILE_SHADER, "camera_position"), 1, camera_position);
	glUniform1i(glGetUniformLocation(TILE_SHADER, "tile_size"), tile_size);
	glUniform1f(glGetUniformLocation(TILE_SHADER, "camera_zoom"), camera_zoom);

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
	last_draw = t;

	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	draw_sim2();

	draw_buttons();

	if (!run_sim)
	{
		canvas->draw_rect(window_margin * 2, window_margin * 2, window_margin * 2, window_margin * 6, 0.0, 1.0, 0.0, VAO, VBO, EBO);
		canvas->draw_rect(window_margin * 6, window_margin * 2, window_margin * 2, window_margin * 6, 0.0, 1.0, 0.0, VAO, VBO, EBO);
	}

	// Swap front and back buffers
	glfwSwapBuffers(canvas->window);
	// Poll for and process events
	glfwPollEvents();
}

int main(int argc, char* argv[])
{
	setup();

	while (!glfwWindowShouldClose(canvas->window))
	{
		get_input();
		
		update();

		if (compare_times(last_draw, t) > 1.0 / target_fps)
			draw();
	}

	cleanup();

	return 0;
}