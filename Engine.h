#pragma once
#include "Window.h"
#include "SDL.h"
#include "Events.h"
#include "Position.h"


class Engine : Events{

public:
	Engine();
	~Engine();
	void main_loop();

private:
	Position pos;
	Window* window;

	void fenInit(std::string, Position* pos);


};




