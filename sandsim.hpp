#ifndef SANDSIM
#define SANDSIM

#include "particle.hpp"
#include "stb_image.h"

#include <vector>

class Particle;

// Chunks only measure if a region is active
// They do NOT simulate the region themselves
struct chunk
{
	// Position
	int x = 0;
	int y = 0;

	bool active = true; // Simulates if true
	bool active_next = true;

	// Abstract
	bool abstracted = false; // Contains only one type of element
	int fill = 0; // Type of element
	int volume = 0; // Quantity of element
	bool just_deabstractified = false;
};

class SandSim
{
public:
	SandSim(int width, int height);
	SandSim(const char img_path[]);
	~SandSim() {};

	bool in_bounds(int x, int y) { return x > -1 && x < x_size && y > -1 && y < y_size; };
	bool chunk_in_bounds(int x, int y) { return x > -1 && x < x_size / chunk_size && y > -1 && y < y_size / chunk_size; };

	Particle& get_tile(int x, int y);
	void set_tile(int x, int y, int new_tile);

	void swap_tiles(int x1, int y1, int x2, int y2);
	std::vector<int> get_path(int x1, int y1, int x2, int y2, int max_length = 0);

	void explode(int x, int y, int force);
	void explode_path(int x1, int y1, int x2, int y2, float force);

	// Turn into aerial particle
	void launch(int x, int y, int vel_x, int vel_y, int mode = 0, float param = 0.0);

	// In bounds and contains nothing
	bool is_tile_empty(int x, int y);
	bool is_tile_solid(int x, int y);

	Particle* create_element(int material);

	void update(); // Step entire simulation
	void simulate_tile(int x, int y); // Step 

	void make_active(int tile_x, int tile_y); // For single tiles
	void make_chunk_active(int chunk_x, int chunk_y); // For abstracted chunks

	void abstractify_chunk(int chunk_x, int chunk_y, int material, int volume);
	void deabstract(int chunk_x, int chunk_y);
	int deabstractify_tile(int x, int y);

	// Delete everything
	void clear();

	int x_size = 0, y_size = 0;
	int time = 0;

	Particle*** tiles;

	chunk** chunks = 0;
	const int chunk_size = 20;


	std::vector<int>& get_texture_data(int origin_x, int origin_y, int width, int height, int precision = 1);

	// Saved texture to avoid redrawing every frame
	std::vector<int> texture;
	int tex_origin[2] = { -234567, 34678 };
	int tex_size[2] = { -234567, 34678 };
	int tex_precision = 0;
};

#endif