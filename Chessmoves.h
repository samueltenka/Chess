#ifndef CHESSMOVES_H
#define CHESSMOVES_H

#include "Chessboard.h"
#include <vector>

struct Coors {int r, c;};
struct SpecialMoveInfo {bool enpassant; bool qcastle, kcastle; Species promotion;};
struct Move {Coors source, dest; SpecialMoveInfo smi;};
Move parse(const char* algebraic);
std::vector<Move> moves_from(const Position p);
Position enact_move(const Position p, const Move m); /*note: enact_move eats p by-value, not by-reference, since we want
                                                       to try multiple moves without the hassle of undoing moves.*/

static bool is_between(int n, int from, int upto) {
   return from<=n && n<upto;
}
static bool on_board(Coors rc) {
   return is_between(rc.r, 0, 8) && is_between(rc.c, 0, 8);
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

static std::vector<Coors> standard_dests_from(const Position p, const Coors rc) {
   std::vector<Coors> dests;
   Piecetype t = p.board[rc.r][rc.c];
   switch(t.s) {
   case empty:
      break;
   case pawn:
      int d = t.c==black ? +1 : -1;
      int n = (rc.r==1 || rc.r==6) ? 2 : 1;
      for(int i=1; i<=n; ++i) {
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

static std::vector<Move> special_moves_from(const Position p, const std::vector<Move> enemy_dests) {
   std::vector<Move> special_moves;
   if(p.ep_target != -1) {
      Coors dest = {(p.turn==black ? 2 : 5), p.ep_target};
      for(int j=-1; j<=1; j=1) {
         Coors source = {(p.turn==black ? 3 : 4), dest.c + j};
         Piecetype t = p.board[source.r][source.c];
         if(t.c==flip(p.turn) && t.s==pawn) {
            special_moves.push_back({source, dest, {true, false, false, empty}});
         }
      }      
   }
   int castle_r = p.turn==black ? 0 : 7;
   Piecetype qr = p.board[castle_r][0],
              k = p.board[castle_r][4],
             kr = p.board[castle_r][7];
   if(!k.has_moved) {
      if(!kr.has_moved) {
         /*    r * * * k * * r
                       ^-^-^    if enemy pieces attack here, or
                         ^-^    if any pieces lie here, then may not castle.
         */
         bool may_castle=true;
         for(int j=5; j<7; ++j) {
            if(p.board[castle_r][j].s != empty) {may_castle = false; break;}
         }
         for(std::vector<Move>::const_iterator i = enemy_dests.begin(); i!= enemy_dests.end(); ++i) {
            if(i->dest.r == may_castle && is_between(i->dest.c, 4, 7)) {may_castle = false; break;}
         }
         if(may_castle) {
            special_moves.push_back({{castle_r, 4}, {castle_r, 6}, {false, false, true, empty}});
         }
      }
      if(!qr.has_moved) {
         /*    r * * * k * * r
                 ^-^-^-^        if enemy pieces attack here, or
                 ^-^-^          if any pieces lie here, then may not castle.
         */
         bool may_castle=true;
         for(int j=1; j<4; ++j) {
            if(p.board[castle_r][j].s != empty) {may_castle = false; break;}
         }
         for(std::vector<Move>::const_iterator i = enemy_dests.begin(); i!= enemy_dests.end(); ++i) {
            if(i->dest.r == may_castle && is_between(i->dest.c, 1, 5)) {may_castle = false; break;}
         }
         if(may_castle) {
            special_moves.push_back({{castle_r, 4}, {castle_r, 6}, {false, true, false, empty}});
         }
      }
   }
}


Position enact_move(const Position p, const Move m) {
   /* the emptied squares also have 'has_moved' set to 'true'. we imagine "moving air bubbles" */
   Position rtrn = p;
   int castle_r = p.turn==black ? 0 : 7;
   rtrn.ep_target = -1; // to be modified (see below) in case a pawn jumps 2 forward.
   if(m.smi.enpassant) {
      Piecetype* s = &rtrn.board[m.source.r][m.source.c];
      Piecetype* d = &rtrn.board[m.dest.r][m.dest.c];
      Piecetype* t = &rtrn.board[m.dest.r + (p.turn==black ? +1 : -1)][m.dest.c];
      *d = *s; d->has_moved = true;
      *s = {grey, empty, true};
      *t = {grey, empty, true};
   } else if(m.smi.qcastle) {
      rtrn.board[castle_r][0] = {grey, empty, true};
      rtrn.board[castle_r][4] = {grey, empty, true};
      rtrn.board[castle_r][2] = {p.turn, king, true};
      rtrn.board[castle_r][3] = {p.turn, rook, true};
   } else if(m.smi.kcastle) {
      rtrn.board[castle_r][4] = {grey, empty, true};
      rtrn.board[castle_r][7] = {grey, empty, true};
      rtrn.board[castle_r][5] = {p.turn, king, true};
      rtrn.board[castle_r][6] = {p.turn, rook, true};
   } else if(m.smi.promotion != empty) {
      Piecetype* s = &rtrn.board[m.source.r][m.source.c];
      Piecetype* d = &rtrn.board[m.dest.r][m.dest.c];
      *d = {p.turn, m.smi.promotion, true}; // 'd->has_moved' we set to true; doesn't affect game, but nicer this way.
                                            // see somewhat related: http://chess.stackexchange.com/questions/8201/did-bobby-fischer-castle-with-a-pawn-promoted-to-a-rook 
      *s = {grey, empty, true};
   } else {
      Piecetype* s = &rtrn.board[m.source.r][m.source.c];
      Piecetype* d = &rtrn.board[m.dest.r][m.dest.c];
      *d = *s; d->has_moved = true;
      *s = {grey, empty, true};
      if(d->s==pawn && (m.dest.r-m.source.r)%2==0) {
         rtrn.ep_target = m.dest.c;
      }
   }
   rtrn.turn = flip(rtrn.turn);
}

#endif //CHESSMOVES_H
