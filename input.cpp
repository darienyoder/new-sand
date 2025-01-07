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
	glfwPollEvents();

	for (auto it = keys_pressed.begin(); it != keys_pressed.end();)
	{
		it->second += 1;
		if (it->second == 0)
			it = keys_pressed.erase(it);
		else
			it++;
	}
}

void InputManager::on_key_callback(GLFWwindow* window_, int key, int scancode, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS:
		keys_pressed[key] == 0;
		break;

	case GLFW_RELEASE:
		for (auto it = keys_pressed.begin(); it != keys_pressed.end(); it++)
		{
			if (it->first == key)
			{
				it->second = -input_buffer;
				break; // If keys can't unpress, try removing this break statement.
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
		keys_pressed[button] == 0;
		break;

	case GLFW_RELEASE:
		for (auto it = keys_pressed.begin(); it != keys_pressed.end(); it++)
		{
			if (it->first == button)
			{
				it->second = -input_buffer;
				break; // If keys can't unpress, try removing this break statement.
			}
		}
		break;
	default:
		break;
	}
}

bool InputManager::is_pressed(int input)
{
	for (auto it = keys_pressed.begin(); it != keys_pressed.end(); it++)
	{
		if (it->first == input)
			return it->second >= 0;
	}
	return false;
}

bool InputManager::is_just_pressed(int input)
{
	for (auto it = keys_pressed.begin(); it != keys_pressed.end(); it++)
	{
		if (it->first == input)
		{
			if (it->second > 0 && it->second < input_buffer)
			{
				it->second += input_buffer;
				return true;
			}
			break;
		}
	}
	return false;
}

bool InputManager::is_just_released(int input)
{
	for (auto it = keys_pressed.begin(); it != keys_pressed.end();)
	{
		if (it->first == input && it->second < 0)
		{
			it = keys_pressed.erase(it);
			return true;
		}
		else
			++it;
	}
	return false;
}