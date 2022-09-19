#pragma once
#include "Window.h"
#include "SDL.h"


class Engine {
	struct Position {
		uint64_t pieceBoards[12];
		uint64_t teamBoards[3];
		uint64_t whiteAttack;
		uint64_t blackAttack;
		uint64_t pinnedPieces;
		uint64_t validSq;
		bool whiteToMove;
		int castlingRights;
		int numCheckers;
		int enPassant;
	};

	
public:
	Engine();
	~Engine();

	Position pos;

	bool running = true;
	Window* window;

	void events();
	void fenInit(std::string, Position* pos);

private:
	int mouse_x, mouse_y = 0;
	uint8_t held_piece = -1;
	uint64_t held_piece_board = 0;
	void mouse_event(uint8_t button, bool mouse_down);

};




