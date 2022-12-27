#include "Opening_book.h"

Opening_book::Opening_book()
{

	FILE* pFile = fopen("gm2600.bin", "rb");
	
	if (pFile == NULL) 
	{
		std::cout << "Book File Not Read\n";
	}
	else 
	{
		fseek(pFile, 0, SEEK_END);
		long position = ftell(pFile);

		if (position < sizeof(S_BOOK_ENTRY))
		{
			std::cout << "No Entries\n";
			return;
		}

		num_entries = position / sizeof(S_BOOK_ENTRY);
		entries = (S_BOOK_ENTRY*)malloc(num_entries * sizeof(S_BOOK_ENTRY));
		rewind(pFile);
		fread(entries, sizeof(S_BOOK_ENTRY), num_entries, pFile);
		
		for (int i = 0; i < num_entries; i++) {
			//Swap endian format
			S_BOOK_ENTRY* entry = &entries[i];
			entry->key = _byteswap_uint64(entry->key);
			entry->move = _byteswap_ushort(entry->move);
			entry->weight = _byteswap_ushort(entry->weight);
			entry->learn = _byteswap_ulong(entry->learn);
			//Restructure the move to be encoded as ours
			//TODO: fix castling moves
			uint16_t move = entry->move;
			uint16_t to_sq = (move & 0x7) + (7 - ((move >> 3) & 0x7)) * 8;
			uint16_t from_sq = ((move >> 6) & 0x7) + (7 - ((move >> 9) & 0x7)) * 8;
			uint16_t flags = move >> 12 == 0 ? 0 : 0x8000 | (move >> 12);
			uint16_t our_move = to_sq | (from_sq << 6) | flags;
			entry->move = our_move;
		}
	}
}


Opening_book::~Opening_book()
{
	delete(entries);
}

uint64 Opening_book::hash_funciton(const Position* pos) const
{
	uint64 final_key = 0;

	/* Pieces */
	for (int i = 0; i < 12; i++) 
	{
		uint64 offset_piece;
		uint64 piece_borad = pos->pieceBoards[i];

		while (piece_borad)
		{
			const int bit = long_bit_scan(piece_borad);
			const int row = 7 - (bit >> 3);
			const int file = bit % 8;
			final_key ^= Random64[(64 * poly_piece[i]) + (row << 3) + file];
			piece_borad &= piece_borad - 1;
		}
	}

	/* Castling */
	if (pos->castlingRights & 1) final_key ^= Random64[CASTLE+0];
	if (pos->castlingRights & 2) final_key ^= Random64[CASTLE+1];
	if (pos->castlingRights & 4) final_key ^= Random64[CASTLE+2];
	if (pos->castlingRights & 8) final_key ^= Random64[CASTLE+3];

	/* Enpassant */
	if (pos->enPassant != 0xFF) {
		const int col = (pos->enPassant & 0xFF) % 8;
		final_key ^= Random64[ENPASSANT + col];
	}

	if (pos->whiteToMove) final_key ^= Random64[TURN];

	return final_key;
}

uint16_t Opening_book::find_book_move(const Position* pos) const{
	
	uint64 p_key = hash_funciton(pos);
	int start = 0;
	S_BOOK_ENTRY* entry;
	uint16_t move;
	std::vector<uint16_t> book_moves;
	for (entry = entries; entry < entries + num_entries; entry++)
	{
		if (p_key == entry->key)
		{
			//TODO: take a random one of the suggested moves
			book_moves.push_back(entry->move);
		}
	}

	if (book_moves.size()) {
		srand(time(NULL));
		return book_moves[rand() % book_moves.size()];
	}
		
	return 0;

}

/*
King = 0
Pawns = 1
Knights = 2
Bishops = 3
Rooks = 4
Queen = 5
*/