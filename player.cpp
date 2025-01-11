#include "player.hpp"

void Player::update(float delta)
{
	if (Input->is_pressed(KEY_RIGHT))
		vel_x = 50;
	else if (Input->is_pressed(KEY_LEFT))
		vel_x = -50;
	else
		vel_x = 0;

	if (is_on_floor && Input->is_just_pressed(KEY_UP))
		vel_y = -200;

	if (!sim->is_tile_solid(x, y + 1))
	{
		vel_y += 500 * delta;
		is_on_floor = false;
	}
	else
		is_on_floor = true;

	if (!sim->is_tile_solid(x, y + vel_y * delta))
	{
		y = y + vel_y * delta;
	}
	else
		vel_y = 0;

	if (!sim->is_tile_solid(x + vel_x * delta, y))
	{
		x = x + vel_x * delta;
	}
	else if (vel_x != 0 && !sim->is_tile_solid(x + vel_x * delta, y - 1))
	{
		x = x + vel_x * delta;
		y = y - 1;
	}
	else
		vel_x = 0;

	/*
	else if (vel_x != 0 && vel_y < 40 * delta && vel_y > 0 && sim->is_tile_empty(x + vel_x * delta, y - 1))
	{
		x = x + vel_x * delta;
		y = y - 1;

		vel_y = 0;
	}
	else
	{
		vel_x = 0;
		vel_y = 0;
	}
	*/
}