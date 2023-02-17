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

#pragma once

#include "position.h"

namespace altair {

/**
 * Ways to limit the search.
 */
struct SearchLimits {
  /**
   * If nonzero, this search is a perft search with the given depth.
   */
  unsigned perft;
};

class Searcher {
 public:
  Searcher(Position& pos, SearchLimits limits);

  void search();

 private:
  Position& pos_;
  SearchLimits limits_;
};

}  // namespace altair