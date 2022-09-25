#include "Engine.h"

Engine::Engine() {
    fenInit("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/6NK w KQkq");//"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq");
}

Engine::~Engine() {

}

uint16_t* Engine::get_legal_moves(uint16_t* move_list) {
    uint16_t* temp = move_list;
    temp = generate_pawn_moves(temp, &pos, true);
    *temp = 0;
    return temp;
}

uint64_t Engine::generate_held_piece_moves(uint16_t* move_list, uint16_t p, Position* pos, uint64_t mask) {

    uint16_t piece_pos = long_bit_scan(mask);

    uint16_t* end = (this->*arr[p % 6])(move_list, pos, p < 6);
    *end = 0;
    uint64_t result = 0;
    while (*move_list) {
        if (((*move_list >> 6) & 0x3F )== piece_pos)
            result |= 1ULL << (*move_list & 0x3F);
        move_list++;
    }
    return result;
}



uint64_t Engine::move_squares(uint16_t* moves, uint16_t* end) {
    uint64_t result = 0ULL;
    while (*moves) {
        result |= 1ULL << (*moves);
        moves++;
    }
    return result;
}




void Engine::fenInit(std::string fen) {
    uint32_t i = 0;
    memset(&pos, 0, sizeof(pos));
    for (char f : fen) {
        if (i < 64) {
            switch (f) {
            case 'K': pos.pieceBoards[0] |= 1ULL << i; break;
            case 'P': pos.pieceBoards[1] |= 1ULL << i; break;
            case 'N': pos.pieceBoards[2] |= 1ULL << i; break;
            case 'B': pos.pieceBoards[3] |= 1ULL << i; break;
            case 'R': pos.pieceBoards[4] |= 1ULL << i; break;
            case 'Q': pos.pieceBoards[5] |= 1ULL << i; break;
            case 'k': pos.pieceBoards[6] |= 1ULL << i; break;
            case 'p': pos.pieceBoards[7] |= 1ULL << i; break;
            case 'n': pos.pieceBoards[8] |= 1ULL << i; break;
            case 'b': pos.pieceBoards[9]|= 1ULL << i; break;
            case 'r': pos.pieceBoards[10] |= 1ULL << i; break;
            case 'q': pos.pieceBoards[11]|= 1ULL << i; break;
            case '/': i--; break;
            default: i += f - 49; break;
            }
            i++;
        }
        else {
            switch (f) {
            case 'w': pos.whiteToMove = true; break;
            case 'b': pos.whiteToMove = false; break;
            case 'K': pos.castlingRights |= 0b0001; break; //White king side castling
            case 'Q': pos.castlingRights |= 0b0010; break; //White queen side castling
            case 'k': pos.castlingRights |= 0b0100; break; //Black king side castling
            case 'q': pos.castlingRights |= 0b1000; break; //Black queen side castling
            default: break;
            }
        
        }
    }
    pos.enPassant = 40;
    pos.whiteAttack = 0;
    pos.teamBoards[1] = pos.pieceBoards[0] | pos.pieceBoards[1] | pos.pieceBoards[2] |
                        pos.pieceBoards[3] | pos.pieceBoards[4] | pos.pieceBoards[5];
    pos.teamBoards[2] = pos.pieceBoards[6] | pos.pieceBoards[7] | pos.pieceBoards[8] |
                        pos.pieceBoards[9] | pos.pieceBoards[10] | pos.pieceBoards[11];
    pos.teamBoards[0] = pos.teamBoards[1] | pos.teamBoards[2];
}



