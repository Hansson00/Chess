#include "Evaluation.h"
#include "Evaluation.h"


int Evaluate(Position* pos) {
    int m_white = 0, m_black = 0;
    for (int i = 1; i < 6; i++) {
        m_white += bit_count(pos->pieceBoards[i]) * piece_value[i];
        m_black += bit_count(pos->pieceBoards[i + 6]) * piece_value[i];
    }

    int eval_cap = pos->whiteToMove ? m_white - m_black : m_black - m_white;

    int eval_pos = 0;

    if (pos->whiteToMove)
    {
        uint64_t pawn = pos->pieceBoards[1];
        while (pawn)
        {
            int p = long_bit_scan(pawn);
            eval_pos += w_pawn_heatmap[p];
            pawn &= pawn - 1;
        }
        uint64_t king = pos->pieceBoards[0];
        while (king)
        {
            int k = long_bit_scan(king);
            eval_pos += w_king_heatmap[k];
            king &= king - 1;
        }
    }
    else
    {
        uint64_t pawn = pos->pieceBoards[7];
        while (pawn)
        {
            int p = long_bit_scan(pawn);
            eval_pos += b_pawn_heatmap[p];
            pawn &= pawn - 1;
        }
        uint64_t king = pos->pieceBoards[6];
        while (king)
        {
            int k = long_bit_scan(king);
            eval_pos += b_king_heatmap[k];
            king &= king - 1;
        }
    }


    return eval_cap * 10 + eval_pos;
}