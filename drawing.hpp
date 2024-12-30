#ifndef DRAWING_FUNCTIONS
#define DRAWING_FUNCTIONS

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <fstream>
#include <iostream>

extern unsigned int TILE_SHADER, COLOR_SHADER;

class Canvas
{
public:
	Canvas(const Canvas& obj) = delete;
	static Canvas* getInstance() {
		if (instancePtr == nullptr)
		{
			if (instancePtr == nullptr) {
				instancePtr = new Canvas();
			}
		}
		return instancePtr;
	}

	void initialize_shaders();

	unsigned int compile_shader(unsigned int type, const char* source);
	unsigned int create_shader(const char* fragment, const char* vertex);

	void draw_rect(float x, float y, float width, float height, float r, float g, float b, unsigned int VAO);

private:
	Canvas() {};
	static Canvas* instancePtr;
};

#endif;