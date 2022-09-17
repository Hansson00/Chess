#include "SDL.h"
#include "Engine.h"


Engine* engine;

int main(int argc, char* argv[]) {
	engine = new Engine();

	while (engine->running) {
		engine->events();
	}
	delete(engine);

	return 0;
}