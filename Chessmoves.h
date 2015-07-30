#ifndef CHESSMOVES_H
#define CHESSMOVES_H

#include "Chessboard.h"
#include <vector>

struct Coors {int r, c;};
struct Move {Coors source, dest;};
Move parse(const char* algebraic);
std::vector<Move> moves_from(const Position p);
Position enact_move(const Position p); /*note: enact_move eats p by-value, not by-reference, since we want
                                        to try multiple moves without the hassle of undoing moves.*/

static bool on_board(Coors rc) {
   return 0<=rc.r && rc.r<8 &&
          0<=rc.c && rc.c<8;
}
Color flip(Color c) {
   return c==white? black : c==black? white : grey;
}
static bool append(const Position p, std::vector<Coors> &dests, const Coors source, int rdisp, int cdisp, bool can_take=true) {
   /*Returns 'true' if the next move in sequence of possible moves would be illegal.
     For example, consider the diagonal ray from a black bishop on (0,0):
     after (+2,+2) comes (+3,+3), which is illegal if (+2,+2) goes off the board or is also black,
     or if (+2,+2) is white. In the latter illegal case, as well as the legal case,
     before returning true, we append the destination to 'dests'*/
   /*parameter 'can_take' allows for the annoying fact that pawns take differently than they move.*/
   Color turn = p.board[source.r][source.c].c;
   Coors dest = {source.r+rdisp, source.c+cdisp};
   if(!on_board(source)) {return true;}
   Color taken = p.board[dest.r][dest.c].c;
   if(taken==turn) {return true;}
   if(can_take || taken==grey) {
      dests.push_back(dest);
   }   
   if(taken==flip(turn)) {return true;}
   return false;
}

static std::vector<Coors> moves_from(const Position p, const Coors rc) {
   std::vector<Coors> dests;
   Piecetype t = p.board[rc.r][rc.c];
   switch(t.s) {
   case empty:
      break;
   case pawn:
      int d = t.c==black ? +1 : -1;
      int n = (rc.r==1 || rc.r==6) ? 2 : 1;
      for(int i=0; i<n; ++i) {
         if(append(p, dests, rc, d*i, 0, false)) {break;}
      }
      append(p, dests, rc, d, -1); append(p, dests, rc, d, +1);
      break;
   case knight:
      for(int i=-2; i<=2; ++i) {
         for(int j=-2; j<=2; ++j) {
            if((i*j)%4 != 2) {continue;}
            append(p, dests, rc, i, j);
         }
      } break;
   case bishop:
      for(int i=-1; i<=1; i=1) {
         for(int j=-1; j<=1; j=1) {
            for(int n=1; n<8; ++n) {
               if(append(p, dests, rc, n*i, n*j)) {break;}
            }
         }
      } break;
   case rook:
      for(int i=-1; i<=1; ++i) {
         for(int j=-1; j<=1; ++j) {
            if((i+j)%2 != 1) {continue;}
            for(int n=1; n<8; ++n) {
               if(append(p, dests, rc, n*i, n*j)) {break;}
            }
         }
      } break;
   case queen:
      for(int i=-1; i<=1; ++i) {
         for(int j=-1; j<=1; ++j) {
            if(i*j == 0) {continue;}
            for(int n=1; n<8; ++n) {
               if(append(p, dests, rc, n*i, n*j)) {break;}
            }
         }
      } break;
   case king:
      for(int i=-1; i<=1; ++i) {
         for(int j=-1; j<=1; ++j) {
            if(i*j == 0) {continue;}
            append(p, dests, rc, i, j);
         }
      } break;
   }
   return dests;
}

#endif //CHESSMOVES_H
