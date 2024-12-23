#include "particle.hpp"

// REQUIRES: x_ and y_ are in bounds
// ENSURES: x and y values are swapped; positions in sim are swapped
void Particle::swap(int x_, int y_)
{
	sim->swap_tiles(x, y, x_, y_);
}

void Particle::set_tile(int x_, int y_, int m_)
{
	sim->set_tile(x_, y_, m_);
}

bool Powder::tick()
{
	int move_chance = std::rand() % 100;

	if (get_move_speed(x, y + 1) > move_chance)
	{
		move_to(x, y + 1);
		return true;
	}
	else
	{
		if (get_move_speed(x, y + 1) != 0)
			sim->make_active(x, y);

		int side = std::rand() % 2 ? 1 : -1;

		if (get_move_speed(x + side, y + 1) > move_chance)
		{
			move_to(x + side, y + 1);
			return true;
		}
		else if (get_move_speed(x - side, y + 1) > move_chance)
		{
			move_to(x - side, y + 1);
			return true;
		}
	}

	return false;
}

// Returns an int percentage of how quickly
// the powder moves through the tile
int Powder::get_move_speed(int x_, int y_)
{
	// Can't pass sim boundary
	if (!sim->in_bounds(x_, y_))
		return 0;
	
	// Can move through empty tiles
	if (sim->is_tile_empty(x_, y_))
		return 100;

	// Moves through empty tiles at a slower rate
	if (Liquid* liquid = dynamic_cast<Liquid*>(sim->tiles[x_][y_]))
		return (density - liquid->density) / 10;

	// Can move through gases
	if (Gas* gas = dynamic_cast<Gas*>(sim->tiles[x_][y_]))
		return 100;
	
	// Everything else is a wall
	return 0;
}

bool Liquid::tick()
{
	if (can_move_through(x, y + 1))
	{
		move_to(x, y + 1);
		return true;
	}
	else
	{
		int spread_value = std::rand() % spread + 1;

		if (can_move_through(x + direction, y + 1))
		{
			move_to(x + direction, y + 1);
			return true;
		}
		else if (can_move_through(x - direction, y + 1))
		{
			move_to(x - direction, y + 1);
			direction *= -1;
			return true;
		}

		else if (can_move_through(x + direction, y))
		{
			for (int i = 0; i < spread_value && can_move_through(x + direction, y); i++)
				move_to(x + direction, y);
			return true;
		}
		else if (can_move_through(x - direction, y))
		{
			direction *= -1;
			for (int i = 0; i < spread_value && can_move_through(x + direction, y); i++)
				move_to(x + direction, y);
			return true;
		}
	}

	return false;
}

bool Liquid::can_move_through(int x_, int y_)
{
	if (sim->in_bounds(x_, y_))
	{
		// Move through empty tiles
		if (sim->is_tile_empty(x_, y_))
			return true;
		// Move through liquids less denses than yourself
		if (Liquid* liquid = dynamic_cast<Liquid*>(&sim->get_tile(x_, y_)))
			return density > liquid->density;
		// Move through all gases
		if (Gas* gas = dynamic_cast<Gas*>(&sim->get_tile(x_, y_)))
			return true;
	}
	// Everything else is a wall
	return false;
}

bool Gas::tick()
{
	// Straight up
	if (sim->is_tile_empty(x, y - 1) && std::rand() % 3 == 0)
	{
		move_to(x, y - 1);
		return true;
	}
	else
	{
		int side_array[2] = { -1, 1 };
		int side = side_array[std::rand() % 2];

		// Upward-diagonal
		if (sim->is_tile_empty(x + side, y - 1) && std::rand() % 3 == 0)
		{
			move_to(x + side, y - 1);
			return true;
		}
		else if (sim->is_tile_empty(x - side, y - 1) && std::rand() % 3 == 0)
		{
			move_to(x - side, y - 1);
			return true;
		}

		// Sideways
		else if (sim->is_tile_empty(x + side, y) && std::rand() % 3 == 0)
		{
			move_to(x + side, y);
			return true;
		}
		else if (sim->is_tile_empty(x - side, y))
		{
			move_to(x - side, y);
			return true;

		}

		// Downward-diagonal
		if (sim->is_tile_empty(x + side, y + 1) && std::rand() % 3 == 0)
		{
			move_to(x + side, y + 1);
			return true;
		}
		else if (sim->is_tile_empty(x - side, y + 1) && std::rand() % 3 == 0)
		{
			move_to(x - side, y + 1);
			return true;
		}

		// Straight down
		if (sim->is_tile_empty(x, y + 1))
		{
			move_to(x, y + 1);
			return true;
		}
	}

	return false;
}


bool Acid::can_move_through(int x_, int y_)
{
	if (sim->in_bounds(x_, y_))
	{
		// Move through empty tiles
		if (sim->is_tile_empty(x_, y_))
			return true;
		// Move through liquids less denses than yourself
		if (Acid* acid = dynamic_cast<Acid*>(&sim->get_tile(x_, y_)))
			return false; //density > liquid->density;
		// Move through all gases
		if (Gas* gas = dynamic_cast<Gas*>(&sim->get_tile(x_, y_)))
			return true;
		// Everything else is a wall
		return std::rand() % 100 < 10;
	}
	return false;
}

void Acid::move_to(int x_, int y_)
{
	int old_x = x, old_y = y;
	swap(x_, y_);
	if (!sim->is_tile_empty(old_x, old_y))
	{
		sim->get_tile(old_x, old_y).remove();
		remove();
	}
}

bool Lava::tick()
{
	Liquid::tick();

	for (int x_ = -1; x_ < 2; x_++)
	for (int y_ = -1; y_ < 2; y_++)
	{
		if (Liquid* liquid = dynamic_cast<Liquid*>(&sim->get_tile(x + x_, y + y_)))
			liquid->evaporate();
		else if (Solid* solid = dynamic_cast<Solid*>(&sim->get_tile(x + x_, y + y_)))
			solid->melt();
	}
	return true;
}