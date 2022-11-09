#include "Chess.h"
#include "Engine.h"
#include "Move_Generator.h"

Chess::Chess(Uint16 width, uint16_t height) {
    SDL_Init(SDL_INIT_EVERYTHING);
    window = new Window(width, height);
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
    //window->draw_texture_at_square(engine->move_squares(engine->pos.whiteAttack, window->attack_square);
    window->draw_texture_at_square(engine->pos.pinnedPieces, window->attack_square);
    window->draw_pieces(engine->pos.pieceBoards, held_piece_board);
    if (held_piece != 255) {
        window->draw_piece_at_mouse(held_piece);
        window->draw_texture_at_square(engine->generate_held_piece_moves(held_piece, &(engine->pos),held_piece_board), window->legal_circle);
    }
    window->draw_eval_bar(eval_test);
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
                    held_piece_board = mouse_pos;
                    //chagne_bitboards(i, 0, mouse_pos);
                    break;
                }
        }
        else { // Place piece
            if (x < 8 && y < 8 && held_piece != 255) { // Check if this legal is as well
                Move_Generator::Move_list move_list;
                memset(move_list.move_list, 0, 60 * sizeof(uint16_t));
                move_list.last = move_list.move_list;
                engine->get_legal_moves(&move_list);
                const uint16_t move = (uint16_t)(tmp | long_bit_scan(held_piece_board) << 6);
                const uint16_t real_move = move_list.contains(move);
                if (real_move != 0) {
                    engine->make_move(&(engine->pos), real_move);
                    switch (engine->sound) {
                    case Engine::s_move: sound_manager->play_sound(sound_manager->move); break;
                    case Engine::s_capture: sound_manager->play_sound(sound_manager->capture); break;
                    case Engine::s_castle: sound_manager->play_sound(sound_manager->castle); break;
                    case Engine::s_check: sound_manager->play_sound(sound_manager->check); break;
                    }
                }
                

                /*
                chagne_bitboards(held_piece, 0, held_piece_board);
                for (int i = 0; i < 12; i++)
                    if ((mouse_pos & engine->pos.pieceBoards[i]) != 0) {
                        chagne_bitboards(i, 0, mouse_pos);
                        break;
                    }
                chagne_bitboards(held_piece, mouse_pos, 0);
                
                */
                
            }
            held_piece = 255;
            held_piece_board = 0;

            //EVAL TEST!!!
            eval_test += 0.4534;
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
    engine->update_attack(&(engine->pos));
}


