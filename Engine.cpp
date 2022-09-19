#include "Engine.h"

Engine::Engine() {
	SDL_Init(SDL_INIT_EVERYTHING);

    window = new Window(1000, 1000);
       
    fenInit("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq", &pos);
}

Engine::~Engine() {
	delete(window);
	SDL_Quit();
}

void Engine::events() {

	SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event)) {
        switch (sdl_event.type) {
        case SDL_MOUSEBUTTONDOWN:  
            mouse_event(sdl_event.button.button, true);
            break;
        case SDL_MOUSEBUTTONUP:
            mouse_event(sdl_event.button.button, false);
            break;
        case SDL_QUIT:
            running = false;
            break;
        default:
            break;
        }
    }

    window->draw_board();
    window->draw_pieces(pos.pieceBoards);
    if (held_piece != 255) window->draw_piece_at_mouse(held_piece);
    window->update();
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
            case 'b': pos -> pieceBoards[9]|= 1ULL << i; break;
            case 'r': pos -> pieceBoards[10] |= 1ULL << i; break;
            case 'q': pos -> pieceBoards[11]|= 1ULL << i; break;
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

void Engine::mouse_event(uint8_t button, bool mouse_down) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    window->mouse_grid_pos(&x, &y);
    if (button == 1) { // x and y is within the board

        int tmp = (x + y * 8);
        uint64_t mouse_pos = 1Ull << (x + y * 8);

        if (mouse_down && x < 8 && y < 8) { // Pick up piece
            
            for (int i = 0; i < 12; i++)
                if ((mouse_pos & pos.pieceBoards[i]) != 0) {
                    held_piece = i;
                    held_piece_board = pos.pieceBoards[i];
                    pos.pieceBoards[i] -= mouse_pos;
                    break;
                }
        }
        else { // Place piece
            if (x < 8 && y < 8 && held_piece != 255) { // Check if this legal is as well
                for (int i = 0; i < 12; i++) 
                    if ((mouse_pos & pos.pieceBoards[i]) != 0) {
                        pos.pieceBoards[i] -= mouse_pos;
                        break;
                    }
                pos.pieceBoards[held_piece] |= mouse_pos;
            }
            else if(held_piece != 255) {
                pos.pieceBoards[held_piece] = held_piece_board;
            }
            held_piece = 255;
        }
    }
}



