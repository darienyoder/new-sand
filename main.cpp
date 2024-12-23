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

SandSim sim(200, 120);
InputManager input;

int mouse_action = 1;

bool run_sim = true;

int tile_size = 5;

const int window_margin = 10;
const int button_size = 50;
const int button_count = 13;
const int button_rows = 10;

void initialize_window()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow(
		"Sand",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		sim.x_size * tile_size + window_margin * 2 + (button_size + window_margin) * (((button_count - 1) / button_rows) + 1),
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

	const int radius = 10;

	for (int x = -radius; x < radius; x++)
	for (int y = -radius; y < radius; y++)
		if (x*x + y*y < radius* radius)
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