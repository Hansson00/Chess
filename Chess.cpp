#include "Chess.h"
#include "Engine.h"

Chess::Chess() {
    SDL_Init(SDL_INIT_EVERYTHING);
    window = new Window(1000, 1000);
    sound_manager = new Sound_Manager();
    engine = new Engine();

}

void Chess::main_loop() {

    while (running) {
        events();
        draw();


        
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

void Chess::draw() {
    window->draw_board();
    uint16_t move_list[40];
    window->draw_attack_sqaure(engine->move_squares(move_list, engine->get_legal_moves(move_list)));
    window->draw_pieces(engine->pos.pieceBoards);
    if (held_piece != 255) window->draw_piece_at_mouse(held_piece);
    window->update();
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
                    chagne_bitboards(i, 0, mouse_pos);
                    break;
                }
        }
        else { // Place piece
            if (x < 8 && y < 8 && held_piece != 255) { // Check if this legal is as well
                for (int i = 0; i < 12; i++)
                    if ((mouse_pos & engine->pos.pieceBoards[i]) != 0) {
                        chagne_bitboards(i, 0, mouse_pos);
                        break;
                    }
                chagne_bitboards(held_piece, mouse_pos, 0);
                sound_manager->play_sound(sound_manager->move);
            }
            else if(held_piece != 255) {
                engine->pos.pieceBoards[held_piece] = held_piece_board;
            }
            held_piece = 255;
        }
    }
}

void Chess::chagne_bitboards(uint32_t p, uint64_t add, uint64_t remove) {

    if (add > 0) {
        engine->pos.pieceBoards[p] |= add;
        engine->pos.teamBoards[0] |= add;
        if (p < 6)
            engine->pos.teamBoards[1] |= add;
        else
            engine->pos.teamBoards[2] |= add;
    }
    if (remove > 0) {
        engine->pos.pieceBoards[p] &= ~remove;
        engine->pos.teamBoards[0] &= ~remove;
        if (p < 6)
            engine->pos.teamBoards[1] &= ~remove;
        else
            engine->pos.teamBoards[2] &= ~remove;
    }
    

}


