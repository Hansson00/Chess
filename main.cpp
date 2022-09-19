#include "SDL.h"
#include "Engine.h"


Engine* engine;

int main(int argc, char* argv[]) {
	engine = new Engine();

	engine->main_loop();
	
	delete(engine);

	return 0;
}