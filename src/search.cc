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

#include "search.h"

#include <chrono>
#include <cstdint>
#include <unordered_map>

#include "movegen.h"

namespace altair {

template <bool Root>
uint64_t perft(Position& pos, unsigned depth) {
  if (depth == 0) {
    return 1;
  }

  [[maybe_unused]] std::chrono::time_point<std::chrono::system_clock> start;
  [[maybe_unused]] std::chrono::time_point<std::chrono::system_clock> end;
  if constexpr (Root) {
    start = std::chrono::system_clock::now();
  }

  std::vector<Move> moves;
  moves.reserve(224);
  movegen::generate_pseudolegal(pos, moves);
  uint64_t running_total = 0;
  for (auto move : moves) {
    pos.make_move(move);
    if (!pos.is_check(!pos.side_to_move())) {
      uint64_t child_nodes = perft<false>(pos, depth - 1);
      if constexpr (Root) {
        UCI() << move.as_uci() << ": " << child_nodes;
      }

      running_total += child_nodes;
    }
    pos.unmake_move(move);
  }

  if constexpr (Root) {
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    UCI() << "Nodes searched: " << running_total;
    UCI() << "Elapsed time: " << diff.count();
    UCI() << "Nodes per second: " << static_cast<uint64_t>(running_total / diff.count());
  }
  return running_total;
}

Searcher::Searcher(Position& pos, SearchLimits limits)
    : pos_(pos), limits_(limits) {}

void Searcher::search() {
  CHECK(limits_.perft != 0) << "PERFT search only right now";
  perft<true>(pos_, limits_.perft);
}

}  // namespace altair
