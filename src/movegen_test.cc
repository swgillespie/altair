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

#include "movegen.h"

#include <initializer_list>
#include <vector>

#include "gtest/gtest.h"
#include "move.h"

using altair::Move;
using altair::PieceKind;
using altair::Position;

namespace {

void assert_moves(const Position& pos,
                  std::initializer_list<Move> expected_moves) {
  std::vector<Move> moves;
  altair::movegen::generate_pseudolegal(pos, moves);
  for (Move expected : expected_moves) {
    bool seen_move = false;
    for (Move move : moves) {
      if (move == expected) {
        seen_move = true;
        break;
      }
    }

    if (!seen_move) {
      FAIL() << "did not see expected move '" << expected.as_uci()
             << "' in generated moves";
    }
  }
}

void assert_not_moves(const Position& pos,
                      std::initializer_list<Move> unexpected_moves) {
  std::vector<Move> moves;
  altair::movegen::generate_pseudolegal(pos, moves);
  for (Move unexpected : unexpected_moves) {
    for (Move move : moves) {
      if (move == unexpected) {
        FAIL() << "saw banned move '" << unexpected.as_uci()
               << "' in generated moves";
      }
    }
  }
}

}  // namespace

TEST(Movegen, pawn_advance_smoke) {
  Position pos;
  pos.set("4k3/8/8/8/8/4P3/8/1K6 w - - 0 1");
  assert_moves(pos, {Move::quiet(altair::E3, altair::E4)});
}

TEST(Movegen, double_pawn_advance) {
  Position pos;
  pos.set("4k3/8/8/8/8/5P2/3P4/1K6 w - - 0 1");
  assert_moves(pos, {Move::quiet(altair::D2, altair::D3),
                     Move::quiet(altair::F3, altair::F4),
                     Move::double_pawn_push(altair::D2, altair::D4)});
  assert_not_moves(pos, {Move::double_pawn_push(altair::F3, altair::F5)});
}

TEST(Movegen, pawn_captures) {
  Position pos;
  pos.set("2k5/8/8/6p1/2p2P2/3P4/8/1K6 w - - 0 1");
  assert_moves(pos, {Move::capture(altair::D3, altair::C4),
                     Move::capture(altair::F4, altair::G5)});
  assert_not_moves(pos, {Move::capture(altair::D3, altair::E4),
                         Move::capture(altair::F4, altair::E5)});
}

TEST(Movegen, pawn_promotions) {
  Position pos;
  pos.set("3p2k1/2P5/8/8/8/8/8/1K6 w - - 0 1");
  for (PieceKind kind :
       {altair::kKnight, altair::kBishop, altair::kRook, altair::kQueen}) {
    assert_moves(pos, {Move::promotion(altair::C7, altair::C8, kind),
                       Move::promotion_capture(altair::C7, altair::D8, kind)});
    assert_not_moves(pos,
                     {Move::promotion_capture(altair::C7, altair::B8, kind),
                      Move::quiet(altair::C7, altair::C8)});
  }
}

TEST(Movegen, sliding_smoke) {
  Position pos;
  pos.set("4k3/8/8/8/8/8/4B3/1K6 w - - 0 1");
  assert_moves(pos, {
                        Move::quiet(altair::E2, altair::F1),
                        Move::quiet(altair::E2, altair::D3),
                        Move::quiet(altair::E2, altair::C4),
                        Move::quiet(altair::E2, altair::B5),
                        Move::quiet(altair::E2, altair::A6),
                        Move::quiet(altair::E2, altair::F3),
                        Move::quiet(altair::E2, altair::G4),
                        Move::quiet(altair::E2, altair::H5),
                    });
}

TEST(Movegen, sliding_capture) {
  Position pos;
  pos.set("5k2/8/8/1b6/8/8/4B3/1K6 w - - 0 1");
  assert_moves(pos, {
                        Move::quiet(altair::E2, altair::D1),
                        Move::quiet(altair::E2, altair::D3),
                        Move::quiet(altair::E2, altair::C4),
                        Move::capture(altair::E2, altair::B5),
                    });
  assert_not_moves(pos, {
                            Move::quiet(altair::E2, altair::A5),
                            Move::capture(altair::E2, altair::F3),
                            Move::capture(altair::E2, altair::F1),
                        });
}

TEST(Movegen, knight_smoke) {
  Position pos;
  pos.set("1k6/8/5b2/4R3/6N1/8/8/1K6 w - - 0 1");
  assert_moves(pos, {
                        Move::quiet(altair::G4, altair::E3),
                        Move::quiet(altair::G4, altair::F2),
                        Move::quiet(altair::G4, altair::H2),
                        Move::capture(altair::G4, altair::F6),
                    });
  assert_not_moves(pos, {
                            Move::quiet(altair::G4, altair::E5),
                            Move::capture(altair::G4, altair::E5),
                        });
}

TEST(Movegen, king_smoke) {
  Position pos;
  pos.set("3k4/8/8/8/8/2K5/1Pr5/8 w - - 0 1");
  assert_moves(pos, {
                        Move::quiet(altair::C3, altair::B3),
                        Move::quiet(altair::C3, altair::B4),
                        Move::quiet(altair::C3, altair::C4),
                        Move::quiet(altair::C3, altair::D4),
                        Move::quiet(altair::C3, altair::D3),
                        Move::quiet(altair::C3, altair::D2),
                        Move::capture(altair::C3, altair::C2),
                    });
  assert_not_moves(pos, {
                            Move::quiet(altair::C3, altair::B2),
                        });
}