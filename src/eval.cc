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

#include <array>

#include "types.h"

namespace altair::eval {

namespace {

class V {
 public:
  constexpr explicit V(int16_t all_game) : V(all_game, all_game) {}

  constexpr V(int16_t mid_game, int16_t end_game)
      : mid_game_(mid_game), end_game_(end_game) {}

  constexpr int16_t evaluate(const Position& pos) const { return mid_game_; }

 private:
  int16_t mid_game_;
  int16_t end_game_;
};

// clang-format off
constexpr std::array<V, kSquareLast> kPawnSquareTable = {
    V(0),  V(0),  V(0),   V(0),   V(0),   V(0),   V(0),  V(0),
    V(5),  V(10), V(10),  V(-20), V(-20), V(10),  V(10), V(5),
    V(5),  V(-5), V(-10), V(0),   V(0),   V(-10), V(-5), V(5),
    V(0),  V(0),  V(0),   V(20),  V(20),  V(0),   V(0),  V(0),
    V(5),  V(5),  V(10),  V(25),  V(25),  V(10),  V(5),  V(5),
    V(10), V(10), V(20),  V(30),  V(30),  V(20),  V(10), V(10),
    V(50), V(50), V(50),  V(50),  V(50),  V(50),  V(50), V(50),
    V(0),  V(0),  V(0),   V(0),   V(0),   V(0),   V(0),  V(0),
};

constexpr std::array<V, kSquareLast> kKnightSquareTable = {
    V(-50), V(-40), V(-30), V(-30), V(-30), V(-30), V(-40), V(-50),
    V(-40), V(-20), V( 0),  V( 5),  V( 5),  V( 0),  V(-20), V(-40),
    V(-30), V(5),   V( 10), V( 15), V( 15), V( 10), V( 5),  V(-30),
    V(-30), V(0),   V( 15), V( 20), V( 20), V( 15), V( 0),  V(-30),
    V(-30), V(5),   V(15),  V( 20), V( 20), V( 15), V( 5),  V(-30),
    V(-30), V(0),   V(10),  V( 15), V( 15), V( 10), V( 0),  V(-30),
    V(-40), V(-20), V(0),   V(0),   V( 0),  V( 0),  V(-20), V(-40),
    V(-50), V(-40), V(-30), V(-30), V(-30), V(-30), V(-40), V(-50),
};
// clang-format on

template <Color Us>
constexpr uint16_t evaluate_table(const Position& pos,
                                  const std::array<V, kSquareLast>& table,
                                  Square sq) {
  if constexpr (Us == kBlack) {
    sq = flip(sq);
  }

  return table.at(sq).evaluate(pos);
}

template <PieceKind Kind>
consteval int16_t piece_value() {
  if constexpr (Kind == kPawn) {
    return 100;
  } else if constexpr (Kind == kKnight) {
    return 300;
  } else if constexpr (Kind == kBishop) {
    return 300;
  } else if constexpr (Kind == kRook) {
    return 500;
  } else if constexpr (Kind == kQueen) {
    return 900;
  } else if constexpr (Kind == kKing) {
    return 10000;
  }
}

template <Color Us, PieceKind Kind>
int16_t evaluate_piece(const Position& pos,
                       const std::array<V, kSquareLast>& table) {
  constexpr int16_t Value = piece_value<Kind>();

  int score = 0;
  Bitboard pieces = pos.pieces(Us, Kind);
  while (!pieces.empty()) {
    score += Value + evaluate_table<Us>(pos, table, pieces.pop());
  }

  return score;
}

template <Color Us>
int16_t evaluate_pawns(const Position& pos) {
  return evaluate_piece<Us, kPawn>(pos, kPawnSquareTable);
}

template <Color Us>
int16_t evaluate_knights(const Position& pos) {
  return evaluate_piece<Us, kKnight>(pos, kKnightSquareTable);
}

}  // namespace

/**
 * Altair's evaluation function.
 */
Value evaluate(const Position& pos) {
  int16_t pawns = evaluate_pawns<kWhite>(pos) - evaluate_pawns<kBlack>(pos);
  int16_t knights =
      evaluate_knights<kWhite>(pos) - evaluate_knights<kBlack>(pos);
  return Value(pawns);
}

}  // namespace altair::eval
