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

		make_active(x, y);
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

		make_active(x1, y1);
		make_active(x2, y2);

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
	case ICE:
		ptr = new Ice;
		break;
	case STEAM:
		ptr = new Steam;
		break;
	case DIRT:
		ptr = new Dirt;
		break;
	case STONE:
		ptr = new Stone;
		break;
	case LAVA:
		ptr = new Lava;
		break;
	case OIL:
		ptr = new Oil;
		break;
	case ACID:
		ptr = new Acid;
		break;
	case FIRE:
		ptr = new Fire;
		break;
	case SMOKE:
		ptr = new Smoke;
		break;
	case WOOD:
		ptr = new Wood;
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
	for (int chunk_y = y_size / chunk_size - 1; chunk_y > -1; --chunk_y)
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
		Particle* tile = tiles[x][y];
		if (tiles[x][y]->tick())
			make_active(x, y);
		if (tiles[tile->x][tile->y]->about_to_delete)
			set_tile(tile->x, tile->y, EMPTY);
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

std::vector<int> SandSim::get_texture_data()
{
	std::vector<int> materialMatrix;

	materialMatrix.resize(x_size * y_size * 2);
	for (size_t i = 0; i < x_size; ++i)
	{
		for (size_t j = 0; j < y_size; ++j)
		{
			materialMatrix[i * y_size * 2 + j * 2] = tiles[i][j]->material;
			materialMatrix[i * y_size * 2 + j * 2 + 1] = 0;
		}
	}
	return materialMatrix;
}

void SandSim::explode(int x, int y, int force)
{
	for (int x_ = -force; x_ < force; x_++)
		explode_path(x, y, x + x_, y - force, force);

	for (int y_ = -force; y_ < force; y_++)
		explode_path(x, y, x - force, y + y_, force);

	for (int x_ = -force; x_ < force; x_++)
		explode_path(x, y, x + x_, y + force, force);

	for (int y_ = -force; y_ < force; y_++)
		explode_path(x, y, x + force, y + y_, force);
}

void SandSim::explode_path(int x1, int y1, int x2, int y2, int force)
{
	std::vector<int> path = get_path(x1, y1, x2, y2, force);

	for (int i = 0; i < path.size() / 2; i++)
	{
		auto tile = get_tile(path[i * 2], path[i * 2 + 1]);

		if (tile.material == EMPTY || tile.material == FIRE)
		{
			set_tile(path[i * 2], path[i * 2 + 1], FIRE);
		}
		else if (Gas* gas = dynamic_cast<Gas*>(&tile))
		{
			set_tile(path[i * 2], path[i * 2 + 1], FIRE);
		}
		else if (tile.material == 2)//(Liquid* liquid = dynamic_cast<Liquid*>(&tile))
		{
			break;
		}
		else
		{
			if (force > tile.hp)
			{
				get_tile(path[i * 2], path[i * 2 + 1]).remove();
				//set_tile(path[i * 2], path[i * 2 + 1], FIRE);
				force -= tile.hp;
			}
			else
				break;
		}
	}
}

std::vector<int> SandSim::get_path(int x1, int y1, int x2, int y2, int max_length)
{
	std::vector<int> path;

	int xDiff = x1 - x2;
	int yDiff = y1 - y2;
	bool xDiffIsLarger = std::abs(xDiff) > std::abs(yDiff);

	int xModifier = xDiff < 0 ? 1 : -1;
	int yModifier = yDiff < 0 ? 1 : -1;

	int longerSideLength = std::max(std::abs(xDiff), std::abs(yDiff));
	int shorterSideLength = std::min(std::abs(xDiff), std::abs(yDiff));
	float slope = (shorterSideLength == 0 || longerSideLength == 0) ? 0 : ((float)(shorterSideLength) / (longerSideLength));

	int pastX = x1;
	int pastY = y1;

	int shorterSideIncrease;
	for (int i = 1; i <= std::min((float)longerSideLength, 300000.0f); i++)
	{
		shorterSideIncrease = std::round(i * slope);
		int yIncrease, xIncrease;
		if (xDiffIsLarger)
		{
			xIncrease = i;
			yIncrease = shorterSideIncrease;
		}
		else
		{
			yIncrease = i;
			xIncrease = shorterSideIncrease;
		}
		int currentY = y1 + (yIncrease * yModifier);
		int currentX = x1 + (xIncrease * xModifier);

		if (max_length && (currentX - x1) * (currentX - x1) + (currentY - y1) * (currentY - y1) > max_length * max_length)
			break;

		path.push_back(currentX);
		path.push_back(currentY);
	}
	return path;
}