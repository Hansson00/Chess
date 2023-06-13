#include "Engine.h"
using namespace std;

Engine::Engine() {

    fenInit(&pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    //fenInit(&pos, "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2");
    //fenInit(&pos, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    //fenInit(&pos, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
    //rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2;

    init_hashtable(&pos);
    ob = new Opening_book();

}

Engine::~Engine() {

}



//Generates all legal move in current game state, and stores them in provided Move_list
void Engine::get_legal_moves(Position* pos, Move_list* move_list) {
    generate_king_moves(move_list, pos);
    if (pos->numCheckers > 1) //Only king moves are legal
        return;
    generate_pawn_moves(move_list, pos);
    generate_knight_moves(move_list, pos);
    generate_bishop_moves(move_list, pos);
    generate_rook_moves(move_list, pos);
    generate_queen_moves(move_list, pos);

    if (pos->pinnedPieces != 0) { //Filter out pinned moves
        filter_pins(move_list, pos);
    }
}

//Return the bitboard of moves for a given piece, used to draw move "dots"
uint64_t Engine::generate_held_piece_moves(uint16_t p_type, Position* pos, uint64_t mask) {

    Move_list move_list;
    (this->*arr[p_type % 6])(&move_list, pos);
    if (pos->pinnedPieces != 0) {
        filter_pins(&move_list, pos);
    }
    uint64_t result = 0ULL;

    uint32_t piece_pos = long_bit_scan(mask);
    uint32_t* start = move_list.start();
    const uint32_t* end = move_list.end();
    uint32_t xd = *start >> 16;
    int xd2 = *(start + 1) >> 16;
    while (start < end) {
        uint32_t from = move_list.from_sq(*start);
        if (from == piece_pos)
            result |= 1ULL << move_list.to_sq(*start);
        start++;
    }
    return result;
}

uint32_t Engine::find_best_move(int depth, Position* pos) {
    if (depth == 0)
        return Evaluate(pos);
    uint16_t book_move = ob->find_book_move(pos);
    
    Move_list moves;
    get_legal_moves(pos, &moves);
    //Get move with flags from legal moves array
    if (book_move) {
        //Castling moves have wierd notation because of chess 960
        const int king = pos->whiteToMove ? 0 : 6;
        if (moves.book_castle(book_move, pos->pieceBoards[king])) {
            //Change to correct to square
            if ((book_move & 0x3F) % 8 == 0)
                book_move += 2;
            else
                book_move -= 1;
        }
        return moves.contains(book_move & 0xFFF);
    }
    Position current = *pos;
    double best_eval = worst_eval; //Initiate as worst eval
    uint32_t best_move = moves.move_list[0];

    if (moves.size() == 0) {
        return 0;
    }

    moves.sort();

    for (uint32_t* move = moves.start(); move < moves.end(); move++) {
        make_move(&current, *move);

        double eval = -search_eval2(depth - 1, worst_eval - depth, -worst_eval + depth, &current);
        parse_move(*move);
        cout << eval << endl;

        if (eval > best_eval) {
            best_move = *move;
        }
        best_eval = std::max(best_eval, eval);
        undo_move(&current, pos);
    }
    //cout << endl << endl;
    return best_move;
}

int Engine::search_eval(int depth, Position* pos) {
    if (depth == 0)
        return Evaluate(pos);
    Move_list moves;
    get_legal_moves(pos, &moves);
    if (moves.size() == 0) {
        if (pos->numCheckers > 0)
            return worst_eval - depth; //Checkmate
        else
            return 0; //Stalemate
    }
    int best_eval = worst_eval;
    Position current = *pos;

    for (uint32_t* move = moves.start(); move < moves.end(); move++) {
        make_move(&current, *move);
        int eval = -search_eval(depth - 1, &current);
        undo_move(&current, pos);
        if (eval > best_eval)
            best_eval = eval;
    }
    return best_eval;
}

double Engine::search_eval2(int depth, int alpha, int beta, Position* pos) {
    if (depth == 0)
        return Evaluate(pos);
    Move_list moves;
    get_legal_moves(pos, &moves);
    if (moves.size() == 0) {
        if (pos->numCheckers > 0)
            return worst_eval - depth; //Checkmate
        else
            return 0; //Stalemate
    }
    Position current = *pos;

    moves.sort();

    for (uint32_t* move = moves.start(); move < moves.end(); move++) {
        make_move(&current, *move);
        int eval = -search_eval2(depth - 1, -beta, -alpha, &current);
        undo_move(&current, pos);
        if (eval >= beta)
            return beta;
        if (eval > alpha)
            alpha = eval;
    }
    return alpha;
}

uint32_t Engine::find_best_move_th(int depth, Position* pos, bool* run) {
    if (depth == 0)
        return Evaluate(pos);
    Move_list moves;
    get_legal_moves(pos, &moves);

    Position current = *pos;
    int best_eval = -999999; //Initiate as worst eval
    uint32_t best_move = moves.move_list[0];

    if (moves.size() == 0) {
        return 0;
    }

    moves.sort();

    for (uint32_t* move = moves.start(); move < moves.end() && *run; move++) {
        make_move(&current, *move);

        int eval = -search_eval2_th(depth - 1, -999999 - depth, 999999 + depth, &current, run);
        //parse_move(*move);
        //cout << eval << endl;

        if (eval > best_eval) {
            best_move = *move;
        }
        best_eval = std::max(best_eval, eval);
        undo_move(&current, pos);
    }
    //cout << endl << endl;
    return best_move;
}

int Engine::search_eval2_th(int depth, int alpha, int beta, Position* pos, bool* run) {
    if (depth == 0)
        return Evaluate(pos);
    Move_list moves;
    get_legal_moves(pos, &moves);
    if (moves.size() == 0) {
        if (pos->numCheckers > 0)
            return -999999 - depth; //Checkmate
        else
            return 0; //Stalemate
    }
    Position current = *pos;

    moves.sort();

    for (uint32_t* move = moves.start(); move < moves.end() && *run; move++) {
        make_move(&current, *move);
        int eval = -search_eval2(depth - 1, -beta, -alpha, &current);
        undo_move(&current, pos);
        if (eval >= beta)
            return beta;
        if (eval > alpha)
            alpha = eval;
    }
    return alpha;
}


//THIS IS SOME SHIT CODE
uint64_t Engine::perft(int depth, Position* pos) {
    hash_hits = 0;
    uint64_t num_positions = 0;
    Position current = *pos;

    Move_list legal_moves;
    get_legal_moves(&current, &legal_moves);

    for (uint32_t* move = legal_moves.start(); move < legal_moves.end(); move++) {
        current.moves++;
        parse_move(*move);
        make_move(&current, *move);

        uint64_t part = search(depth - 1, &current);

        //Print how many positions are reached after the move
        cout << part << endl;

        num_positions += part;
        undo_move(&current, pos);
    }
    cout << "Total: ";
    cout << num_positions << endl;
    cout << "Hash hits: " << hash_hits << endl;
    perft_map.clear();
    return num_positions;
}

void Engine::_perft_debug(int depth, Position* pos) {

    perft_out = "";
    uint64_t num_positions = 0;
    Position current = *pos;

    Move_list legal_moves;
    get_legal_moves(&current, &legal_moves);

    for (uint32_t* move = legal_moves.start(); move < legal_moves.end(); move++) {
        //Print the move being made
        parse_move(*move);
        make_move(&current, *move);

        uint64_t part = search(depth - 1, &current);

        //Print how many positions are reached after the move
        cout << part << endl;

        perft_out += std::to_string(part) + "\n";

        num_positions += part;
        undo_move(&current, pos);
    }
    cout << "Total: ";
    cout << num_positions << endl;
    perft_out.pop_back();
    std::ofstream outfile("ours.txt");

    outfile << perft_out;

    outfile.close();
}

void Engine::init_hashtable(Position* pos) {
    std::random_device os_seed;
    const uint32_t seed = os_seed();
    std::mt19937 generator(seed);
    std::uniform_int_distribution< uint64_t > distribute(0, LLONG_MAX);

    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 64; j++) {
            hash_table[i][j] = distribute(generator);
        }
    }
    for (int i = 0; i < 8; i++) {
        ep_hash[i] = distribute(generator);
    }

    castle_hash[0] = distribute(generator);
    castle_hash[1] = distribute(generator);
    castle_hash[2] = distribute(generator);
    castle_hash[3] = distribute(generator);
    black_to_move_hash = distribute(generator);
    move_hash = distribute(generator);
}

uint64_t Engine::zobrist_hash(Position* pos) {

    uint64_t hash = pos->moves * move_hash;

    if (!pos->whiteToMove)
        hash ^= black_to_move_hash;
    uint64_t board = pos->teamBoards[0];
    const uint64_t white = pos->teamBoards[1];
    while (board) {
        const int bit = long_bit_scan(board);
        const uint64_t b_pos = 1ULL << bit;
        const int id = (white & b_pos) ? pos->find_mask(b_pos, 0, 6) : pos->find_mask(b_pos, 6, 12);
        hash ^= hash_table[id][bit];
        board &= board - 1;
    }
    uint8_t castle = pos->castlingRights;
    for (int i = 0; i < 4; i++) {
        if (castle & 1)
            hash ^= castle_hash[i];
        castle >>= 1;
    }
    if (pos->enPassant != 0xFF) {
        int col = (pos->enPassant & 0xFF) % 8;
        hash ^= ep_hash[col];
    }
    return hash;
}

uint64_t hash_pos(Position* pos) {

    static int prime_list[12] = { 593, 859, 1237, 1069, 2887, 2269, 2953, 2153, 2621, 3709, 4519, 3769 };

    uint64_t temp = 0;
    for (int i = 0; i < 12; i++) {
        temp ^= pos->pieceBoards[i] * prime_list[i];
    }
    temp ^= pos->teamBoards[0] * 397;
    temp ^= pos->teamBoards[1] * 877;
    temp ^= pos->teamBoards[2] * 677;

    temp ^= pos->whiteAttack * 349;
    temp ^= pos->blackAttack * 479;
    temp ^= pos->checker * 977;
    temp ^= pos->enPassant * 389;
    temp ^= pos->castlingRights * 1181;
    temp ^= pos->moves * 227;

    return temp;
}

uint64_t Engine::search(int depth, Position* pos) {

    if (depth == 0)
        return 1;

    uint64_t hash;
    if (depth > 1) {
        hash = zobrist_hash(pos);
        if (perft_map.find(hash) != perft_map.end()) {
            hash_hits++;
            uint64_t tmp = perft_map.at(hash);
            return tmp;
        }
    }

    uint64_t num_positions = 0;
    Position current = *pos;

    //Get legal moves for current position
    Move_list legal_moves;
    get_legal_moves(&current, &legal_moves);

    if (depth == 1)
        return (uint64_t)legal_moves.size();

    for (uint32_t* move = legal_moves.start(); move < legal_moves.end(); move++) {
        current.moves++;
        make_move(&current, *move);
        Evaluate(&current);
        num_positions += search(depth - 1, &current);
        undo_move(&current, pos);
    }

    if (depth > 1) {
        perft_map[hash] = num_positions;
    }
    return num_positions;
}



////NOT FINISHED
//Use this to replace function in window moving the pieces
void Engine::make_move(Position* pos, uint32_t move) {
    sound = s_move;
    const bool white_to_move = pos->whiteToMove;
    const uint16_t piece_offset = white_to_move ? 0 : 6;
    const uint16_t them_offset = white_to_move ? 6 : 0;
    const uint64_t from_sq = 1ULL << ((move >> 6) & 0x3F);
    const uint64_t to_sq = 1ULL << (move & 0x3F);
    const uint16_t flags = (move >> 12) & 0xF;
    const int team = 1 + piece_offset / 6;
    const int enemy = team ^ 3; //enemy = team == 1 ? 2 : 1;


    //TODO: Find better way to know which piece made the move, maybe save as flag in move itself
    uint32_t pieceID = move >> 16;
    pos->pieceBoards[pieceID] &= ~from_sq;

    if (flags == 5) { //En passant capture
        uint32_t xd = pos->enPassant >> 8;
        uint64_t realPawn = ~(1ULL << (pos->enPassant >> 8));
        pos->pieceBoards[them_offset + 1] &= realPawn; //Remove real pawn
        pos->teamBoards[enemy] &= realPawn;
        pos->teamBoards[0] &= realPawn;
        sound = s_capture;
    }
    else if ((flags & 4) == 4) { //Regular capture
        int capID = pos->find_mask(to_sq, them_offset + 1, them_offset + 6);
        //TODO: If rook gets captured change castling rights
        pos->pieceBoards[capID] &= ~to_sq;
        pos->teamBoards[enemy] &= ~to_sq;
        sound = s_capture;
    }
    if (flags == 1) { //Double push
        en_passant(pos, move & 0x3F);
    }
    else {
        pos->enPassant = 0xFF; //Disable en passant
    }
    if (flags == 2 || flags == 3) { //Castling move
        const uint64_t rook = flags == 2 ? to_sq << 1 : to_sq >> 2;
        const uint64_t to = flags == 2 ? to_sq >> 1 : to_sq << 1;
        castle(pos, ~rook, to); //Clear rook and move to "to" square
        sound = s_castle;
    }

    if (flags > 7) { //Promotion
        pieceID = piece_offset + 2 + (flags & 0x3); //Change which bitboard should get the set bit
    }


    static uint64_t bking_corner = 1ULL << 7;
    static uint64_t bqueen_corner = 1ULL;
    static uint64_t wking_corner = 1ULL << 63;
    static uint64_t wqueen_corner = 1ULL << 56;

    const uint64_t rooks = pos->pieceBoards[them_offset + 4];
    if (!white_to_move) {
        if ((rooks & wking_corner) == 0) {
            pos->castlingRights &= 0b1110;
        }
        if ((rooks & wqueen_corner) == 0) {
            pos->castlingRights &= 0b1101;
        }
    }
    else {
        if ((rooks & bking_corner) == 0) {
            pos->castlingRights &= 0b1011;
        }
        if ((rooks & bqueen_corner) == 0) {
            pos->castlingRights &= 0b0111;
        }
    }


    //King moves disable castling rights
    if (pieceID - piece_offset == 0) {
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
    pos->moves++;
    update_attack(pos);
}

void Engine::undo_move(Position* current, Position* perv) {
    *current = *perv;
}

void Engine::update_attack(Position* pos) {
    pos->numCheckers = 0;
    pos->pinnedPieces = 0ULL;
    pos->checker = ~0ULL;
    pos->block_check = 0;
    if (pos->whiteToMove) { //Only need to update black attack
        uint64_t black_attack = 0ULL;
        const int king_pos = long_bit_scan(pos->pieceBoards[6]);
        const uint64_t board_without_king = pos->teamBoards[0] & ~pos->pieceBoards[0];
        black_attack |= king_attacks[king_pos];
        black_attack |= pawn_attacks(pos->pieceBoards[7], false);
        black_attack |= piece_attacks(pos->teamBoards[0], pos->pieceBoards[8], 0);
        black_attack |= piece_attacks(board_without_king, pos->pieceBoards[9], 1);
        black_attack |= piece_attacks(board_without_king, pos->pieceBoards[10], 2);
        black_attack |= piece_attacks(board_without_king, pos->pieceBoards[11], 3);
        pos->blackAttack = black_attack;
        if ((black_attack & pos->pieceBoards[0]) != 0) { //Intersection with king gives check
            in_check_masks(pos, true);
            sound = s_check;
        }
    }
    else {
        uint64_t white_attack = 0ULL;
        const int king_pos = long_bit_scan(pos->pieceBoards[0]);
        const uint64_t board_without_king = pos->teamBoards[0] & ~pos->pieceBoards[6];
        white_attack |= king_attacks[king_pos];
        white_attack |= pawn_attacks(pos->pieceBoards[1], true);
        white_attack |= piece_attacks(pos->teamBoards[0], pos->pieceBoards[2], 0);
        white_attack |= piece_attacks(board_without_king, pos->pieceBoards[3], 1);
        white_attack |= piece_attacks(board_without_king, pos->pieceBoards[4], 2);
        white_attack |= piece_attacks(board_without_king, pos->pieceBoards[5], 3);
        pos->whiteAttack = white_attack;
        if ((white_attack & pos->pieceBoards[6]) != 0) { //Intersection with king gives check
            in_check_masks(pos, false);
            sound = s_check;
        }
    }
    find_pins(pos);

}

void Engine::player_make_move(const uint32_t move) {
    move_highlight = move & 0xFFFF;

    make_move(&pos, move);
    p_list = new Position_list(p_list, &pos);
    pos = p_list->curr_pos;
    Move_list next;
    get_legal_moves(&pos, &next);
    if (next.size() == 0) {
        if (pos.numCheckers > 0) {
            sound = s_checkmate;
            std::cout << "Checkmate" << std::endl;
        }
        else
            std::cout << "Stalemate" << std::endl;
        return;
    }
}

void Engine::player_undo_move() {
    if (p_list->prev_positions != nullptr) {
        Position_list* tmp = p_list;
        p_list = p_list->prev_positions;
        pos = p_list->curr_pos;
        delete(tmp);
    }
}




//Filter out pinned pieces illeagal moves
void Engine::filter_pins(Move_list* move_list, Position* pos) {
    uint32_t filtered_moves[100];
    const uint64_t pinned = pos->pinnedPieces;
    const int king_pos = pos->whiteToMove ? long_bit_scan(pos->pieceBoards[0]) : long_bit_scan(pos->pieceBoards[6]);

    //If the from square is on the pinned board then it might need to be filtered
    int filter_size = 0;
    for (int i = 0; i < move_list->size(); i++) {
        const uint16_t move_from = move_list->from_sq(move_list->move_list[i]);
        if (((1ULL << move_from) & pinned) != 0) {
            const uint64_t move_to = 1ULL << move_list->to_sq(move_list->move_list[i]);
            const uint64_t ray = pinned_ray(king_pos, move_from);
            if ((move_to & ray) != 0) {
                filtered_moves[filter_size++] = move_list->move_list[i];
            }
        }
        else {
            filtered_moves[filter_size++] = move_list->move_list[i];
        }
    }
    //Copy back the filtered moves to the move_list and update last
    memcpy(move_list->move_list, filtered_moves, filter_size * sizeof(uint32_t));
    move_list->last = move_list->move_list + filter_size;
}


//Find the ray in which the piece is able to move
const uint64_t Engine::pinned_ray(int king, int piece) {
    //On the same rank
    if (king / 8 == piece / 8)
        return ranks[king / 8];
    //On the same file
    if (king % 8 == piece % 8)
        return files[king % 8];
    const int diagonal = king / 8 + king % 8;
    //On the same anti-diagonal
    if (diagonal == piece / 8 + piece % 8)
        return anti_diagonals[diagonal];
    //Otherwise on the same main-diagonal
    return main_diagonals[7 - king / 8 + king % 8];
}

//Edits the rooks position
void Engine::castle(Position* pos, uint64_t rook, uint64_t to) {
    const int team = to > 0xFFF ? 1 : 2;
    const int rook_id = to > 0xFFF ? 4 : 10;

    pos->teamBoards[0] &= rook;
    pos->teamBoards[0] |= to;
    pos->teamBoards[team] &= rook;
    pos->teamBoards[team] |= to;
    pos->pieceBoards[rook_id] &= rook;
    pos->pieceBoards[rook_id] |= to;

}

//Set check evasion square in the position object-> Is used to find legal moves when in check
void Engine::in_check_masks(Position* pos, bool white_in_check) {
    const int piece_offset = white_in_check ? 6 : 0;
    const uint64_t board = pos->teamBoards[0];
    const uint64_t king = white_in_check ? pos->pieceBoards[0] : pos->pieceBoards[6];
    uint64_t checker = 0ULL;
    uint64_t block = 0ULL;

    uint64_t pawnCheck = pawn_attacks(king, white_in_check) & pos->pieceBoards[1 + piece_offset];
    uint64_t knightCheck = piece_attacks(board, king, 0) & pos->pieceBoards[2 + piece_offset];
    uint64_t diagonalCheck = piece_attacks(board, king, 1) & (pos->pieceBoards[3 + piece_offset] | pos->pieceBoards[5 + piece_offset]);
    uint64_t straightCheck = piece_attacks(board, king, 2) & (pos->pieceBoards[4 + piece_offset] | pos->pieceBoards[5 + piece_offset]);

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
    pos->checker = checker == 0 ? ~0ULL : checker;
}

void Engine::en_passant(Position* pos, int pushedPawn) {
    //White double push
    pos->enPassant = 0xFF;
    if (pos->whiteToMove) {
        //If no black pawns present on 5th rank then no en passant possible
        if ((pos->pieceBoards[7] & 0xFF00000000L) == 0)
            return;
        uint64_t sliders = pos->pieceBoards[4] | pos->pieceBoards[5];
        //If any of the slider occupy the fifth rank then we need to check for pin
        if ((sliders & 0xFF00000000L) > 0) {
            uint64_t slideAttack = piece_attacks(pos->teamBoards[0], sliders, 6);
            uint64_t kingAttack = piece_attacks(pos->teamBoards[0], pos->pieceBoards[6], 6);
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
            uint64_t slideAttack = piece_attacks(pos->teamBoards[0], sliders, 6);
            uint64_t kingAttack = piece_attacks(pos->teamBoards[0], pos->pieceBoards[0], 6);
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


//Find the square on which your pieces can block the check
uint64_t Engine::find_block(uint32_t king, uint32_t checker) {
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

void Engine::find_pins(Position* pos) {
    const bool white_to_move = pos->whiteToMove;
    const int start = pos->whiteToMove ? 0 : 6;
    const uint64_t king_board = pos->pieceBoards[start];
    const int king_pos = long_bit_scan(king_board);
    const uint64_t straight = white_to_move ? pos->pieceBoards[10] | pos->pieceBoards[11] : pos->pieceBoards[4] | pos->pieceBoards[5];
    const uint64_t diagonal = white_to_move ? pos->pieceBoards[9] | pos->pieceBoards[11] : pos->pieceBoards[3] | pos->pieceBoards[5];
    const uint64_t king_file = files[king_pos % 8];
    const uint64_t king_rank = ranks[king_pos / 8];
    const uint64_t kingMD = main_diagonals[7 + king_pos % 8 - king_pos / 8];
    const uint64_t kingAD = anti_diagonals[king_pos % 8 + king_pos / 8];

    //Possible pinning pieces are aligned with the king
    const uint64_t relevant_main = kingMD & diagonal;
    const uint64_t relevant_anti = kingAD & diagonal;
    const uint64_t relevant_rank = king_rank & straight;
    const uint64_t relevant_file = king_file & straight;

    //Possible pinning pieces, INVESTIGATE
    uint64_t nSameMask = white_to_move ? ~pos->teamBoards[2] : ~pos->teamBoards[1];
    const uint64_t board = pos->teamBoards[0];
    if (relevant_main != 0) {
        const uint64_t piece = piece_attacks(board, relevant_main, 4) & nSameMask;
        const uint64_t king = piece_attacks(board, king_board, 4);
        pos->pinnedPieces |= piece & king; //If the king and the sliding pieces attack the same piece then it is pinned
    }
    if (relevant_anti != 0) {
        const uint64_t piece = piece_attacks(board, relevant_anti, 5) & nSameMask;
        const uint64_t king = piece_attacks(board, king_board, 5);
        pos->pinnedPieces |= piece & king; //If the king and the sliding pieces attack the same piece then it is pinned
    }
    if (relevant_rank != 0) {
        const uint64_t piece = piece_attacks(board, relevant_rank, 6) & nSameMask;
        const uint64_t king = piece_attacks(board, king_board, 6);
        pos->pinnedPieces |= piece & king; //If the king and the sliding pieces attack the same piece then it is pinned
    }
    if (relevant_file != 0) {
        const uint64_t piece = piece_attacks(board, relevant_file, 7) & nSameMask;
        const uint64_t king = piece_attacks(board, king_board, 7);
        pos->pinnedPieces |= piece & king; //If the king and the sliding pieces attack the same piece then it is pinned
    }
}

void Engine::fenInit(Position* pos, std::string fen) {
    uint32_t i = 0;
    memset(pos, 0, sizeof(*pos));
    for (char f : fen) {
        if (i < 64) {
            switch (f) {
            case 'K': pos->pieceBoards[0] |= 1ULL << i; break;
            case 'P': pos->pieceBoards[1] |= 1ULL << i; break;
            case 'N': pos->pieceBoards[2] |= 1ULL << i; break;
            case 'B': pos->pieceBoards[3] |= 1ULL << i; break;
            case 'R': pos->pieceBoards[4] |= 1ULL << i; break;
            case 'Q': pos->pieceBoards[5] |= 1ULL << i; break;
            case 'k': pos->pieceBoards[6] |= 1ULL << i; break;
            case 'p': pos->pieceBoards[7] |= 1ULL << i; break;
            case 'n': pos->pieceBoards[8] |= 1ULL << i; break;
            case 'b': pos->pieceBoards[9] |= 1ULL << i; break;
            case 'r': pos->pieceBoards[10] |= 1ULL << i; break;
            case 'q': pos->pieceBoards[11] |= 1ULL << i; break;
            case '/': i--; break;
            default: i += f - 49; break;
            }
            i++;
        }
        else {
            switch (f) {
            case 'w': pos->whiteToMove = true; break;
            case 'b': pos->whiteToMove = false; break;
            case 'K': pos->castlingRights |= 0b0001; break; //White king side castling
            case 'Q': pos->castlingRights |= 0b0010; break; //White queen side castling
            case 'k': pos->castlingRights |= 0b0100; break; //Black king side castling
            case 'q': pos->castlingRights |= 0b1000; break; //Black queen side castling
            default: break;
            }

        }
    }
    pos->teamBoards[1] = pos->pieceBoards[0] | pos->pieceBoards[1] | pos->pieceBoards[2] |
        pos->pieceBoards[3] | pos->pieceBoards[4] | pos->pieceBoards[5];
    pos->teamBoards[2] = pos->pieceBoards[6] | pos->pieceBoards[7] | pos->pieceBoards[8] |
        pos->pieceBoards[9] | pos->pieceBoards[10] | pos->pieceBoards[11];
    pos->teamBoards[0] = pos->teamBoards[1] | pos->teamBoards[2];
    pos->enPassant = 0xFF;
    Engine::update_attack(pos);
    p_list = new Position_list(nullptr, pos);
}
