#include "SDL.h"
#include "Chess.h"


Chess* chess;

#define DEBUGWIN

#ifdef DEBUGWIN
#define WIDTH 1300
#define HEIGHT 1000 
#else
#define WIDTH 1000
#define HEIGHT 1000 
#endif


int main(int argc, char* argv[]) {
	chess = new Chess(WIDTH, HEIGHT);

	chess->main_loop();
	
	delete(chess);
	atexit(SDL_Quit);
	exit( EXIT_SUCCESS);
	return 0;
}