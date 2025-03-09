#include "player.hpp"

void Player::update(float delta)
{
	if (Input->is_pressed(KEY_D))
		vel_x = 50;
	else if (Input->is_pressed(KEY_A))
		vel_x = -50;
	else
		vel_x = 0;

	if (is_on_floor && Input->is_just_pressed(KEY_W))
		vel_y = -150;
	else if (Input->is_pressed(KEY_W) && sim->get_tile(x, y).material == WATER)
		vel_y = -50;

	if (can_fit(x, y + 1))
	{
		if (sim->get_tile(x, y).material == WATER)
		{
			if (!Input->is_pressed(KEY_W))
				vel_y = 30;
		}
		else
			vel_y += 500 * delta;
		is_on_floor = false;
	}
	else
		is_on_floor = true;

	if (can_fit(x, y + vel_y * delta))
	{
		y = y + vel_y * delta;
	}
	else
		vel_y = 0;

	if (can_fit(x + vel_x * delta, y))
	{
		x = x + vel_x * delta;
	}
	else if (vel_x != 0 && can_fit(x + vel_x * delta, y - 1))
	{
		x = x + vel_x * delta;
		y = y - 1;
	}
	else
		vel_x = 0;
}

bool Player::can_fit(int center_x, int center_y)
{
	for (int x_ = -size[0] * 0.5; x_ < size[0] * 0.5; x_++)
	for (int y_ = -size[1] * 0.5; y_ < size[1] * 0.5; y_++)
	{
		if (vel_y > 0 && y_ == ceil(size[1] * 0.5) - 1 && sim->get_tile(center_x + x_, center_y + y_).material == PLATFORM && !Input->is_pressed(KEY_S))
			return false;
		if (sim->is_tile_solid(center_x + x_, center_y + y_))
			return false;
	}
	return true;
}