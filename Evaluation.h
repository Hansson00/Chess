#pragma once
#include "Chess_utility.h"


	const int piece_value[6] = { 0, 1, 3, 3, 5, 10 };

	int Evaluate(Position* pos);
