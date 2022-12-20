#include "Evaluation.h"


int Evaluation::Evaluate(Position* pos) {
	int m_white = 0, m_black = 0;
	for (int i = 1; i < 6; i++) {
		m_white += bit_count(pos->pieceBoards[i]) * piece_value[i];
		m_black += bit_count(pos->pieceBoards[i + 6]) * piece_value[i];
	}


	double eval_cap = pos->whiteToMove ? m_white - m_black : m_black - m_white;
	int eval_pos = 0;

	int us_offset = pos->whiteToMove ? 0 : 6;
	int they_offset = pos->whiteToMove ? 6 : 0;


	/* Pawns */
	eval_pos += eval_piece(pos->pieceBoards[1 + us_offset],
		pos->whiteToMove ? w_pawn_heatmap : b_pawn_heatmap);
	eval_pos -= eval_piece(pos->pieceBoards[1 + they_offset],
		!pos->whiteToMove ? w_pawn_heatmap : b_pawn_heatmap) >> 2;


	/* King */
	eval_pos += eval_piece(pos->pieceBoards[0 + us_offset],
		pos->whiteToMove ? w_king_heatmap : b_king_heatmap);
	eval_pos -= eval_piece(pos->pieceBoards[0 + they_offset], 
		!pos->whiteToMove ? w_king_heatmap : b_king_heatmap)>> 2;

	/* Rooks */
	eval_pos += eval_piece(pos->pieceBoards[4 + us_offset],
		pos->whiteToMove ? w_rook_heatmap : b_rook_heatmap);
	eval_pos -= eval_piece(pos->pieceBoards[4 + they_offset], 
		!pos->whiteToMove ? w_rook_heatmap : b_rook_heatmap)>>2;

	/* Knights */
	eval_pos += eval_piece(pos->pieceBoards[2 + us_offset],
		pos->whiteToMove ? w_knight_heatmap : b_knight_heatmap);
	eval_pos -= eval_piece(pos->pieceBoards[2 + they_offset],
		!pos->whiteToMove ? w_knight_heatmap : b_knight_heatmap) >> 2;


	return eval_cap * 1000 + eval_pos;

}

int Evaluation::eval_piece(uint64_t piece_board, const int* heatmap)
{
	int eval_pos = 0;
	while (piece_board)
	{
		eval_pos += heatmap[long_bit_scan(piece_board)];
		piece_board &= piece_board - 1;
	}
	return eval_pos;

}

//Return evaluation based on whites perspective
double Evaluation::eval_pawn_structure(Position* pos) {
	//Constants used in calculation
	const uint64_t board = pos->teamBoards[0];
	const uint64_t w_pawns = pos->pieceBoards[1];
	const uint64_t b_pawns = pos->pieceBoards[7];
	const int w_king = long_bit_scan(pos->pieceBoards[0]);
	const int b_king = long_bit_scan(pos->pieceBoards[6]);
	
	//Friendly pieces blocking attacks on the king
	const int w_blockers_for_king = bit_count(king_attacks[w_king] & pos->teamBoards[1]);
	const int b_blockers_for_king = bit_count(king_attacks[w_king] & pos->teamBoards[2]);

	//Number of squares enemy sliders can attack from
	const int w_open_line_danger = long_bit_scan(queen_attacks(board, w_king));
	const int b_open_line_danger = long_bit_scan(queen_attacks(board, b_king));

	//Space advantage
	const int space = pos->whiteToMove ? bit_count(central_white_squares & ~pos->blackAttack): 
										 bit_count(central_black_squares & ~pos->whiteAttack);

	return 0.5*(w_blockers_for_king - b_blockers_for_king) + 0.1*(b_open_line_danger - w_open_line_danger);
}




