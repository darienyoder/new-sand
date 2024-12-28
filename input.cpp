#include "input.hpp"

InputManager* InputManager::instancePtr = nullptr;

// Proxy functions

void key_callback(GLFWwindow* window_, int key, int scancode, int action, int mods)
{
	InputManager::getInstance()->on_key_callback(window_, key, scancode, action, mods);
}

static void cursor_position_callback(GLFWwindow* window_, double xpos, double ypos)
{
	InputManager::getInstance()->on_cursor_position_callback(window_, xpos, ypos);
}

void mouse_button_callback(GLFWwindow* window_, int button, int action, int mods)
{
	InputManager::getInstance()->on_mouse_button_callback(window_, button, action, mods);
}

// InputManager methods

void InputManager::setup(GLFWwindow* window)
{
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
}

void InputManager::update()
{
	keys_just_pressed.clear();
	keys_just_released.clear();
	just_clicked = false;
	just_right_clicked = false;

	glfwPollEvents();
}

void InputManager::on_key_callback(GLFWwindow* window_, int key, int scancode, int action, int mods)
{
	bool has = false;
	switch (action)
	{
	case GLFW_PRESS:
		for (std::list<int>::const_iterator pi = keys_pressed.begin(); pi != keys_pressed.end(); ++pi)
		{
			if (*pi == key)
			{
				has = true;
				break;
			}
		}
		if (!has)
		{
			keys_pressed.push_back(key);
			keys_just_pressed.push_back(key);
		}
		break;

	case GLFW_RELEASE:
		for (std::list<int>::const_iterator i = keys_pressed.begin(); i != keys_pressed.end(); ++i)
		{
			if (key == *i)
			{
				keys_pressed.erase(i);
				break;
			}
		}
		break;
	default:
		break;
	}
}

void InputManager::on_cursor_position_callback(GLFWwindow* window_, double xpos, double ypos)
{
	mouse_x = xpos;
	mouse_y = ypos;
}

void InputManager::on_mouse_button_callback(GLFWwindow* window_, int button, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS:
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if (!mouse_down)
				just_clicked = true;
			mouse_down = true;
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			if (!right_mouse_down)
				just_right_clicked = true;
			right_mouse_down = true;
		}
		break;
	case GLFW_RELEASE:
		if (button == GLFW_MOUSE_BUTTON_LEFT)
			mouse_down = false;
		else if (button == GLFW_MOUSE_BUTTON_RIGHT)
			right_mouse_down = false;
		break;
	}
}

bool InputManager::is_pressed(int input)
{
	for (std::list<int>::const_iterator i = keys_pressed.begin(); i != keys_pressed.end(); ++i)
	{
		if (*i == input)
		{
			return true;
		}
	}
	return false;
}

bool InputManager::is_just_pressed(int input)
{
	for (std::list<int>::const_iterator i = keys_just_pressed.begin(); i != keys_just_pressed.end(); ++i)
	{
		if (*i == input)
		{
			return true;
		}
	}
	return false;
}

bool InputManager::is_just_released(int input)
{
	for (std::list<int>::const_iterator i = keys_just_released.begin(); i != keys_just_released.end(); ++i)
	{
		if (*i == input)
		{
			return true;
		}
	}
	return false;
}