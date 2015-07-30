#include "Chessboard.h"
#include "Chessmoves.h"

void isolate(int r, int c) {
   Position p;
   init_pos(p);
   p.board[r][c] = {grey, empty, false};
   
   print_pos(p);
   std::vector<Move> moves = moves_from(p);
   for(std::vector<Move>::const_iterator i=moves.begin(); i!=moves.end(); ++i) {
      printf("%d %d ---> %d %d\n", i->source.r, i->source.c, i->dest.r, i->dest.c);
   }
}

void main() {
   isolate(6, 1);
   isolate(6, 2);
   isolate(6, 3);
   isolate(6, 4);
   char L; scanf_s("%c", &L);
}
