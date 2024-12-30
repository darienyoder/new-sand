#version 330 core

layout(origin_upper_left) in vec4 gl_FragCoord;
out vec4 color;

uniform isampler2D materialTexture;

uniform int tile_size;
uniform vec2 window_size;

uniform vec2 camera_position;
uniform float camera_zoom;

int get_material(vec2 coords)
{
	if (coords.x < 0.0 || coords.y < 0.0 || coords.x >= window_size.x / tile_size || coords.y >= window_size.y / tile_size)
		return -1;
	return texture(materialTexture, coords.yx / window_size.yx * tile_size).r;
}

vec3 get_material_color(int material, float value)
{
	if (material == -1)
		return vec3(0.1);
	if (material == 0)
		return vec3(0.0);
	else if (material == 1) // SAND
		return vec3(1.0, 0.9, 0.5) * value;
	else if (material == 2) // WATER
		return vec3(0.0, 0.2, 1.0);
	else if (material == 3) // ICE
		return vec3(0.2, 0.6, 1.0);
	else if (material == 4) // STEAM
		return vec3(0.9, 0.9, 0.9) * 0.0;
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
		return vec3(0.3, 0.3, 0.3) * 0.0;
	else if (material == 12) // WOOD
		return vec3(0.427, 0.275, 0.012) * value;

	return vec3(1.0, 0.2, 1.0);
}

vec2 screen_to_game(vec2 coords)
{
	return ivec2( vec2(-window_size.xy / 2.0 + coords.xy) / tile_size / camera_zoom + camera_position.xy);
}

vec3 first_pass(vec2 screen_coords, float value)
{
	vec2 coords = screen_to_game(screen_coords.xy);
	int material = get_material(coords);
	vec3 clr = get_material_color(material, value);

	if (material == -1)
		return clr;

	for (int x = -5; x < 6; x++)
	for (int y = -5; y < 6; y++)
	{
		if (x*x + y*y <= 25)
		{
			int neighbor = get_material(coords + vec2(x, y));
			
			float scale = 0.0; // Steam
			if ((material == 0 || material == 4 || material == 11) && neighbor == 4)
				scale += 0.015;
			clr = vec3(0.9) * scale + clr * (1.0 - scale);

			scale = 0.0; // Smoke
			if ((true || material == 0 || material == 11 || material == 4) && neighbor == 11)
				scale += 0.03;
			clr = vec3(0.3) * scale + clr * (1.0 - scale);

			if (neighbor == 7) // Lava
				clr += vec3(0.02, 0.0, 0.0);

			if (neighbor == 10) // Fire
				clr += vec3(0.01, 0.0, 0.0);// * (1.0 - (x*x + y*y) / 25.0);

			if (material == 2 && neighbor == 0 && x == 0 && y == -1)
				clr = vec3(1.0, 1.0, 1.0);
		}
	}
	return clr;
}

void main()
{

	vec2 coords = screen_to_game(gl_FragCoord.xy);

	int material = get_material(coords);

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
			color.rgb += first_pass(gl_FragCoord.xy + vec2(x, y) * tile_size / hardness, value);
			count++;
		}
		color.rgb /= count;
	}
}