#include "Engine.h"

Engine::Engine() {
	SDL_Init(SDL_INIT_EVERYTHING);

    window = new Window(1000, 1000);
    Position pos;
       
    fenInit("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq", &pos);
	window->draw_board();
	window->draw_piece(pos.pieceBoards);
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


void Engine::fenInit(std::string fen, Position* pos) {
    uint32_t i = 0;
    for(int j = 0; j < 12; j++)
        pos->pieceBoards[j] = 0;
    for (char f : fen) {
        if (i < 64) {
            switch (f) {
            case 'K': pos -> pieceBoards[0] |= 1ULL << i; break;
            case 'P': pos -> pieceBoards[1] |= 1ULL << i; break;
            case 'N': pos -> pieceBoards[2] |= 1ULL << i; break;
            case 'B': pos -> pieceBoards[4] |= 1ULL << i; break;
            case 'R': pos -> pieceBoards[3] |= 1ULL << i; break;
            case 'Q': pos -> pieceBoards[5] |= 1ULL << i; break;
            case 'k': pos -> pieceBoards[6] |= 1ULL << i; break;
            case 'p': pos -> pieceBoards[7] |= 1ULL << i; break;
            case 'n': pos -> pieceBoards[8] |= 1ULL << i; break;
            case 'b': pos -> pieceBoards[10] |= 1ULL << i; break;
            case 'r': pos -> pieceBoards[9] |= 1ULL << i; break;
            case 'q': pos -> pieceBoards[11] |= 1ULL << i; break;
            case '/': i--; break;
            default: i += f - 49; break;
            }
            i++;
        }
        else {
            switch (f) {
            case 'w': pos -> whiteToMove = true; break;
            case 'b': pos -> whiteToMove = false; break;
            case 'K': pos -> castlingRights |= 0b0001; break; //White king side castling
            case 'Q': pos -> castlingRights |= 0b0010; break; //White queen side castling
            case 'k': pos -> castlingRights |= 0b0100; break; //Black king side castling
            case 'q': pos -> castlingRights |= 0b1000; break; //Black queen side castling
            default: break;
            }
        
        }
    }
}


