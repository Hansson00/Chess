#pragma once
#include "iostream"
#include "Move_Generator.h"
#include "Thread"
#include "future"


void T_perft(int depth, Position* pos, Move_Generator* mg);

uint64_t search(int depth, Position* prev_pos, Move_Generator* mg);

void get_legal_moves(Position* pos, Move_list* move_list, Move_Generator* mg);

void filter_pins(Move_list* move_list, Position* pos, Move_Generator* mg);

const uint64_t pinned_ray(int king, int piece, Move_Generator* mg);

void parse_move(uint16_t move);

void undo_move(Position* current, Position* prev);

void make_move(Position* pos, uint16_t move, Move_Generator* mg);

void en_passant(Position* pos, int pushedPawn, Move_Generator* mg);

void castle(Position* pos, uint64_t rook, uint64_t to);

void update_attack(Position* pos, Move_Generator* mg);

void in_check_masks(Position* pos, bool white_in_check, Move_Generator* mg);

uint64_t find_block(uint32_t king, uint32_t checker);

void find_pins(Position* pos, Move_Generator* mg);

