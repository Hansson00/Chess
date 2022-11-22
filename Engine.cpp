#include "Engine.h"
using namespace std;

Engine::Engine() {

    fenInit(&pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq");//"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq");
    //fenInit(&pos, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ");//"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq");
    //fenInit(&pos, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
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

uint64_t Engine::move_squares(uint16_t* moves, uint16_t* end) {
    uint64_t result = 0ULL;
    while (*moves) {
        result |= 1ULL << (*moves);
        moves++;
    }
    return result;
}

//THIS IS SOME SHIT CODE
uint64_t Engine::perft(int depth, Position* pos) {
    hash_hits = 0;
    uint64_t num_positions = 0;
    Position current = *pos;
    uint64_t hash = zobrist_hash(pos);

    Move_list legal_moves;
    get_legal_moves(&current, &legal_moves);

    for (uint32_t* move = legal_moves.start(); move < legal_moves.end(); move++) {
        parse_move(*move);
        uint64_t new_hash = make_move(&current, *move, hash);

        uint64_t part = search(depth - 1, &current, new_hash);

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

void Engine::test() {
    auto start = std::chrono::system_clock::now();
    Position curr = pos;
    uint64_t hash = zobrist_hash(&pos);
    Move_list moves;
    get_legal_moves(&pos, &moves);
    for (int i = 0; i < 99999999; i++) {
        make_move(&curr, moves.move_list[5], hash);

        /// <summary>
        /// 
        /// 
        /// hello xd
        /// 
        /// 
        /// 
        /// 
        /// 
        /// </summary>
        undo_move(&curr, &pos);
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = (end - start);
    cout << diff.count() << endl;
}

void Engine::_perft_debug(int depth, Position* pos) {

    perft_out = "";
    uint64_t num_positions = 0;
    Position current = *pos;
    uint64_t hash = zobrist_hash(pos);

    Move_list legal_moves;
    get_legal_moves(&current, &legal_moves);

    for (uint32_t* move = legal_moves.start(); move < legal_moves.end(); move++) {
        //Print the move being made
        parse_move(*move);
        uint64_t new_hash = make_move(&current, *move, hash);

        uint64_t part = search(depth - 1, &current, new_hash);

        //Print how many positions are reached after the move
        cout << part << endl;

        perft_out += std::to_string(part)+ "\n";

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

//Init all squares with random number for each piece board
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
    //if (!pos->whiteToMove)
       // hash ^= black_to_move_hash;
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

uint64_t Engine::search(int depth, Position* pos, uint64_t hash) {
    if (depth == 0)
        return 1;

    //Get legal moves for current position
    Move_list legal_moves;
    get_legal_moves(pos, &legal_moves);

    if (depth == 1)
        return (uint64_t)legal_moves.size();

    uint64_t num_positions = 0;
    Position current = *pos;

    for (uint32_t* move = legal_moves.start(); move < legal_moves.end(); move++) {
        uint64_t new_hash = make_move(&current, *move, hash);
        
        if (perft_map.find(new_hash) != perft_map.end()) {
            hash_hits++;
            num_positions += perft_map.at(new_hash);
        }
        else {
            num_positions += search(depth - 1, &current, new_hash);
        }
        undo_move(&current, pos);
    }
    
    if (depth > 1) {
        perft_map[hash] = num_positions;
    }
    
    return num_positions;
}


void Engine::parse_move(uint16_t move) {
    int from = (move >> 6) & 0x3F;
    int to = move & 0x3F;
    cout << (char)(from % 8 + 'a') << (char)('8' - from / 8) 
        << (char)('a' + to % 8) << (char)('8' - to / 8) << ": ";
    
    perft_out += (char)(from % 8 + 'a');
    perft_out += (char)('8' - from / 8);
    perft_out += (char)('a' + to % 8);
    perft_out += (char)('8' - to / 8);
    perft_out += ": ";


}

////NOT FINISHED
//Use this to replace function in window moving the pieces
uint64_t Engine::make_move(Position* pos, uint32_t move, uint64_t curr_hash) {
    sound = s_move;
    const bool white_to_move = pos->whiteToMove;
    const uint16_t piece_offset = white_to_move ? 0 : 6;
    const uint16_t them_offset = white_to_move ? 6 : 0;
    const int to = move & 0x3F;
    const int from = (move >> 6) & 0x3F;
    const uint64_t from_sq = 1ULL << from;
    const uint64_t to_sq = 1ULL << to;
    const uint16_t flags = (move >> 12) & 0xF;
    const int team = 1 + piece_offset / 6;
    const int enemy = team ^ 3; //enemy = team == 1 ? 2 : 1;
    uint8_t previous_castling = pos->castlingRights;
    
    //Flip side to move hash
    //curr_hash ^= black_to_move_hash;
    //TODO: Find better way to know which piece made the move, maybe save as flag in move itself
    uint32_t pieceID = move >> 16;
    pos->pieceBoards[pieceID] &= ~from_sq;
    curr_hash ^= hash_table[pieceID][from];
   
    if (flags == 5) { //En passant capture
        uint32_t real_pawn = pos->enPassant >> 8;
        uint64_t pawn_board = 1ULL << (real_pawn);
        pos->pieceBoards[them_offset + 1] ^= pawn_board; //Remove real pawn
        pos->teamBoards[enemy] ^= pawn_board;
        pos->teamBoards[0] ^= pawn_board;
        //Update hash when capturing en passant
        curr_hash ^= hash_table[them_offset + 1][real_pawn];
        sound = s_capture;
    }
    else if ((flags & 4) == 4) { //Regular capture
        int capID = pos->find_mask(to_sq, them_offset + 1, them_offset + 6);
        //TODO: If rook gets captured change castling rights
        pos->pieceBoards[capID] ^= to_sq;
        pos->teamBoards[enemy] ^= to_sq;
        //XOR out captured piece
        curr_hash ^= hash_table[capID][to];
        sound = s_capture;
    }
    if (flags == 1) { //Double push
        curr_hash = en_passant(pos, move & 0x3F, curr_hash);
    }
    else if(pos->enPassant != 0xFF){
        curr_hash ^= ep_hash[(pos->enPassant & 0xFF) % 8];
        pos->enPassant = 0xFF; //Disable en passant
    }
    if (flags == 2 || flags == 3) { //Castling move
        const uint64_t rook = flags == 2 ? to_sq << 1 : to_sq >> 2;
        const uint64_t to = flags == 2 ? to_sq >> 1 : to_sq << 1;
        curr_hash = castle(pos, rook, to, curr_hash); //Clear rook and move to "to" square
        sound = s_castle;
    }
    
    else if (flags > 7) { //Promotion
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
    

    //King moves disable castling rights and castling hashes
    if (pieceID - piece_offset == 0) {
        const int set = white_to_move ? 0b1100 : 0b0011;
        pos->castlingRights &= set;
    }

    //If castling was available then remove prev castling and add new
    
    if (previous_castling) {
        uint8_t updated_castling = pos->castlingRights;
        for (int i = 0; i < 4; i++) {
            if (updated_castling & 1)
                curr_hash ^= castle_hash[i];
            if (previous_castling & 1)
                curr_hash ^= castle_hash[i];
            //Next castling check
            updated_castling >>= 1;
            previous_castling >>= 1;
        }
    }
    

    //Move the piece in all bitboards
    pos->pieceBoards[pieceID] ^= to_sq; //Setting bit is done after promotion check because promoting fill change pieceID
    pos->teamBoards[team] ^= from_sq;
    pos->teamBoards[team] ^= to_sq;
    pos->teamBoards[0] ^= from_sq;
    pos->teamBoards[0] |= to_sq;

    //Updating piece to hash
    curr_hash ^= hash_table[pieceID][to];
    
    
    pos->whiteToMove = !pos->whiteToMove;
    update_attack(pos);

    //Update move counter hash
    curr_hash ^= pos->moves * move_hash;
    curr_hash ^= ++pos->moves * move_hash;
    
    
    const uint64_t tmp = zobrist_hash(pos);

    if (curr_hash != tmp) {
        cout << "Faulty hash" << endl;
    }
    
    return curr_hash;
}

void Engine::undo_move(Position* current, Position* perv){
    *current = *perv;
}

void Engine::update_attack(Position* pos) {
    pos->numCheckers = 0;
    pos->pinnedPieces = 0ULL;
    pos->checker = ~0ULL;
    pos->block_check = 0;
    if (pos->whiteToMove){ //Only need to update black attack
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
    else{
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
    /*
        if (sound == s_check) {
        Move_list m;
        get_legal_moves(pos, &m);
        if (m.size() == 0)
            sound = s_checkmate;
    }
    */
}

void Engine::player_make_move(const uint32_t move) {
    make_move(&pos, move, zobrist_hash(&pos));
    p_list = new Position_list(p_list, &pos);
    pos = p_list->curr_pos;
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
    for (uint32_t i = 0; i < move_list->size(); i++) {
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
const uint64_t Engine::pinned_ray (const int& king, const int& piece) const{
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
uint64_t Engine::castle(Position* pos, uint64_t rook, uint64_t to, uint64_t hash) {
    const int team = to > 0xFFF ? 1 : 2;
    const int rook_id = to > 0xFFF ? 4 : 10;
    const int rook_from = long_bit_scan(rook);
    const int rook_to = long_bit_scan(to);
    pos->teamBoards[0] ^= rook;
    pos->teamBoards[0] ^= to;
    pos->teamBoards[team] ^= rook;
    pos->teamBoards[team] ^= to;
    pos->pieceBoards[rook_id] ^= rook;
    pos->pieceBoards[rook_id] ^= to;
    //Update hash for rook
    hash ^= hash_table[rook_id][rook_from];
    hash ^= hash_table[rook_id][rook_to];
    return hash;
}

//Set check evasion square in the position object-> Is used to find legal moves when in check
void Engine::in_check_masks(Position* pos, bool white_in_check) {
    const int piece_offset = white_in_check ? 6 : 0;
    const uint64_t board = pos->teamBoards[0];
    const uint64_t king = white_in_check ? pos->pieceBoards[0] : pos->pieceBoards[6];
    uint64_t checker = 0ULL;
    uint64_t block = 0ULL;

    uint64_t pawnCheck = pawn_attacks(king, white_in_check) & pos->pieceBoards[1 + piece_offset];
    uint64_t knightCheck = piece_attacks(board ,king, 0) & pos->pieceBoards[2 + piece_offset];
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

uint64_t Engine::en_passant(Position* pos, int pushedPawn, uint64_t hash) {
    //EP hashing updating
    const int ep = pos->enPassant;
    if (ep != 0xFF) {
        hash ^= ep_hash[(ep & 0xFF) % 8];
        pos->enPassant = 0xFF;
    }
    
    //White double push
    if (pos->whiteToMove) {
        //If no black pawns present on 5th rank then no en passant possible
        if ((pos->pieceBoards[7] & 0xFF00000000L) == 0)
            return hash;
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
                    return hash;
                }
            }
            else if ((kingAttack & pawnPos) != 0) {
                //If the slider sees pawn next to pushed pawn then no en passant available
                if ((slideAttack & (pawnPos >> 1)) != 0 || (slideAttack & (pawnPos << 1)) != 0) {
                    return hash;
                }
            }
        }
    }
    else {
        //If no white pawns present on 4th rank then no en passant possible
        if ((pos->pieceBoards[1] & 0xFF000000L) == 0)
            return hash;
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
                    return hash;
                }
            }
            else if ((kingAttack & pawnPos) != 0) {
                //If the slider sees pawn next to pushed pawn then no en passant available
                if ((slideAttack & (pawnPos >> 1)) != 0 || (slideAttack & (pawnPos << 1)) != 0) {
                    return hash;
                }
            }
        }
    }
    const int pseudo = pos->whiteToMove ? pushedPawn + 8 : pushedPawn - 8;
    //Stores both real pawn and pseudoPawn
    pos->enPassant = (pushedPawn << 8) | pseudo;
    return hash ^ ep_hash[pseudo % 8];
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
            case 'b': pos->pieceBoards[9]|= 1ULL << i; break;
            case 'r': pos->pieceBoards[10] |= 1ULL << i; break;
            case 'q': pos->pieceBoards[11]|= 1ULL << i; break;
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
    init_hashtable(pos);
    hash_start = zobrist_hash(pos);
}
