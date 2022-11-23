#include "Evaluation.h"


int Evaluate(Position* pos) {
	int m_white = 0, m_black = 0;
	for (int i = 1; i < 6; i++) {
		m_white += bit_count(pos->pieceBoards[i]) * piece_value[i];
		m_black += bit_count(pos->pieceBoards[i + 6]) * piece_value[i];
	}

	int eval_cap = pos->whiteToMove ? m_white - m_black : m_black - m_white;
	int eval_pos = 0;
	const int* heatmap;

	int us_offset = pos->whiteToMove ? 0 : 6;
	int they_offset = pos->whiteToMove ? 6 : 0;


	/* Pawn */
	heatmap = pos->whiteToMove ? w_pawn_heatmap : b_pawn_heatmap;
	eval_pos += eval_piece(pos->pieceBoards[1 + us_offset], heatmap);
	eval_pos -= eval_piece(pos->pieceBoards[1 + they_offset], heatmap);


	/* KING */
	//heatmap = pos->whiteToMove ? w_king_heatmap : b_king_heatmap;
	heatmap = king_late_game;
	eval_pos += eval_piece(pos->pieceBoards[0 + us_offset], heatmap);
	eval_pos -= eval_piece(pos->pieceBoards[0 + they_offset], heatmap);

	/* Rooks */
	heatmap = pos->whiteToMove ? w_rook_heatmap : b_rook_heatmap;
	eval_pos += eval_piece(pos->pieceBoards[4 + us_offset], heatmap);
	eval_pos -= eval_piece(pos->pieceBoards[4 + they_offset], heatmap);

	return eval_cap + eval_pos;
}

int eval_piece(uint64_t piece_board, const int* heatmap)
{
	int eval_pos = 0;
	while (piece_board)
	{
		eval_pos += 10 * heatmap[long_bit_scan(piece_board)];
		piece_board &= piece_board - 1;
	}
	return eval_pos;

}




