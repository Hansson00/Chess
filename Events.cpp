#include "Events.h"

void Events::events(Window* window, Position* pos) {

    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event)) {
        switch (sdl_event.type) {
        case SDL_MOUSEBUTTONDOWN:
            mouse_event(sdl_event.button.button, true, window, pos);
            break;
        case SDL_MOUSEBUTTONUP:
            mouse_event(sdl_event.button.button, false, window, pos);
            break;
        case SDL_QUIT:
            running = false;
            break;
        default:
            break;
        }
    }
}


void Events::mouse_event(uint8_t button, bool mouse_down, Window* window, Position* pos) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    window->mouse_grid_pos(&x, &y);
    if (button == 1) { // x and y is within the board

        int tmp = (x + y * 8);
        uint64_t mouse_pos = 1Ull << (x + y * 8);

        if (mouse_down && x < 8 && y < 8) { // Pick up piece

            for (int i = 0; i < 12; i++)
                if ((mouse_pos & pos->pieceBoards[i]) != 0) {
                    held_piece = i;
                    held_piece_board = pos->pieceBoards[i];
                    pos->pieceBoards[i] -= mouse_pos;
                    break;
                }
        }
        else { // Place piece
            if (x < 8 && y < 8 && held_piece != 255) { // Check if this legal is as well
                for (int i = 0; i < 12; i++)
                    if ((mouse_pos & pos->pieceBoards[i]) != 0) {
                        pos->pieceBoards[i] -= mouse_pos;
                        break;
                    }
                pos->pieceBoards[held_piece] |= mouse_pos;
            }
            else if (held_piece != 255) {
                pos->pieceBoards[held_piece] = held_piece_board;
            }
            held_piece = 255;
        }
    }
}