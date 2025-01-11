#ifndef INPUT_MANAGER
#define INPUT_MANAGER

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <map>

enum {
	MOUSE_LEFT = GLFW_MOUSE_BUTTON_LEFT,
	MOUSE_RIGHT = GLFW_MOUSE_BUTTON_RIGHT,
	MOUSE_MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE,

	KEY_0 = GLFW_KEY_0,
	KEY_1 = GLFW_KEY_1,
	KEY_2 = GLFW_KEY_2,
	KEY_3 = GLFW_KEY_3,
	KEY_4 = GLFW_KEY_4,
	KEY_5 = GLFW_KEY_5,
	KEY_6 = GLFW_KEY_6,
	KEY_7 = GLFW_KEY_7,
	KEY_8 = GLFW_KEY_8,
	KEY_9 = GLFW_KEY_9,

	KEY_A = GLFW_KEY_A,
	KEY_B = GLFW_KEY_B,
	KEY_C = GLFW_KEY_C,
	KEY_D = GLFW_KEY_D,
	KEY_E = GLFW_KEY_E,
	KEY_F = GLFW_KEY_F,
	KEY_G = GLFW_KEY_G,
	KEY_H = GLFW_KEY_H,
	KEY_I = GLFW_KEY_I,
	KEY_J = GLFW_KEY_J,
	KEY_K = GLFW_KEY_K,
	KEY_L = GLFW_KEY_L,
	KEY_M = GLFW_KEY_M,
	KEY_N = GLFW_KEY_N,
	KEY_O = GLFW_KEY_O,
	KEY_P = GLFW_KEY_P,
	KEY_Q = GLFW_KEY_Q,
	KEY_R = GLFW_KEY_R,
	KEY_S = GLFW_KEY_S,
	KEY_T = GLFW_KEY_T,
	KEY_U = GLFW_KEY_U,
	KEY_V = GLFW_KEY_V,
	KEY_W = GLFW_KEY_W,
	KEY_X = GLFW_KEY_X,
	KEY_Y = GLFW_KEY_Y,
	KEY_Z = GLFW_KEY_Z,

	KEY_LEFT = GLFW_KEY_LEFT,
	KEY_RIGHT = GLFW_KEY_RIGHT,
	KEY_UP = GLFW_KEY_UP,
	KEY_DOWN = GLFW_KEY_DOWN,

	KEY_ESCAPE = GLFW_KEY_ESCAPE,
	KEY_TAB = GLFW_KEY_TAB,
	KEY_SHIFT = GLFW_KEY_LEFT_SHIFT,
	KEY_CTRL = GLFW_KEY_LEFT_CONTROL,
	KEY_ALT = GLFW_KEY_LEFT_ALT,
	KEY_SPACE = GLFW_KEY_SPACE,
};

class InputManager
{
public:

	InputManager(const InputManager& obj) = delete;

	static InputManager* getInstance() {
		if (instancePtr == nullptr)
		{
			if (instancePtr == nullptr) {
				instancePtr = new InputManager();
			}
		}
		return instancePtr;
	}

	// Connects auxillary functions
	void setup(GLFWwindow* window);

	// Checks for new input
	void update();


	bool is_pressed(int key);
	bool is_just_pressed(int key);
	bool is_just_released(int key);

	// Auxillary functions for GLFW input
	void on_key_callback(GLFWwindow* window_, int key, int scancode, int action, int mods);
	void on_cursor_position_callback(GLFWwindow* window_, double xpos, double ypos);
	void on_mouse_button_callback(GLFWwindow* window_, int button, int action, int mods);

	// Mouse coordinates; updated every frame
	int mouse_x = 0, mouse_y = 0;

private:
	InputManager() {};
	static InputManager* instancePtr;

	// First value is key; second value is number of ticks since press
	std::map<int, int> keys_pressed;

	// How many frames can count as the first press
	const int input_buffer = 10;
};

extern InputManager* Input;

#endif