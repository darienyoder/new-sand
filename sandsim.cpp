#include "sandsim.hpp"
#include <iostream>

SandSim::SandSim(int width, int height)
{
	tiles = new Particle** [width];
	for (int i = 0; i < width; ++i)
	{
		tiles[i] = new Particle* [height];
	}

	for (int i = 0; i < width; ++i)
	for (int j = 0; j < height; ++j)
	{
		tiles[i][j] = new Air;
		tiles[i][j]->material = EMPTY;
		tiles[i][j]->x = i;
		tiles[i][j]->y = j;
	}

	chunks = new chunk * [width / chunk_size];
	for (int x = 0; x < width / chunk_size; x++)
		chunks[x] = new chunk[height / chunk_size];

	x_size = width;
	y_size = height;
}

void SandSim::clear()
{
	for (int x = 0; x < x_size; x++)
	for (int y = 0; y < y_size; y++)
		set_tile(x, y, EMPTY);
}

Particle& SandSim::get_tile(int x, int y)
{
	if (in_bounds(x, y))
		return *tiles[x][y];
	Air barrier;
	return barrier;
}

void SandSim::set_tile(int x, int y, int material)
{
	if (in_bounds(x, y))
	{
		Particle* temp = tiles[x][y];

		tiles[x][y] = create_element(material);

		tiles[x][y]->sim = this;
		tiles[x][y]->x = x;
		tiles[x][y]->y = y;

		delete temp;
	}
}

void SandSim::swap_tiles(int x1, int y1, int x2, int y2)
{
	if (in_bounds(x1, y1) && in_bounds(x2, y2))
	{
		// Swap sim addresses
		Particle** p1 = &(tiles[x1][y1]);
		Particle** p2 = &(tiles[x2][y2]);
		Particle* temp = *p1;
		*p1 = *p2;
		*p2 = temp;

		// Swap x and y values

		tiles[x1][y1]->x = x1;
		tiles[x1][y1]->y = y1;

		tiles[x2][y2]->x = x2;
		tiles[x2][y2]->y = y2;

		// Debug print
		/*
		std::cout << "\n=================================\n";
		if (x1 != tiles[x1][y1]->x || x2 != tiles[x2][y2]->x || y1 != tiles[x1][y1]->y || y2 != tiles[x2][y2]->y)
			std::cout << "\n! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !";
		std::cout << "\n(" << x1 << ", " << y1 << ") -> (" << x2 << ", " << y2 << ")";
		std::cout << "\n(" << x1 << ", " << y1 << ") X: " << tiles[x1][y1]->x << " | Y: " << tiles[x1][y1]->y << " | " << tiles[x1][y1]->material;
		std::cout << "\n(" << x2 << ", " << y2 << ") X: " << tiles[x2][y2]->x << " | Y: " << tiles[x2][y2]->y << " | " << tiles[x2][y2]->material << "\n";
		*/
	}
}

bool SandSim::is_tile_empty(int x, int y)
{
	return in_bounds(x, y) && tiles[x][y]->material == EMPTY;
}

Particle* SandSim::create_element(int material)
{
	Particle* ptr;
	switch (material)
	{
	case EMPTY:
		ptr = new Air;
		break;
	case SAND:
		ptr = new Sand;
		break;
	case WATER:
		ptr = new Water;
		break;
	default:
		ptr = new Air;
		break;
	}
	ptr->material = material;
	return ptr;
}

void SandSim::update()
{
	++time;

	for (int chunk_x = 0; chunk_x < x_size / chunk_size; ++chunk_x)
	for (int chunk_y = 0; chunk_y < y_size / chunk_size; ++chunk_y)
	{
		chunks[chunk_x][chunk_y].active = chunks[chunk_x][chunk_y].active_next;
		chunks[chunk_x][chunk_y].active_next = false;
	}

	const int spacing = 2;
	int sim_dir = std::rand() % 2;
	// For every chunk...
	for (int chunk_x = 0; chunk_x < x_size / chunk_size; ++chunk_x)
	for (int chunk_y = 0; chunk_y < y_size / chunk_size; ++chunk_y)
		// If that chunk is active...
		if (chunks[chunk_x][chunk_y].active)
			// Divide the chunk into checkerboards
			for (int i_x = 0; i_x < spacing; ++i_x)
			for (int i_y = 0; i_y < spacing; ++i_y)
				// Iterate over every cell in the checkerboard
				for (int x = 0; x < chunk_size / spacing; ++x)
				for (int y = 0; y < chunk_size / spacing; ++y)
					simulate_tile
					(
						chunk_x * chunk_size + (chunk_size - 1 - x * spacing - i_x) * sim_dir + (x * spacing + i_x) * (1 - sim_dir),
						chunk_y * chunk_size + (chunk_size - 1 - y * spacing - i_y) * sim_dir + (y * spacing + i_y) * (1 - sim_dir)
					);
}

void SandSim::simulate_tile(int x, int y)
{
	if (tiles[x][y]->last_tick < time)
	{
		tiles[x][y]->last_tick = time;
		if (tiles[x][y]->tick())
			make_active(x, y);
	}
}

void SandSim::make_active(int tile_x, int tile_y)
{
	if (tile_x < 0 || tile_x >= x_size || tile_y < 0 || tile_y >= y_size)
		return;
	chunks[tile_x / chunk_size][tile_y / chunk_size].active_next = true;

	if (tile_x % chunk_size == 0 && tile_x != 0)
		chunks[tile_x / chunk_size - 1][tile_y / chunk_size].active_next = true;
	if (tile_x % chunk_size == chunk_size - 1 && tile_x != x_size - 1)
		chunks[tile_x / chunk_size + 1][tile_y / chunk_size].active_next = true;

	if (tile_y % chunk_size == 0 && tile_y != 0)
		chunks[tile_x / chunk_size][tile_y / chunk_size - 1].active_next = true;
	if (tile_y % chunk_size == chunk_size - 1 && tile_y != y_size - 1)
		chunks[tile_x / chunk_size][tile_y / chunk_size + 1].active_next = true;
}