#include <iostream>
#include <SDL.h>
#include <math.h>
#include "sandsim.hpp"
#include "input.hpp"

SDL_Window* window;
SDL_Renderer* renderer;

bool game_is_running = false;
float time_since_last_frame = 0.0;
int target_fps = 60;
int tps = 60;
int t = 0;

SandSim sim(500, 240);
InputManager input;

int mouse_action = 1;

bool run_sim = true;

int tile_size = 2;

const int window_margin = 10;
const int button_size = 50;
const int button_count = 3;
const int button_rows = 10;

void initialize_window()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow(
		"Sand",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		sim.x_size * tile_size + window_margin * 2 + (button_size + window_margin) * ((button_count / button_rows) + 1),
		sim.y_size * tile_size + window_margin * 2,
		0//SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED
	);
	renderer = SDL_CreateRenderer(window, -1, 0);

	game_is_running = true;
}

void cleanup()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void setup()
{
	input.set_monitering(SDLK_ESCAPE);
}

void screen_to_sim(int x, int y, int output[])
{
	output[0] = (x - window_margin) / tile_size;
	output[1] = (y - window_margin) / tile_size;
}

void place_tile(int new_tile, int state = 0)
{
	int tile[2];
	screen_to_sim(input.mouse_x, input.mouse_y, tile);
	for (int x = -5; x < 5; x++)
	for (int y = -5; y < 5; y++) 
		if (x*x + y*y < 25)
		{
			sim.set_tile(tile[0] + x, tile[1] + y, new_tile);
			sim.make_active(tile[0] + x, tile[1] + y);
		}
}

void get_input()
{
	input.update();
	
	if (input.mouse_down)
	{
		if (input.mouse_x > window_margin && input.mouse_x < window_margin + sim.x_size * tile_size && input.mouse_y > window_margin && input.mouse_y < window_margin + sim.y_size * tile_size)
			switch (mouse_action)
			{
			case 1:
				place_tile(SAND);
				break;
				
			case 2:
				place_tile(WATER);
				break;
				/*
			case 3:
				place_tile(WATER, POWDER);
				break;
			case 4:
				place_tile(WATER, GAS);
				break;
			case 5:
				place_tile(WOOD, SOLID);
				break;
			case 6:
				place_tile(STONE, SOLID);
				break;
			case 7:
				place_tile(STONE, LIQUID);
				break;
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

				// COLUMN 2

			case 10:
				place_tile(WIRE, SOLID);
				break;
			case 11:
				place_tile(BATTERY, SOLID);
				break;
			case 12:
				place_tile(HEATER, SOLID);
				break;
				*/
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
	else if (input.right_mouse_down)
	{
		place_tile(EMPTY);
	}
	

	if (input.is_pressed(SDLK_ESCAPE))
		game_is_running = false;

}

void update()
{
	++t;

	int time_to_wait = (1000.0f / target_fps) - (SDL_GetTicks() - time_since_last_frame);

	if (time_to_wait > 0 && time_to_wait <= (1000.0f / target_fps))
		SDL_Delay(time_to_wait);


	float delta = (SDL_GetTicks() - time_since_last_frame) / 1000.0f;
	time_since_last_frame = SDL_GetTicks();

	if (run_sim && t % (target_fps / tps) == 0)
	{
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
}

void draw_sim()
{
	// Black background
	SDL_Rect background = { 10, 10, tile_size * sim.x_size, tile_size * sim.y_size };
	SDL_SetRenderDrawColor(renderer, 00, 00, 00, 255);
	SDL_RenderFillRect(renderer, &background);

	int r = 255, g = 0, b = 0;
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

			switch (sim.tiles[x][y]->material)
			{
			case EMPTY: // EMPTY
				r = 255;
				g = 255;
				b = 255;
				break;
			case SAND: // SAND
				r = 255;
				g = 230;
				b = 128;
				break;
			case WATER:
				r = 0;
				g = 50;
				b = 255;
				break;
			}
			

			SDL_Rect box = { x * tile_size + 10, y * tile_size + 10, tile_size, tile_size };
			SDL_SetRenderDrawColor(renderer, r * value, g * value, b * value, 255);
			SDL_RenderFillRect(renderer, &box);
		}
	}
}

void draw()
{
	SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
	SDL_RenderClear(renderer);

	int size[2];
	SDL_GetWindowSize(window, &size[0], &size[1]);
	if ((float)size[0] / (float)size[1] < (float)sim.x_size / (float)sim.y_size)
		tile_size = size[0] / sim.x_size;
	else
		tile_size = size[1] / sim.y_size;

	draw_sim();

	draw_buttons();

	SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[])
{
	initialize_window();

	setup();

	while (game_is_running && !input.clicked_x)
	{
		get_input();
		update();
		draw();
	}

	cleanup();

	return 0;
}