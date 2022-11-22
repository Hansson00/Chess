#include "Evaluation.h"


	int Evaluate(Position* pos) {
		int m_white = 0, m_black = 0;
		for (int i = 1; i < 6; i++) {
			m_white += bit_count(pos->pieceBoards[i]) * piece_value[i];
			m_black += bit_count(pos->pieceBoards[i + 6]) * piece_value[i];
		}
		return pos->whiteToMove ? m_white - m_black : m_black - m_white;
	}
