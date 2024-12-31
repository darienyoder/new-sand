#ifndef SANDSIM
#define SANDSIM

#include "particle.hpp"
#include <vector>

class Particle;

// Chunks only measure if a region is active
// They do NOT simulate the region themselves
struct chunk
{
	bool active = false;
	bool active_next = false;
	int x = 0;
	int y = 0;
};

class SandSim
{
public:
	SandSim(int width, int height);
	~SandSim() {};

	bool in_bounds(int x, int y) { return x > -1 && x < x_size && y > -1 && y < y_size; };

	Particle& get_tile(int x, int y);
	void set_tile(int x, int y, int new_tile);

	void swap_tiles(int x1, int y1, int x2, int y2);
	std::vector<int> get_path(int x1, int y1, int x2, int y2, int max_length = 0);

	void explode(int x, int y, int force);
	void explode_path(int x1, int y1, int x2, int y2, int force);

	bool is_tile_empty(int x, int y);

	Particle* create_element(int material);

	void update();
	void simulate_tile(int x, int y);

	void make_active(int tile_x, int tile_y);

	void clear();

	std::vector<int> get_texture_data();

	int x_size = 0, y_size = 0;
	int time = 0;

	Particle*** tiles;

	chunk** chunks = 0;
	const int chunk_size = 20;
};

#endif