#pragma once
#include "Window.h"
#include "SDL.h"
#include "Position.h"
#include "Move_Generator.h"


class Engine : private Move_Generator {

public:
	Engine();
	~Engine();
	Position pos;
private:
	void fenInit(std::string);

};
