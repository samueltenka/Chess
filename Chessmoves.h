#include <vector.h>

struct Move;
vector<Move> moves_from(const Position p);
Position enact_move(const Position p); /*note: enact_move eats p by-value, not by-reference, since we want
                                        to try multiple moves without the hassle of undoing moves.*/
