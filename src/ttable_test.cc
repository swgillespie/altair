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

#include "ttable.h"

#include "gtest/gtest.h"
#include "position.h"
#include "value.h"

using altair::Move;
using altair::Position;
using altair::Square;
using altair::TableEntry;
using altair::Value;

class TTableTest : public ::testing::Test {
  void SetUp() override { altair::ttable::initialize(4 /* MB */); }
  void TearDown() override { altair::ttable::destroy(); }
};

TEST_F(TTableTest, query_miss) {
  Position pos;
  pos.set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  auto found = altair::ttable::query(
      pos, [&pos = std::as_const(pos)](const TableEntry& entry) {
        return entry.zobrist_key == pos.hash();
      });
  EXPECT_FALSE(found);
}

TEST_F(TTableTest, query_pv) {
  Position pos;
  pos.set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  altair::ttable::record_pv(pos, Move::quiet(Square::A2, Square::A4), 10,
                            Value(100));
  auto found = altair::ttable::query(
      pos, [&pos = std::as_const(pos)](const TableEntry& entry) -> bool {
        EXPECT_EQ(entry.zobrist_key, pos.hash());
        EXPECT_EQ(entry.move, Move::quiet(Square::A2, Square::A4));
        EXPECT_EQ(entry.value, Value(100));
        EXPECT_EQ(entry.depth, 10);
        return true;
      });
  EXPECT_TRUE(found);
}
