#include "SDL.h"
#include "Chess.h"


Chess* chess;

int main(int argc, char* argv[]) {
	chess = new Chess();

	chess->main_loop();
	
	delete(chess);

	return 0;
}