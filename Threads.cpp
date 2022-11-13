#include "Threads.h"

void T_perft(int depth, Position* pos, Move_Generator* mg) {
    uint64_t num_positions = 0;
    Position prev = *pos;
    

    Move_list legal_moves;
    get_legal_moves(&prev, &legal_moves, mg);

    auto futures = new std::future<uint64_t>[legal_moves.size()];
    auto positions = new Position*[legal_moves.size()];

    int i = 0;

    for (uint16_t* move = legal_moves.start(); move < legal_moves.end(); move++) {
        //Print the move being made
        //parse_move(*move);

        positions[i] = new Position;
        *positions[i] = prev;
        make_move(positions[i], *move, mg);

        futures[i] = std::async(std::launch::async, search, depth - 1, positions[i], mg);
        //std::cout << typeid(furute).name() << std::endl;

        //uint64_t part = search(depth - 1, &current, mg);

        //Print how many positions are reached after the move
        //std::cout << part << std::endl;
        //num_positions += part;
        //undo_move(&current, &prev);
        i++;
    }
       
    for (int j = 0; j < i; j++) {
        num_positions += futures[j].get();
        delete(positions[j]);
    }
    delete(positions);

    std::cout << "Total: ";
    std::cout << num_positions << std::endl;


}

uint64_t search(int depth, Position* prev_pos, Move_Generator* mg) {

    if (depth == 0)
        return 1;

    uint64_t num_positions = 0;
    Position prev = *prev_pos;
    Position current = prev;
    //Get legal moves for current position
    Move_list legal_moves;
    get_legal_moves(&prev, &legal_moves, mg);

    if (depth == 1)
        return (uint64_t)legal_moves.size();

    for (uint16_t* move = legal_moves.start(); move < legal_moves.end(); move++) {
        make_move(&current, *move, mg);
        num_positions += search(depth - 1, &current, mg);
        undo_move(&current, &prev);
    }
    return num_positions;
}

void get_legal_moves(Position* pos, Move_list* move_list, Move_Generator* mg) {

    mg->generate_king_moves(move_list, pos);
    mg->generate_pawn_moves(move_list, pos);
    mg->generate_knight_moves(move_list, pos);
    mg->generate_bishop_moves(move_list, pos);
    mg->generate_rook_moves(move_list, pos);
    mg->generate_queen_moves(move_list, pos);

    if (pos->pinnedPieces != 0) { //Filter out pinned moves
        filter_pins(move_list, pos, mg);
    }
}

void filter_pins(Move_list* move_list, Position* pos, Move_Generator* mg) {
    uint16_t filtered_moves[60];
    const uint64_t pinned = pos->pinnedPieces;
    const int king_pos = pos->whiteToMove ? long_bit_scan(pos->pieceBoards[0]) : long_bit_scan(pos->pieceBoards[6]);

    //If the from square is on the pinned board then it might need to be filtered
    int filter_size = 0;
    for (int i = 0; i < move_list->size(); i++) {
        const uint16_t move_from = move_list->from_sq(move_list->move_list[i]);
        if (((1ULL << move_from) & pinned) != 0) {
            const uint64_t move_to = 1ULL << move_list->to_sq(move_list->move_list[i]);
            const uint64_t ray = pinned_ray(king_pos, move_from, mg);
            if ((move_to & ray) != 0) {
                filtered_moves[filter_size++] = move_list->move_list[i];
            }
        }
        else {
            filtered_moves[filter_size++] = move_list->move_list[i];
        }
    }
    //Copy back the filtered moves to the move_list and update last
    memcpy(move_list->move_list, filtered_moves, filter_size * sizeof(uint16_t));
    move_list->last = move_list->move_list + filter_size;
}

const uint64_t pinned_ray(int king, int piece, Move_Generator* mg) {
    //On the same rank
    if (king / 8 == piece / 8)
        return mg->ranks[king / 8];
    //On the same file
    if (king % 8 == piece % 8)
        return mg->files[king % 8];
    const int diagonal = king / 8 + king % 8;
    //On the same anti-diagonal
    if (diagonal == piece / 8 + piece % 8)
        return mg->anti_diagonals[diagonal];
    //Otherwise on the same main-diagonal
    return mg->main_diagonals[7 - king / 8 + king % 8];
}

void parse_move(uint16_t move) {
    int from = (move >> 6) & 0x3F;
    int to = move & 0x3F;
    std::cout << (char)(from % 8 + 'a') << (char)('8' - from / 8)
        << (char)('a' + to % 8) << (char)('8' - to / 8) << ':';
}

void undo_move(Position* current, Position* prev) {
    *current = *prev;
}

void make_move(Position* pos, uint16_t move, Move_Generator* mg) {
    const bool white_to_move = pos->whiteToMove;
    const uint16_t piece_offset = white_to_move ? 0 : 6;
    const uint16_t them_offset = white_to_move ? 6 : 0;
    const uint64_t from_sq = 1ULL << ((move >> 6) & 0x3F);
    const uint64_t to_sq = 1ULL << (move & 0x3F);
    const uint16_t flags = move >> 12;
    const int team = 1 + piece_offset / 6;
    const int enemy = team ^ 3; //enemy = team == 1 ? 2 : 1;


    //TODO: Find better way to know which piece made the move, maybe save as flag in move itself
    int pieceID = pos->find_mask(from_sq, piece_offset, piece_offset + 6);
    pos->pieceBoards[pieceID] &= ~from_sq;

    if (flags == 5) { //En passant capture
        uint64_t realPawn = ~(1ULL << (pos->enPassant >> 8));
        pos->pieceBoards[them_offset + 1] &= realPawn; //Remove real pawn
        pos->teamBoards[enemy] &= realPawn;
        pos->teamBoards[0] &= realPawn;
    }
    else if (flags == 1) { //Double push
        en_passant(pos, move & 0x3F, mg);
    }
    else {
        pos->enPassant = 0xFF; //Disable en passant
    }
    if (flags == 2 || flags == 3) { //Castling move
        const uint64_t rook = flags == 2 ? to_sq << 1 : to_sq >> 2;
        const uint64_t to = flags == 2 ? to_sq >> 1 : to_sq << 1;
        castle(pos, ~rook, to); //Clear rook and move to "to" square
    }
    else if ((flags & 4) == 4) { //Regular capture
        int capID = pos->find_mask(to_sq, them_offset + 1, them_offset + 6);
        //TODO: If rook gets captured change castling rights
        pos->pieceBoards[capID] &= ~to_sq;
        pos->teamBoards[enemy] &= ~to_sq;
        //TODO: store either captured piece or save entire gamestate
    }
    if (flags > 7) { //Promotion
        pieceID = piece_offset + 2 + (flags & 0x3); //Change which bitboard should get the set bit
    }

    //Rook moves disable castling rights
    if (pieceID - piece_offset == 4) {
        const int shift = white_to_move ? 0 : 2;
        //If the from square is on the 7th file, i.e. king castling
        const int set = ((move >> 6) & 0b111) == 7 ? 0b10 << shift : 0b01 << shift;
        pos->castlingRights &= set;
    }
    //King moves disable castling rights
    else if (pieceID - piece_offset == 0) {
        const int set = piece_offset == 0 ? 0b1100 : 0b0011;
        pos->castlingRights &= set;
    }


    //Move the piece in all bitboards
    pos->pieceBoards[pieceID] |= to_sq; //Setting bit is done after promotion check because promoting fill change pieceID
    pos->teamBoards[team] &= ~from_sq;
    pos->teamBoards[team] |= to_sq;
    pos->teamBoards[0] &= ~from_sq;
    pos->teamBoards[0] |= to_sq;

    pos->whiteToMove = !pos->whiteToMove;
    update_attack(pos, mg);
}

void en_passant(Position* pos, int pushedPawn, Move_Generator* mg) {
    //White double push
    if (pos->whiteToMove) {
        //If no black pawns present on 5th rank then no en passant possible
        if ((pos->pieceBoards[7] & 0xFF00000000L) == 0)
            return;
        uint64_t sliders = pos->pieceBoards[4] | pos->pieceBoards[5];
        //If any of the slider occupy the fifth rank then we need to check for pin
        if ((sliders & 0xFF00000000L) > 0) {
            uint64_t slideAttack = mg->piece_attacks(pos->teamBoards[0], sliders, 1);
            uint64_t kingAttack = mg->piece_attacks(pos->teamBoards[0], pos->pieceBoards[6], 1);
            uint64_t pawnPos = 1ULL << pushedPawn;
            //If either the king or sliders see the en passant pawn then a possible pin is there
            if ((slideAttack & pawnPos) != 0) {
                //If the king sees the piece next to the pawn then there is no en passant available
                if ((kingAttack & (pawnPos >> 1)) != 0 || (kingAttack & (pawnPos << 1)) != 0) {
                    return;
                }
            }
            else if ((kingAttack & pawnPos) != 0) {
                //If the slider sees pawn next to pushed pawn then no en passant available
                if ((slideAttack & (pawnPos >> 1)) != 0 || (slideAttack & (pawnPos << 1)) != 0) {
                    return;
                }
            }
        }
    }
    else {
        //If no white pawns present on 4th rank then no en passant possible
        if ((pos->pieceBoards[1] & 0xFF000000L) == 0)
            return;
        uint64_t sliders = pos->pieceBoards[10] | pos->pieceBoards[11];
        //If any of the slider occupy the forth rank then we need to check for pin
        if ((sliders & 0xFF000000L) > 0) {
            uint64_t slideAttack = mg->piece_attacks(pos->teamBoards[0], sliders, 1);
            uint64_t kingAttack = mg->piece_attacks(pos->teamBoards[0], pos->pieceBoards[0], 1);
            uint64_t pawnPos = 1ULL << pushedPawn;
            //If either the king or sliders see the en passant pawn then a possible pin is there
            if ((slideAttack & pawnPos) != 0) {
                //If the king sees the piece next to the pawn then there is no en passant available
                if ((kingAttack & (pawnPos >> 1)) != 0 || (kingAttack & (pawnPos << 1)) != 0) {
                    return;
                }
            }
            else if ((kingAttack & pawnPos) != 0) {
                //If the slider sees pawn next to pushed pawn then no en passant available
                if ((slideAttack & (pawnPos >> 1)) != 0 || (slideAttack & (pawnPos << 1)) != 0) {
                    return;
                }
            }
        }
    }
    int pseudo = pos->whiteToMove ? pushedPawn + 8 : pushedPawn - 8;
    //Stores both real pawn and pseudoPawn
    pos->enPassant = (pushedPawn << 8) | pseudo;
}

void castle(Position* pos, uint64_t rook, uint64_t to) {
    const int team = to > 0xFFF ? 1 : 2;
    const int rook_id = to > 0xFFF ? 4 : 10;

    pos->teamBoards[0] &= rook;
    pos->teamBoards[0] |= to;
    pos->teamBoards[team] &= rook;
    pos->teamBoards[team] |= to;
    pos->pieceBoards[rook_id] &= rook;
    pos->pieceBoards[rook_id] |= to;

}

void update_attack(Position* pos, Move_Generator* mg) {
    pos->numCheckers = 0;
    pos->pinnedPieces = 0ULL;
    pos->checker = 0;
    pos->block_check = 0;
    if (pos->whiteToMove) { //Only need to update black attack
        uint64_t black_attack = 0ULL;
        const int king_pos = long_bit_scan(pos->pieceBoards[6]);
        const uint64_t board_without_king = pos->teamBoards[0] & ~pos->pieceBoards[0];
        black_attack |= mg->king_attacks[king_pos];
        black_attack |= mg->pawn_attacks(pos->pieceBoards[7], false);
        black_attack |= mg->piece_attacks(pos->teamBoards[0], pos->pieceBoards[8], 0);
        black_attack |= mg->piece_attacks(board_without_king, pos->pieceBoards[9], 1);
        black_attack |= mg->piece_attacks(board_without_king, pos->pieceBoards[10], 2);
        black_attack |= mg->piece_attacks(board_without_king, pos->pieceBoards[11], 3);
        pos->blackAttack = black_attack;
        if ((black_attack & pos->pieceBoards[0]) != 0) { //Intersection with king gives check
            in_check_masks(pos, true, mg);
        }
    }
    else {
        uint64_t white_attack = 0ULL;
        const int king_pos = long_bit_scan(pos->pieceBoards[0]);
        const uint64_t board_without_king = pos->teamBoards[0] & ~pos->pieceBoards[6];
        white_attack |= mg->king_attacks[king_pos];
        white_attack |= mg->pawn_attacks(pos->pieceBoards[1], true);
        white_attack |= mg->piece_attacks(pos->teamBoards[0], pos->pieceBoards[2], 0);
        white_attack |= mg->piece_attacks(board_without_king, pos->pieceBoards[3], 1);
        white_attack |= mg->piece_attacks(board_without_king, pos->pieceBoards[4], 2);
        white_attack |= mg->piece_attacks(board_without_king, pos->pieceBoards[5], 3);
        pos->whiteAttack = white_attack;
        if ((white_attack & pos->pieceBoards[6]) != 0) { //Intersection with king gives check
            in_check_masks(pos, false, mg);
        }
    }
    find_pins(pos, mg);
}

void in_check_masks(Position* pos, bool white_in_check, Move_Generator* mg) {
    const int piece_offset = white_in_check ? 6 : 0;
    const uint64_t board = pos->teamBoards[0];
    const uint64_t king = white_in_check ? pos->pieceBoards[0] : pos->pieceBoards[6];
    uint64_t checker = 0ULL;
    uint64_t block = 0ULL;

    uint64_t pawnCheck = mg->pawn_attacks(king, white_in_check) & pos->pieceBoards[1 + piece_offset];
    uint64_t knightCheck = mg->piece_attacks(board, king, 0) & pos->pieceBoards[2 + piece_offset];
    uint64_t diagonalCheck = mg->piece_attacks(board, king, 1) & (pos->pieceBoards[3 + piece_offset] | pos->pieceBoards[5 + piece_offset]);
    uint64_t straightCheck = mg->piece_attacks(board, king, 2) & (pos->pieceBoards[4 + piece_offset] | pos->pieceBoards[5 + piece_offset]);

    //If there are more than one checker this will not matter since only king moves are possible
    if (knightCheck != 0) { checker = knightCheck; pos->numCheckers++; }
    else if (pawnCheck != 0) { checker = pawnCheck; pos->numCheckers++; }
    if (diagonalCheck != 0) {
        //Bishop or queen is checking
        checker = diagonalCheck;
        //Find block mask
        block = find_block(long_bit_scan(king), long_bit_scan(diagonalCheck));
        pos->numCheckers++;
    }
    if (straightCheck != 0) {
        //Rook or queen check
        checker = straightCheck;
        //Find block mask
        block = find_block(long_bit_scan(king), long_bit_scan(straightCheck));
        pos->numCheckers++;
    }
    pos->block_check = block;
    pos->checker = checker;
}

uint64_t find_block(uint32_t king, uint32_t checker) {
    int dir;
    if (king % 8 == checker % 8) {
        dir = king > checker ? -8 : 8;
    }
    else if (king / 8 == checker / 8) {
        dir = king > checker ? -1 : 1;
    }
    else if (king / 8 + king % 8 == checker / 8 + checker % 8) {
        dir = king > checker ? -7 : 7;
    }
    else {
        dir = king > checker ? -9 : 9;
    }
    king += dir;
    uint64_t block = 0ULL;
    //Traverse towards checker, block does not contain checker
    while (king != checker) {
        block |= 1ULL << king;
        king += dir;
    }
    return block;
}

void find_pins(Position* pos, Move_Generator* mg) {
    const bool white_to_move = pos->whiteToMove;
    const int start = pos->whiteToMove ? 0 : 6;
    const uint64_t king_board = pos->pieceBoards[start];
    const int king_pos = long_bit_scan(king_board);
    const uint64_t straight = white_to_move ? pos->pieceBoards[10] | pos->pieceBoards[11] : pos->pieceBoards[4] | pos->pieceBoards[5];
    const uint64_t diagonal = white_to_move ? pos->pieceBoards[9] | pos->pieceBoards[11] : pos->pieceBoards[3] | pos->pieceBoards[5];
    const uint64_t king_file = mg->files[king_pos % 8];
    const uint64_t king_rank = mg->ranks[king_pos / 8];
    const uint64_t kingMD = mg->main_diagonals[7 + king_pos % 8 - king_pos / 8];
    const uint64_t kingAD = mg->anti_diagonals[king_pos % 8 + king_pos / 8];

    //Possible pinning pieces are aligned with the king
    const uint64_t relevant_main = kingMD & diagonal;
    const uint64_t relevant_anti = kingAD & diagonal;
    const uint64_t relevant_rank = king_rank & straight;
    const uint64_t relevant_file = king_file & straight;

    //Possible pinning pieces, INVESTIGATE
    uint64_t nSameMask = white_to_move ? ~pos->teamBoards[2] : ~pos->teamBoards[1];
    const uint64_t board = pos->teamBoards[0];
    if (relevant_main != 0) {
        const uint64_t piece = mg->piece_attacks(board, relevant_main, 4) & nSameMask;
        const uint64_t king = mg->piece_attacks(board, king_board, 4);
        pos->pinnedPieces |= piece & king; //If the king and the sliding pieces attack the same piece then it is pinned
    }
    if (relevant_anti != 0) {
        const uint64_t piece = mg->piece_attacks(board, relevant_anti, 5) & nSameMask;
        const uint64_t king = mg->piece_attacks(board, king_board, 5);
        pos->pinnedPieces |= piece & king; //If the king and the sliding pieces attack the same piece then it is pinned
    }
    if (relevant_rank != 0) {
        const uint64_t piece = mg->piece_attacks(board, relevant_rank, 6) & nSameMask;
        const uint64_t king = mg->piece_attacks(board, king_board, 6);
        pos->pinnedPieces |= piece & king; //If the king and the sliding pieces attack the same piece then it is pinned
    }
    if (relevant_file != 0) {
        const uint64_t piece = mg->piece_attacks(board, relevant_file, 7) & nSameMask;
        const uint64_t king = mg->piece_attacks(board, king_board, 7);
        pos->pinnedPieces |= piece & king; //If the king and the sliding pieces attack the same piece then it is pinned
    }
}