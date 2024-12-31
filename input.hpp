#ifndef INPUT_MANAGER
#define INPUT_MANAGER

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <list>

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

	void setup(GLFWwindow* window);
	void update();

	bool is_pressed(int key);
	bool is_just_pressed(int key);
	bool is_just_released(int key);

	void on_key_callback(GLFWwindow* window_, int key, int scancode, int action, int mods);
	void on_cursor_position_callback(GLFWwindow* window_, double xpos, double ypos);
	void on_mouse_button_callback(GLFWwindow* window_, int button, int action, int mods);

	int mouse_x = 0, mouse_y = 0;
	bool mouse_down = false, right_mouse_down = false;
	bool just_clicked = false, just_right_clicked = false;

	int click_time = 0;
	bool click_ready = false;

	bool just_click();

private:
	InputManager() {};
	static InputManager* instancePtr;

	std::list<int> keys_monitering;

	std::list<int> keys_pressed;
	std::list<int> keys_just_pressed;
	std::list<int> keys_just_released;
};

#endif