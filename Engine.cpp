#include "Engine.h"

Engine::Engine() {
	SDL_Init(SDL_INIT_EVERYTHING);

	window = new Window(1000, 1000);

	window->draw_board();
	window->draw_piece(0xff000000000000ll, 'P');
	window->draw_piece(0xff00ll, 'p');
	window->update();
}

Engine::~Engine() {
	delete(window);
	SDL_Quit();
}

void Engine::events() {

	SDL_Event sdl_event;
	SDL_PollEvent(&sdl_event);

	switch (sdl_event.type) {
	case SDL_QUIT:
		running = false;
		break;
	default:
		break;
	}
}
