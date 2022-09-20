#include "Chess.h"
#include "Engine.h"

Chess::Chess() {
    SDL_Init(SDL_INIT_EVERYTHING);
    window = new Window(1000, 1000);
    sound = new Sound_Manager();
    engine = new Engine();

}

void Chess::main_loop() {

    while (running) {
        events();
        // make a draw function
        window->draw_board();
        window->draw_attack_sqaure(20, 20);
        window->draw_pieces(engine->pos.pieceBoards);
        if (held_piece != 255) window->draw_piece_at_mouse(held_piece);
        window->update();
    }
}

void Chess::events() {

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
}

void Chess::mouse_event(uint8_t button, bool mouse_down) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    window->mouse_grid_pos(&x, &y);
    if (button == 1) { // x and y is within the board

        int tmp = (x + y * 8);
        uint64_t mouse_pos = 1Ull << (x + y * 8);

        if (mouse_down && x < 8 && y < 8) { // Pick up piece

            for (int i = 0; i < 12; i++)
                if ((mouse_pos & engine->pos.pieceBoards[i]) != 0) {
                    held_piece = i;
                    held_piece_board = engine->pos.pieceBoards[i];
                    engine->pos.pieceBoards[i] -= mouse_pos;
                    break;
                }
        }
        else { // Place piece
            if (x < 8 && y < 8 && held_piece != 255) { // Check if this legal is as well
                for (int i = 0; i < 12; i++)
                    if ((mouse_pos & engine->pos.pieceBoards[i]) != 0) {
                        engine->pos.pieceBoards[i] -= mouse_pos;
                        break;
                    }
                engine->pos.pieceBoards[held_piece] |= mouse_pos;
                sound->play_sound(sound->move);
            }
            else if(held_piece != 255) {
                engine->pos.pieceBoards[held_piece] = held_piece_board;
            }
            held_piece = 255;
        }
    }
}