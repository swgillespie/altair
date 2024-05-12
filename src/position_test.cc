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

#include "position.h"

#include "bitboard.h"
#include "gtest/gtest.h"
#include "move.h"
#include "types.h"

using altair::Bitboard;
using altair::Move;
using altair::Position;

TEST(Position, piece_smoke) {
  Position pos;
  ASSERT_EQ(pos.piece_at(altair::A4), altair::kNoPiece);
  pos.add_piece(altair::kWhiteRook, altair::A4);
  ASSERT_EQ(pos.piece_at(altair::A4), altair::kWhiteRook);
  ASSERT_EQ(pos.remove_piece(altair::A4), altair::kWhiteRook);
  ASSERT_EQ(pos.piece_at(altair::A4), altair::kNoPiece);
}

TEST(Position, fen_roundtrip) {
  const char* start =
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  Position pos;
  pos.set(start);
  ASSERT_EQ(pos.fen(), start);
}

TEST(Position, make_unmake_smoke) {
  Position pos;
  pos.set("5k2/4q3/8/8/8/2Q5/8/1K6 w - - 0 1");
  ASSERT_EQ(pos.piece_at(altair::C3), altair::kWhiteQueen);
  Move mov = Move::quiet(altair::C3, altair::G3);
  pos.make_move(mov);
  ASSERT_EQ(pos.piece_at(altair::C3), altair::kNoPiece);
  ASSERT_EQ(pos.piece_at(altair::G3), altair::kWhiteQueen);
  ASSERT_EQ(pos.side_to_move(), altair::kBlack);
  ASSERT_EQ(pos.ply(), 1);
  ASSERT_EQ(pos.halfmove_clock(), 1);
  pos.unmake_move(mov);
  ASSERT_EQ(pos.piece_at(altair::C3), altair::kWhiteQueen);
  ASSERT_EQ(pos.piece_at(altair::G3), altair::kNoPiece);
  ASSERT_EQ(pos.ply(), 0);
  ASSERT_EQ(pos.halfmove_clock(), 0);
}

TEST(Position, make_unmake_capture) {
  Position pos;
  pos.set("4k3/8/8/8/8/2Q3q1/8/1K6 w - - 5 7");
  Move mov = Move::capture(altair::C3, altair::G3);
  pos.make_move(mov);
  ASSERT_EQ(pos.fen(), "4k3/8/8/8/8/6Q1/8/1K6 b - - 0 7");
  pos.unmake_move(mov);
  ASSERT_EQ(pos.fen(), "4k3/8/8/8/8/2Q3q1/8/1K6 w - - 5 7");
}

TEST(Position, squares_attacking_smoke) {
  Position pos;
  pos.set("6k1/R7/8/2P5/5B2/5N1P/3R2P1/1K6 w - - 0 1");
  Bitboard attackers = pos.squares_attacking(altair::D6, altair::kWhite);
  ASSERT_EQ(attackers.size(), 3);
  ASSERT_TRUE(attackers.test(altair::C5));
  ASSERT_TRUE(attackers.test(altair::D2));
  ASSERT_TRUE(attackers.test(altair::F4));
}

TEST(Position, kingside_castle_smoke) {
  Position pos;
  pos.set(
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  pos.make_move(Move::kingside_castle(altair::E1, altair::G1));
  ASSERT_EQ(
      pos.fen(),
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R4RK1 b kq - 1 1");
  pos.unmake_move(Move::kingside_castle(altair::E1, altair::G1));
  ASSERT_EQ(
      pos.fen(),
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
}

TEST(Position, promo_smoke) {
  Position pos;
  pos.set(
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/PPN2Q2/2PBBPpP/R3K2R b KQkq - 0 2");
  pos.make_move(Move::promotion(altair::G2, altair::G1, altair::kQueen));
  ASSERT_EQ(
      pos.fen(),
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/PPN2Q2/2PBBP1P/R3K1qR w KQkq - 0 3");
  pos.unmake_move(Move::promotion(altair::G2, altair::G1, altair::kQueen));
  ASSERT_EQ(
      pos.fen(),
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/PPN2Q2/2PBBPpP/R3K2R b KQkq - 0 2");
}

TEST(Position, king_move_prevents_castling) {
  Position pos;
  pos.set(
      "r3k2r/p1ppqpb1/1n2pnp1/3PN3/1p2P3/2N2Q1p/PPPB1PPP/R3Kb1R w KQkq - 0 2");
  pos.make_move(Move::capture(altair::E1, altair::F1));
  ASSERT_FALSE(pos.can_castle_kingside(altair::kWhite));
  ASSERT_FALSE(pos.can_castle_queenside(altair::kWhite));
}

TEST(Position, piece_square_move_modifies_hash) {
  Position pos;
  pos.set("8/p3kp2/1n6/8/3K4/8/P4P2/8 w - - 4 59");
  size_t hash = std::hash<Position>{}(pos);
  pos.make_move(Move::quiet(altair::D4, altair::D5));
  size_t new_hash = std::hash<Position>{}(pos);
  ASSERT_NE(hash, new_hash);
}

TEST(Position, fen_set_has_nonzero_hash) {
  Position pos;
  pos.set("8/p3kp2/1n6/8/3K4/8/P4P2/8 w - - 4 59");
  size_t hash = std::hash<Position>{}(pos);
  ASSERT_NE(hash, 0);
}