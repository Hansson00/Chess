#pragma once
#include "Window.h"
#include "SDL.h"


class Engine {
public:
	Engine();
	~Engine();

	bool running = true;
	Window* window;

	void events();
};

