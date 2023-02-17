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

#include <iostream>

namespace altair {

void Bitboard::dump() const {
  std::cerr << std::endl;
  for (int rank = kRank8; rank >= kRank1; rank--) {
    for (int file = kFileA; file < kFileLast; file++) {
      Square sq = square_of(static_cast<File>(file), static_cast<Rank>(rank));
      if (test(sq)) {
        std::cerr << " 1 ";
      } else {
        std::cerr << " . ";
      }
    }

    std::cerr << "| " << rank_char(static_cast<Rank>(rank)) << std::endl;
  }

  for (int i = kFileA; i < kFileLast; i++) {
    std::cerr << "---";
  }
  std::cerr << std::endl;
  for (int i = kFileA; i < kFileLast; i++) {
    std::cerr << " " << static_cast<char>(i + 97) << " ";
  }
  std::cerr << std::endl;
}

}  // namespace altair
