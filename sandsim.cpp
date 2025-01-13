#include "sandsim.hpp"
#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// SandSim

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

SandSim::SandSim(const char img_path[])
{
	int channels;
	unsigned char* texture = stbi_load(img_path, &x_size, &y_size, &channels, 0);

	{
		tiles = new Particle** [x_size];
		for (int i = 0; i < x_size; ++i)
		{
			tiles[i] = new Particle* [y_size];
		}

		for (int i = 0; i < x_size; ++i)
			for (int j = 0; j < y_size; ++j)
			{
				tiles[i][j] = new Air;
				tiles[i][j]->material = EMPTY;
				tiles[i][j]->x = i;
				tiles[i][j]->y = j;
			}

		chunks = new chunk* [x_size / chunk_size];
		for (int x = 0; x < x_size / chunk_size; x++)
			chunks[x] = new chunk[y_size / chunk_size];
	}

	std::vector<int> keys;
	std::vector<int> values;

	for (int y = 0; ; y++)
	{
		if (texture[y * x_size * channels + 1 * channels] == 0)
			break;

		keys.push_back(texture[y * x_size * channels + 0]);
		keys.push_back(texture[y * x_size * channels + 1]);
		keys.push_back(texture[y * x_size * channels + 2]);

		values.push_back(texture[y * x_size * channels + 1 * channels]);

		for (int i = 0; i < channels * 2; i++)
			texture[y * x_size * channels + i] = 0;
	}

	for (int x = 0; x < x_size; x++)
	for (int y = 0; y < y_size; y++)
	{
		int index = y * x_size * channels + x * channels;
		for (int key = 0; key < keys.size() / channels; key++)
		{
			if (keys[key * keys.size() / channels] == texture[index] && keys[key * keys.size() / channels + 1] == texture[index + 1] && keys[key * keys.size() / channels + 2] == texture[index + 2])
			{
				set_tile(x, y, values[key]);
				break;
			}
		}
	}

	stbi_image_free(texture);
}

void SandSim::clear()
{
	// Delete all tiles
	for (int x = 0; x < x_size; x++)
	for (int y = 0; y < y_size; y++)
		set_tile(x, y, EMPTY);

	// Deabstractify all chunks
	for (int x = 0; x < x_size / chunk_size; x++)
	for (int y = 0; y < y_size / chunk_size; y++)
	{
		chunks[x][y].abstracted = true;
		chunks[x][y].fill = EMPTY;
		chunks[x][y].volume = 0;
	}
}

Particle& SandSim::get_tile(int x, int y)
{
	if (in_bounds(x, y))
	{
		if (chunks[x / chunk_size][y / chunk_size].abstracted)
			return *sample[deabstractify_tile(x, y)];
		else
			return *tiles[x][y];
	}
	Air barrier;
	return barrier;
}

void SandSim::set_tile(int x, int y, int material)
{
	if (in_bounds(x, y))
	{
		if (chunks[x / chunk_size][y / chunk_size].abstracted && chunks[x / chunk_size][y / chunk_size].fill == material)
		{
			chunks[x / chunk_size][y / chunk_size].volume = std::min(chunk_size * chunk_size, chunks[x / chunk_size][y / chunk_size].volume + 1);
			make_chunk_active(x / chunk_size, y / chunk_size);
			return;
		}
		else
		{
			if (chunks[x / chunk_size][y / chunk_size].abstracted)
				deabstract(x / chunk_size, y / chunk_size);
		}

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
	// Tiles must be valid
	if (in_bounds(x1, y1) && in_bounds(x2, y2))
	{
		// Ensure chunks are not abstracted
		if (chunks[x1 / chunk_size][y1 / chunk_size].abstracted)
			deabstract(x1 / chunk_size, y1 / chunk_size);
		if (chunks[x2 / chunk_size][y2 / chunk_size].abstracted)
			deabstract(x2 / chunk_size, y2 / chunk_size);

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

		// Activate chunks
		make_active(x1, y1);
		make_active(x2, y2);
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
	case FIREWORK:
		ptr = new Firework;
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

	// Determine active chunks
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
		{
			// If abstracted
			if (chunks[chunk_x][chunk_y].abstracted && false)
			{
				// Skip empty chunks
				if (chunks[chunk_x][chunk_y].fill != EMPTY)
				{
					// Get state of chunk fill
					int state = 0;
					if (Powder* pow = dynamic_cast<Powder*>(sample[chunks[chunk_x][chunk_y].fill]))
						state = SAND;
					else if (Liquid* liq = dynamic_cast<Liquid*>(sample[chunks[chunk_x][chunk_y].fill]))
						state = WATER;
					else if (Gas* gas = dynamic_cast<Gas*>(sample[chunks[chunk_x][chunk_y].fill]))
						state = STEAM;

					// Valid moves for each state of matter
					std::vector<int> movement;
					if (state == SAND)
						movement = { 0, 1, 1, 1, -1, 1 };
					else if (state == WATER)
						movement = { 0, 1, 1, 1, -1, 1, 1, 0, -1, 0 };
					else if (state == STEAM)
						movement = { 0, -1, 1, -1, -1, 1, -1, 0, -1, 0 };

					int dir = rand() % 2 == 0 ? -1 : 1;

					// For each 
					for (int i = 0; i < movement.size() / 2; i++)
					{
						// If adjacent chunk in bounds...
						if (chunk_x + movement[i * 2] * dir >= 0 && chunk_y + movement[i * 2 + 1] >= 0 && chunk_x + movement[i * 2] * dir < x_size / chunk_size && chunk_y + movement[i * 2 + 1] < y_size / chunk_size)
						{
							chunk* adjacent_chunk = &chunks[chunk_x + movement[i * 2] * dir][chunk_y + movement[i * 2 + 1]];
							
								// If chunk is abstracted
							if (adjacent_chunk->abstracted
								// and has space
								&& adjacent_chunk->volume < chunk_size * chunk_size
								// and, if spreading horizontally, is spreading to a less full chunk
								&& (movement[i * 2 + 1] != 0 || chunks[chunk_x][chunk_y].volume > adjacent_chunk->volume )
								// and the fills match
								&& (adjacent_chunk->fill == chunks[chunk_x][chunk_y].fill || adjacent_chunk->fill == EMPTY)
								// and an arbitrary rule to make liquids less jittery
								&& (state != WATER || movement[i * 2 + 1] != 0 || chunks[chunk_x][chunk_y].volume - adjacent_chunk->volume > chunk_size))
							{
								// Set empty chunks to have same fill
								if (adjacent_chunk->fill == EMPTY)
								{
									chunks[chunk_x + movement[i * 2] * dir][chunk_y + movement[i * 2 + 1]].fill = chunks[chunk_x][chunk_y].fill;
									chunks[chunk_x + movement[i * 2] * dir][chunk_y + movement[i * 2 + 1]].volume = 0;
								}
								// Transfer no more than source can give or sink can take
								int transfer = std::min(std::min(chunks[chunk_x][chunk_y].volume, chunk_size), chunk_size * chunk_size - adjacent_chunk->volume);
								chunks[chunk_x][chunk_y].volume -= transfer;
								chunks[chunk_x + movement[i * 2] * dir][chunk_y + movement[i * 2 + 1]].volume += transfer;

								make_chunk_active(chunk_x, chunk_y);

								break;
							}
						}
					}

					if (false && (state == SAND || state == WATER)
						&& chunk_y != y_size / chunk_size - 1
						&& !chunks[chunk_x][chunk_y + 1].abstracted)
					{
						for (int bottom_tile = 0; bottom_tile < chunk_size; bottom_tile++)
						{
							if (chunks[chunk_x][chunk_y].volume > 0 && get_tile(chunk_x * chunk_size + bottom_tile, (chunk_y + 1) * chunk_size).material == EMPTY)
							{
								chunks[chunk_x][chunk_y].volume -= 1;
								set_tile(chunk_x * chunk_size + bottom_tile, (chunk_y + 1) * chunk_size, chunks[chunk_x][chunk_y].fill);
							}
						}
					}

					if (chunks[chunk_x][chunk_y].volume <= 0)
					{
						chunks[chunk_x][chunk_y].fill = EMPTY;
						chunks[chunk_x][chunk_y].volume = 0;
					}
				}
			}
			// If not abstracted
			else
			{
				chunks[chunk_x][chunk_y].fill = EMPTY;
				chunks[chunk_x][chunk_y].volume = 0;
				bool solid_floor = true;

				// Divide the chunk into checkerboards
				for (int i_x = 0; i_x < spacing; ++i_x)
				for (int i_y = 0; i_y < spacing; ++i_y)
					// Iterate over every cell in the checkerboard
					for (int x = 0; x < chunk_size / spacing; ++x)
					for (int y = 0; y < chunk_size / spacing; ++y)
					{
						int tile_x = chunk_x * chunk_size + (chunk_size - 1 - x * spacing - i_x) * sim_dir + (x * spacing + i_x) * (1 - sim_dir);
						int tile_y = chunk_y * chunk_size + (chunk_size - 1 - y * spacing - i_y) * sim_dir + (y * spacing + i_y) * (1 - sim_dir);
						int tile = tiles[tile_x][tile_y]->material;
						// Aerial tiles read as their cargo
						if (tile == AERIAL)
						{
							Aerial* aer = dynamic_cast<Aerial*>(tiles[tile_x][tile_y]);
							tile = aer->p->material;
						}

						// 
						if (chunks[chunk_x][chunk_y].fill != -1 && chunks[chunk_x][chunk_y].fill != tile && tile != EMPTY)
						{
							if (chunks[chunk_x][chunk_y].fill == EMPTY)
								chunks[chunk_x][chunk_y].fill = tile;
							else
								chunks[chunk_x][chunk_y].fill = -1;
						}


						if (tile != EMPTY)
							chunks[chunk_x][chunk_y].volume += 1;
						else if (tile_y == (chunk_y + 1) * chunk_size - 1)
							solid_floor = false;

						if (!chunks[chunk_x][chunk_y].just_deabstractified)
							simulate_tile( tile_x, tile_y );
					}
				if (chunks[chunk_x][chunk_y].fill != -1)
				{
					if (chunks[chunk_x][chunk_y].fill == 0)
						abstractify_chunk(chunk_x, chunk_y, chunks[chunk_x][chunk_y].fill, chunks[chunk_x][chunk_y].volume);
					else
					{
						int state = 0;
						if (Powder* pow = dynamic_cast<Powder*>(sample[chunks[chunk_x][chunk_y].fill]))
							state = SAND;
						else if (Liquid* liq = dynamic_cast<Liquid*>(sample[chunks[chunk_x][chunk_y].fill]))
							state = WATER;
						else if (Gas* gas = dynamic_cast<Gas*>(sample[chunks[chunk_x][chunk_y].fill]))
							state = STEAM;

						if (state != 0)
							abstractify_chunk(chunk_x, chunk_y, chunks[chunk_x][chunk_y].fill, chunks[chunk_x][chunk_y].volume);
					}
				}
				chunks[chunk_x][chunk_y].just_deabstractified = false;
			}
		}
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
		{
			if (tiles[tile->x][tile->y]->material == AERIAL)
			{
				Aerial* aer = dynamic_cast<Aerial*>(tiles[tile->x][tile->y]);

				if (aer->p->material == FIREWORK)
				{
					explode(tile->x, tile->y, 30 + rand() % 30);
				}
				
				if (aer->about_to_full_delete)
					set_tile(tile->x, tile->y, EMPTY);
				else
				{
					Particle* temp = tiles[tile->x][tile->y];

					tiles[tile->x][tile->y] = aer->p;

					tiles[tile->x][tile->y]->x = tile->x;
					tiles[tile->x][tile->y]->y = tile->y;

					delete temp;
				}
			}
			else
				set_tile(tile->x, tile->y, EMPTY);
		}
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

std::vector<int>& SandSim::get_texture_data(int origin_x, int origin_y, int width, int height, int precision)
{
	origin_x = (origin_x / chunk_size) * chunk_size;
	origin_y = (origin_y / chunk_size) * chunk_size;
	width = (width / chunk_size) * chunk_size;
	height = (height / chunk_size) * chunk_size;

	bool camera_moved = (origin_x != tex_origin[0] || origin_y != tex_origin[1] || width != tex_size[0] || height != tex_size[1]);

	texture.resize(width * height * 2 / (precision * precision));

	int tile_x;
	int tile_y;
	int index;

	for (int chunk_x = 0; chunk_x < width / chunk_size; ++chunk_x)
	for (int chunk_y = 0; chunk_y < height / chunk_size; ++chunk_y)
		if (!chunk_in_bounds(origin_x / chunk_size + chunk_x, origin_y / chunk_size + chunk_y)
		|| (camera_moved || chunks[origin_x / chunk_size + chunk_x][origin_y / chunk_size + chunk_y].active))
			for (int x = 0; x < chunk_size / precision; ++x)
			for (int y = 0; y < chunk_size / precision; ++y)
			{
				tile_x = origin_x + chunk_x * chunk_size + x * precision;
				tile_y = origin_y + chunk_y * chunk_size + y * precision;
				index = ((chunk_x * chunk_size) / precision + x) * (height / precision) * 2 + ((chunk_y * chunk_size) / precision + y) * 2;

				if (!in_bounds(tile_x, tile_y))
				{
					texture[index] = -1;
					texture[index + 1] = 0;
				}
				else if (chunks[origin_x / chunk_size + chunk_x][origin_y / chunk_size + chunk_y].abstracted)
				{
					texture[index] = deabstractify_tile(tile_x, tile_y);
					texture[index + 1] = chunks[origin_x / chunk_size + chunk_x][origin_y / chunk_size + chunk_y].active;
				}
				else
				{
					if (Aerial* aer = dynamic_cast<Aerial*>(tiles[tile_x][tile_y]))
						texture[index] = aer->p->material;
					else
						texture[index] = tiles[tile_x][tile_y]->material;
					texture[index + 1] = chunks[origin_x / chunk_size + chunk_x][origin_y / chunk_size + chunk_y].active;
				}
			}

	return texture;
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

void SandSim::explode_path(int x1, int y1, int x2, int y2, float force)
{
	float init_force = force;
	std::vector<int> path = get_path(x1, y1, x2, y2);

	float total_dist = 0.0;

	if (force > get_tile(x1, y1).hp)
		set_tile(x1, y1, FIRE);

	bool destroy = true;

	for (int i = 0; i < path.size() / 2; i++)
	{
		if (i == 0)
			continue;
		if (!in_bounds(path[i * 2], path[i * 2 + 1]))
			break;

		auto tile = tiles[path[i * 2]][path[i * 2 + 1]];
		total_dist = std::sqrt((path[i * 2] - x1) * (path[i * 2] - x1) + (path[i * 2 + 1] - y1) * (path[i * 2 + 1] - y1));
		float dist = total_dist - std::sqrt((path[(i-1) * 2] - x1) * (path[(i-1) * 2] - x1) + (path[(i-1) * 2 + 1] - y1) * (path[(i-1) * 2 + 1] - y1));

		// Ignore air tiles that have just been filled with fire
		if (tile->material == FIRE)
		{
			//force -= dist;
			float length = std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
			int speed = std::rand() % 3 + 1;
			launch(path[i * 2], path[i * 2 + 1], (x2 - x1) / 10 * speed, (y2 - y1) / 10 * speed, PARTICLE_MODE_DISTANCE, std::max(std::abs(x2 - x1), std::abs(y2 - y1)) - total_dist);
			if (force <= 0)
				break;
		}
		// Just exploded tiles
		else if (tile->material == AERIAL)
		{
			if (Liquid* liq = dynamic_cast<Liquid*>(dynamic_cast<Aerial*>(tile)->p))
				destroy = false;
			force -= tile->hp * (1 + (rand() % 5 - 2) * 0.1) * (0.5 + dist * 0.5);
		}
		// Empty air tiles fill with fire
		else if (tile->material == EMPTY)
		{
			if (destroy && (rand() % 100) / 50.0 < force / init_force)// && total_dist < std::max(std::abs(x2 - x1), std::abs(y2 - y1)) * 0.25)
				set_tile(path[i * 2], path[i * 2 + 1], FIRE);
			force -= dist;
			if (force <= 0)
				break;
		}
		// Gases become fire
		else if (Gas* gas = dynamic_cast<Gas*>(tile))
		{
			force -= dist;
			if (destroy && (rand() % 100) / 50.0 < force / init_force && total_dist < std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)) * 0.25)
				set_tile(path[i * 2], path[i * 2 + 1], FIRE);
		}
		// Everything else
		else
		{
			force -= tile->hp * (1 + (rand() % 5 - 2) * 0.1) * (0.5 + dist * 0.5);
			if (force < 1)
				break;
			
			if (Liquid* liq = dynamic_cast<Liquid*>(tile))
				destroy = false;

			if (destroy)
				if (Powder* pow = dynamic_cast<Powder*>(tile))
					destroy = force > init_force * 0.25;

			if (Solid* sol = dynamic_cast<Solid*>(tile))
				if (!destroy || force < init_force * 0.25)
					break;

			if (destroy)
			{
				if ((rand() % 100) / 100.0 > force / init_force)
					set_tile(path[i * 2], path[i * 2 + 1], FIRE);
				else
					set_tile(path[i * 2], path[i * 2 + 1], EMPTY);
			}
			else
			{
				float length = std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
				int speed = std::rand() % 3 + 1;
				launch(path[i * 2], path[i * 2 + 1], (x2 - x1) / 10 * speed + rand() % 5 - 2, -std::abs((y1 - y2) / 10 * speed));
			}
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
	for (int i = 0; i <= std::min((float)longerSideLength, 300000.0f); i++)
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

void SandSim::launch(int x, int y, int vel_x, int vel_y, int mode, float param)
{
	if (in_bounds(x, y))
	{
		if (tiles[x][y]->material == AERIAL)
			return;

		Aerial* aer = new Aerial(get_tile(x, y));

		switch (mode)
		{
		case PARTICLE_MODE_GRAVITY:
			aer->mode = PARTICLE_MODE_GRAVITY;
			break;
		case PARTICLE_MODE_DISTANCE:
			aer->mode = PARTICLE_MODE_DISTANCE;
			aer->limit = param;
			break;
		case PARTICLE_MODE_TIME:
			aer->mode = PARTICLE_MODE_TIME;
			aer->limit = param;
			break;
		}

		aer->vel_x = vel_x;
		aer->vel_y = vel_y;
		tiles[x][y] = aer;
	}
}

void SandSim::abstractify_chunk(int chunk_x, int chunk_y, int material, int volume)
{
	return;
	chunks[chunk_x][chunk_y].abstracted = true;
	chunks[chunk_x][chunk_y].fill = material;

	if (material == 0)
		chunks[chunk_x][chunk_y].volume = 0;
	else
	{
		chunks[chunk_x][chunk_y].volume = volume;
		for (int x = 0; x < chunk_size; x++)
			for (int y = 0; y < chunk_size; y++)
			{
				set_tile(chunk_x * chunk_size + x, chunk_y * chunk_size + y, EMPTY);
			}
	}
	chunks[chunk_x][chunk_y].abstracted = true;
}

void SandSim::deabstract(int chunk_x, int chunk_y)
{
	if (chunks[chunk_x][chunk_y].abstracted)
	{
		chunks[chunk_x][chunk_y].abstracted = false;

		for (int x = 0; x < chunk_size; x++)
		for (int y = 0; y < chunk_size; y++)
		{
			set_tile(chunk_x * chunk_size + x, chunk_y * chunk_size + y, deabstractify_tile(chunk_x * chunk_size + x, chunk_y * chunk_size + y));
		}
		chunks[chunk_x][chunk_y].just_deabstractified = true;
	}
}

// Chunk must be abstracted or unexpected things will happen
int SandSim::deabstractify_tile(int x, int y)
{
	return chunks[x / chunk_size][y / chunk_size].fill * int((chunk_size - (y % chunk_size) - 1) * chunk_size + (x % chunk_size) < chunks[x / chunk_size][y / chunk_size].volume);
}

void SandSim::make_chunk_active(int chunk_x, int chunk_y)
{
	for (int x = -1; x < 2; x++)
	for (int y = -1; y < 2; y++)
		if (chunk_in_bounds(chunk_x + x, chunk_y + y))
			chunks[chunk_x + x][chunk_y + y].active_next = true;
}

bool SandSim::is_tile_solid(int x, int y)
{
	if (!in_bounds(x, y))
		return true;
	Particle* p = &get_tile(x, y);
	if (Solid* sol = dynamic_cast<Solid*>(p))
		return true;
	if (Powder* pow = dynamic_cast<Powder*>(p))
		return true;
	return false;
}