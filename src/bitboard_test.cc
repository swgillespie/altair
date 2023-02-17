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

#include "bitboard.h"

#include "gtest/gtest.h"
#include "types.h"

using altair::Bitboard;

TEST(Bitboard, smoke) {
  Bitboard b;
  ASSERT_TRUE(b.empty());
  b.set(altair::A1);
  ASSERT_FALSE(b.empty());
  ASSERT_EQ(b.size(), 1);
}
