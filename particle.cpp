#include "particle.hpp"

// REQUIRES: x_ and y_ are in bounds
// ENSURES: x and y values are swapped; positions in sim are swapped
void Particle::swap(int x_, int y_)
{
	sim->swap_tiles(x, y, x_, y_);
	return;

	if (sim->in_bounds(x_, y_))
	{
		// Record initial x and y values
		int pos[2] = { x, y };
		int pos_[2] = { x_, y_ };

		// Swap sim addresses
		Particle** p1 = &(sim->tiles[x][y]);
		Particle** p2 = &(sim->tiles[x_][y_]);
		Particle* temp = *p1;
		*p1 = *p2;
		*p2 = temp;

		// Swap x and y values

		sim->tiles[x][y]->x = pos[0];
		sim->tiles[x][y]->y = pos[1];

		sim->tiles[x_][y_]->y = pos_[0];
		sim->tiles[x_][y_]->y = pos_[1];
	}
}

bool Fluid::tick()
{
	// Down
	if (flow_directions[0] && sim->is_tile_empty(x, y + 1))
	{
		swap(x, y + 1);
		return true;
	}

	// Down-side
	else if (flow_directions[1] && sim->is_tile_empty(x + 1 * bias, y + 1))
	{
		swap(x + 1 * bias, y + 1);
		return true;
	}
	else if (flow_directions[1] && sim->is_tile_empty(x - 1 * bias, y + 1))
	{
		swap(x - 1 * bias, y + 1);
		return true;
	}

	// Up
	else if (flow_directions[4] && sim->is_tile_empty(x, y - 1))
	{
		swap(x, y - 1);
		return true;
	}

	// Up-side
	else if (flow_directions[3] && sim->is_tile_empty(x + 1 * bias, y - 1))
	{
		swap(x + 1 * bias, y - 1);
		return true;
	}
	else if (flow_directions[3] && sim->is_tile_empty(x - 1 * bias, y - 1))
	{
		swap(x - 1 * bias, y - 1);
		return true;
	}

	// Side
	else if (flow_directions[2] && sim->is_tile_empty(x + 1 * bias, y))
	{
		swap(x + 1 * bias, y);
		return true;
	}
	else if (flow_directions[2] && sim->is_tile_empty(x - 1 * bias, y))
	{
		swap(x - 1 * bias, y);
		return true;
	}

	return false;
}