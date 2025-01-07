#version 330 core

layout(origin_upper_left) in vec4 gl_FragCoord;
out vec4 color;

// Data
uniform isampler2D materialTexture;
uniform ivec2 origin;
uniform int tile_precision;

// Scale
uniform int tile_size;
uniform vec2 window_size;
uniform vec2 sim_size;

// Camera
uniform vec2 camera_position;
uniform float camera_zoom;

const vec3 background = vec3(0.0);//vec3(0.125,0.086,0.51);

const int outline_chunks = 0;

int get_material(vec2 coords)
{
	if (coords.x < origin.x || coords.y < origin.y || coords.x >= origin.x + sim_size.x || coords.y >= origin.y + sim_size.y)
		return -1;
	return texture(materialTexture, (coords.yx - origin.yx) / sim_size.yx).r;
}

vec3 get_material_color(int material, float value)
{
	if (material == -1)
		return vec3(0.1);
	if (material == 0)
		return background;
	else if (material == 1) // SAND
		return vec3(1.0, 0.9, 0.5) * value;
	else if (material == 2) // WATER
		return vec3(0.0, 0.2, 1.0);
	else if (material == 3) // ICE
		return vec3(0.2, 0.6, 1.0);
	else if (material == 4) // STEAM
		return background; //return vec3(0.9, 0.9, 0.9) * 0.0;
	else if (material == 5) // DIRT
		return vec3(0.5, 0.25, 0.0) * value;
	else if (material == 6) // STONE
		return vec3(0.2, 0.2, 0.2) * value;
	else if (material == 7) // LAVA
		return vec3(0.9, 0.6, 0.2) * value;
	else if (material == 8) // OIL
		return vec3(0.6, 0.4, 0.1);
	else if (material == 9) // ACID
		return vec3(0.4, 0.7, 0.0);
	else if (material == 10) // FIRE
		return vec3(0.9, 0.4, 0.1);
	else if (material == 11) // SMOKE
		return background; //return vec3(0.3, 0.3, 0.3) * 0.0;
	else if (material == 12) // WOOD
		return vec3(0.427, 0.275, 0.012) * value;
	else if (material == 13) // AERIAL
		return vec3(1.0); // this will never be used
	else if (material == 14) // FIREWORK
		return vec3(0.9, 0.4, 0.1);

	return vec3(1.0, 0.2, 1.0);
}

vec2 screen_to_game(vec2 coords)
{
	return vec2( vec2(-window_size.xy / 2.0 + coords.xy) / camera_zoom + camera_position.xy);
}

vec3 first_pass(vec2 screen_coords, float value)
{
	vec2 coords = screen_to_game(screen_coords.xy);
	int material = get_material(coords);
	vec3 clr = get_material_color(material, value);

	if (material == -1)
		return clr;

	float steam = 0.0;
	float smoke = 0.0;

	for (int x = -5; x < 6; x++)
	for (int y = -5; y < 6; y++)
	{
		if (x*x + y*y <= 25)
		{
			int neighbor = get_material(coords + vec2(x, y));
			
			if ((true || material == 0 || material == 4 || material == 11) && neighbor == 4)
				steam += 0.02;

			if ((true || material == 0 || material == 11 || material == 4) && neighbor == 11)
				smoke += 0.02;

			if (neighbor == 7) // Lava
				clr += vec3(0.02, 0.0, 0.0);

			if (neighbor == 10 || neighbor == 14) // Fire
				clr += vec3(0.01, 0.0, 0.0);// * (1.0 - (x*x + y*y) / 25.0);

			if (material == 2 && neighbor == 0 && x == 0 && y == -1)
				clr = vec3(1.0, 1.0, 1.0);
		}
	}
	
	clr = vec3(0.9) * steam + clr * (1.0 - steam);
	clr = vec3(0.3) * smoke + clr * (1.0 - smoke);

	return clr;
}

void main()
{

	vec2 coords = screen_to_game(gl_FragCoord.xy);

	int material = get_material(coords);

	if (outline_chunks == 1 && material != -1 && (int(coords.x * 2) % 40 == 0 || int(coords.y * 2) % 40 == 0))
	{
		if (texture(materialTexture, (coords.yx - origin.yx) / sim_size.yx).g == 1.0)
			color.rgb = vec3(1, 0, 0);
		return;
	}
	if (outline_chunks == 1 && material == 0 && tile_precision == 1)
	{
		color.rgb = texture(materialTexture, coords.yx / sim_size.yx).g == 1.0 ? vec3(0.4, 0.2, 0) : background;
		return;
	}

	float value = ( int(pow(int(coords.x) % 50 + 50, (int(coords.y) % 20 + 20) * 0.1)) % 100) / 100.0;
	value = value * 0.25 + 0.75;

	color.rgb = first_pass(gl_FragCoord.xy, value);
	return;

	// Second Pass
	
	bool surrounded = (
		get_material(coords + vec2(1, 0)) == material
		&& get_material(coords + vec2(-1, 0)) == material
		&& get_material(coords + vec2(0, 1)) == material
		&& get_material(coords + vec2(0, -1)) == material
		&& (material != 2 || get_material(coords + vec2(0, -2)) == material)
	);
	
	if (surrounded)
		color.rgb = first_pass(gl_FragCoord.xy, value);
	else
	{
		int count = 0;
		float hardness = (material == 1) ? 2.0 : 4.0;
		for (int x = -2; x < 3; x++)
		for (int y = -2; y < 3; y++)
		//	if (x*x + y*y <= 4)
		{
			color.rgb += first_pass(gl_FragCoord.xy + vec2(x, y) / hardness, value);
			count++;
		}
		color.rgb /= count;
	}
}