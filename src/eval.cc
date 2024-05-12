/*
 *  This file is a part of Altair, a chess engine.
 *  Copyright (C) 2017-2023 Sean Gillespie <sean@swgillespie.me>.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "eval.h"

#include "log.h"

/**
 * Altair's evaluation function is almost exactly Tomasz Michniewski's
 * "Simplified Evaluation Function", for now.
 */

namespace altair::eval {

namespace {

/* clang-format off */

Value kPawnTable[kSquareLast] = {
  //       a   b   c   d   e   f   g   h
  /* 1 */  0,  0,  0,  0,  0,  0,  0,  0,
  /* 2 */  5, 10, 10,-20,-20, 10, 10,  5,
  /* 3 */  5, -5,-10,  0,  0,-10, -5,  5,
  /* 4 */  0,  0,  0, 20, 20,  0,  0,  0,
  /* 5 */  5,  5, 10, 25, 25, 10,  5,  5,
  /* 6 */ 10, 10, 20, 30, 30, 20, 10, 10,
  /* 7 */ 50, 50, 50, 50, 50, 50, 50, 50,
  /* 8 */  0,  0,  0,  0,  0,  0,  0,  0,
};

Value kKnightTable[kSquareLast] = {
  //       a   b   c   d   e   f   g   h
  /* 1 */-50,-40,-30,-30,-30,-30,-40,-50,
  /* 2 */-40,-20,  0,  5,  5,  0,-20,-40,
  /* 3 */-30,  0, 10, 15, 15, 10,  0,-30,
  /* 4 */-30,  5, 15, 20, 20, 15,  5,-30,
  /* 5 */-30,  0, 15, 20, 20, 15,  0,-30,
  /* 6 */-30,  5, 10, 15, 15, 10,  5,-30,
  /* 7 */-40,-20,  0,  5,  5,  0,-20,-40,
  /* 8 */-50,-40,-30,-30,-30,-30,-40,-50,
};

Value kBishopTable[kSquareLast] = {
  //       a   b   c   d   e   f   g   h
  /* 1 */-20,-10,-10,-10,-10,-10,-10,-20,
  /* 2 */-10,  5,  0,  0,  0,  0,  5,-10,
  /* 3 */-10, 10, 10, 10, 10, 10, 10,-10,
  /* 4 */-10,  0, 10, 10, 10, 10,  0,-10,
  /* 5 */-10,  5,  5, 10, 10,  5,  5,-10,
  /* 6 */-10,  0,  5, 10, 10,  5,  0,-10,
  /* 7 */-10,  0,  0,  0,  0,  0,  0,-10,
  /* 8 */-20,-10,-10,-10,-10,-10,-10,-20,
};

Value kRookTable[kSquareLast] = {
  //       a   b   c   d   e   f   g   h
  /* 1 */  0,  0,  0,  5,  5,  0,  0,  0,
  /* 2 */ -5,  0,  0,  0,  0,  0,  0, -5,
  /* 3 */ -5,  0,  0,  0,  0,  0,  0, -5,
  /* 4 */ -5,  0,  0,  0,  0,  0,  0, -5,
  /* 5 */ -5,  0,  0,  0,  0,  0,  0, -5,
  /* 6 */ -5,  0,  0,  0,  0,  0,  0, -5,
  /* 7 */  5, 10, 10, 10, 10, 10, 10,  5,
  /* 8 */  0,  0,  0,  0,  0,  0,  0,  0,
};

Value kQueenTable[kSquareLast] = {
  //       a   b   c   d   e   f   g   h
  /* 1 */-20,-10,-10, -5, -5,-10,-10,-20,
  /* 2 */-10,  0,  5,  0,  0,  0,  0,-10,
  /* 3 */-10,  5,  5,  5,  5,  5,  5,-10,
  /* 4 */  0,  0,  5,  5,  5,  5,  0, -5,
  /* 5 */ -5,  0,  5,  5,  5,  5,  0, -5,
  /* 6 */-10,  0,  5,  5,  5,  5,  0,-10,
  /* 7 */-10,  0,  0,  0,  0,  0,  0,-10,
  /* 8 */-20,-10,-10, -5, -5,-10,-10,-20,
};

Value kPieceValues[kPieceKindLast] = {
  /* P */ 100,
  /* N */ 320,
  /* B */ 330,
  /* R */ 500,
  /* Q */ 900,
  /* K */ 10000,
};

/* clang-format on */

Value evaluate_piece(const Position& pos, Square piece_sq, Color side) {
  Piece piece = pos.piece_at(piece_sq);
  CHECK(piece != kNoPiece) << "no piece at square: evaluate_piece";
  PieceKind kind = kind_of(piece);
  Value value = kPieceValues[kind];
  Square normalized_square =
      side == kWhite ? piece_sq : horizontal_flip(piece_sq);
  switch (kind) {
    case kPawn:
      value += kPawnTable[normalized_square];
      break;
    case kKnight:
      value += kKnightTable[normalized_square];
      break;
    case kBishop:
      value += kBishopTable[normalized_square];
      break;
    case kRook:
      value += kRookTable[normalized_square];
      break;
    case kQueen:
      value += kQueenTable[normalized_square];
      break;
    default:
      break;
  }

  return value;
}

}  // namespace

Value evaluate(const Position& pos) {
  Value white_total = 0;
  Value black_total = 0;

  Bitboard our_pieces = pos.pieces(kWhite);
  Bitboard their_pieces = pos.pieces(kBlack);

  while (!our_pieces.empty()) {
    white_total += evaluate_piece(pos, our_pieces.pop(), kWhite);
  }

  while (!their_pieces.empty()) {
    black_total += evaluate_piece(pos, their_pieces.pop(), kBlack);
  }

  return white_total - black_total;
}

}  // namespace altair::eval
