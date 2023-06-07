#include "Chess.h"


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
        command_promt();
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

        case SDL_KEYUP:
            switch (sdl_event.key.keysym.sym) {
            case SDLK_SPACE:
                engine->player_undo_move();
                break;
            case SDLK_BACKSPACE:
                window->flip = !window->flip;
                break;
            case SDLK_1:
                //engine->player_make_move(generate_moves(engine->pos));
                //play_sound();
                break;
            case SDLK_2:
                engine->player_make_move(engine->find_best_move(12, &engine->pos));
                play_sound();
                break;
            case SDLK_3:
                engine->player_make_move(engine->find_best_move(3, &engine->pos));
                play_sound();
                break;
            case SDLK_4:
                engine->player_make_move(engine->find_best_move(4, &engine->pos));
                play_sound();
                break;
            case SDLK_5:
                engine->player_make_move(engine->find_best_move(5, &engine->pos));
                play_sound();
                break;
            case SDLK_6:
                engine->player_make_move(engine->find_best_move(6, &engine->pos));
                play_sound();
                break;
            case SDLK_7:
                engine->player_make_move(engine->find_best_move(7, &engine->pos));
                play_sound();
                break;
            case SDLK_8:
                engine->player_make_move(engine->find_best_move(8, &engine->pos));
                play_sound();
                break;
            case SDLK_9:
                engine->player_make_move(engine->find_best_move(9, &engine->pos));
                play_sound();
                break;
            case SDLK_0:
                engine->player_make_move(engine->find_best_move(10, &engine->pos));
                play_sound();
                break;
            default:
                break;
            }
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
    window->draw_texture_at_square(1ULL << (engine -> move_highlight & 0x3F), window->move_square);
    window->draw_texture_at_square(1ULL << (engine -> move_highlight >> 6), window->move_square);
    window->draw_texture_at_square(engine->pos.king_squares, window->move_square);
    
    window->draw_pieces(engine->pos.pieceBoards, held_piece_board);
    if (held_piece != 255) {
        window->draw_piece_at_mouse(held_piece);
        window->draw_texture_at_square(engine->generate_held_piece_moves(held_piece, &(engine->pos), held_piece_board), window->legal_circle);
    }
    //window->draw_eval_bar(eval_test);
    window->update();
    
}

void Chess::mouse_event(uint8_t button, bool mouse_down) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    window->mouse_grid_pos(&x, &y);
    if (button == 1) { // x and y is within the board

        int tmp = window->flip ? 63 - (x + y * 8) : (x + y * 8);
        uint64_t mouse_pos = 1Ull << tmp;

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
                Move_list move_list;
                engine->get_legal_moves(&engine->pos, &move_list);
                const uint32_t mouse_move = (uint32_t)(tmp | long_bit_scan(held_piece_board) << 6);
                const uint32_t real_move = move_list.contains(mouse_move);
                if (real_move != 0) {
                    engine->player_make_move(real_move);
                    play_sound();
                    //engine->player_make_move(engine->find_best_move(6, &engine->pos));
                    //play_sound();
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

void Chess::play_sound() {
    switch (engine->sound) {
    case Engine::s_move: sound_manager->play_sound(sound_manager->move); break;
    case Engine::s_capture: sound_manager->play_sound(sound_manager->capture); break;
    case Engine::s_castle: sound_manager->play_sound(sound_manager->castle); break;
    case Engine::s_check: sound_manager->play_sound(sound_manager->check); break;
    case Engine::s_checkmate: sound_manager->play_sound(sound_manager->checkmate); break;
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

void Chess::command_promt() {

    static bool bl = false;
    static std::future<std::string> future = std::async(get_input, &bl);

    if (bl) {
        std::string str = future.get();
        bl = false;
       
        auto start = std::chrono::system_clock::now();
        if (std::strcmp(str.c_str(), "perft 1") == 0) { compare_perft(1, 1486); }
        else if (std::strcmp(str.c_str(), "perft 2") == 0) { compare_perft(2, 1486); }
        else if (std::strcmp(str.c_str(), "perft 3") == 0) { compare_perft(3, 62379); }
        else if (std::strcmp(str.c_str(), "perft 4") == 0) { compare_perft(4, 2103487); }
        else if (std::strcmp(str.c_str(), "perft 5") == 0) { compare_perft(5, 89941194); }
        else if (std::strcmp(str.c_str(), "perft 6") == 0) { compare_perft(6, 3048196529); }
        else if (std::strcmp(str.c_str(), "perft 7") == 0) { compare_perft(7, 3195901860); }
        else if (std::strcmp(str.c_str(), "perft 8") == 0) { compare_perft(8, 84998978956); }
        else if (std::strcmp(str.c_str(), "moves") == 0) { engine->op->find_book_move(&engine->pos); }
        else if (std::strcmp(str.c_str(), "hash") == 0) { std::cout << std::hex << engine->op->hash_funciton(&engine->pos) << std::endl; }
        //else if (std::strcmp(str.c_str(), "hash2") == 0) { std::cout << std::hex << engine->zobrist_hash(&engine->pos) << std::endl; }
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "Time:" << diff.count() << std::endl;

        future = std::async(get_input, &bl);
    }
}

void Chess::compare_perft(int depth, int64_t positions) {

    int64_t perft = engine->perft(depth, &engine->pos);
    if (positions == perft) {
        std::cout << "100% correct\n" << std::endl;
    }
    else {
        std::cout << std::abs(positions- perft) << " incorrect\n" << std::endl;
    }
}

static std::string get_input(bool* bl) {
    std::string answer;
    std::getline(std::cin, answer);
    *bl = true;
    return answer;
}
/*
uint32_t Chess::generate_moves(const Position& pos)
{
    using namespace std::chrono;
    auto clock = steady_clock::now();
    uint32_t deepest = -999999;
    bool done = false;
    bool run = true;
    int depth = 6;
    std::future<uint32_t> future = std::async(std::launch::async, &Chess::_generate_moves_wrapper, this, pos, depth, &done, &run);

    while (duration_cast<seconds>(steady_clock::now() - clock).count() < 10)
    {
        if (done)
        {
            std::cout << "Depth " << depth << " done." << std::endl;
            deepest = future.get();
            done = false;
            depth++;
            future = std::async(std::launch::async, &Chess::_generate_moves_wrapper, this, pos, depth, &done, &run);
        }
    }
    run = false;
    std::cout << "Finished at a depth of: " << depth - 1 << std::endl << std::endl;
    return deepest;

}*/

/*
* uint32_t Chess::_generate_moves_wrapper(const Position& pos, int depth, bool* done, bool* run)
{
    uint32_t eval = engine->find_best_move_th(depth, &engine->pos, run);
    *done = true;
    return eval;
}
*/
