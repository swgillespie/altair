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

#include <vector>

#include "position.h"

namespace altair::movegen {

/**
 * Generates pseudolegal moves for the given position.
 *
 * Pseudolegal moves are moves accounting for most, but not all, of the rules of
 * chess. Rules that are enforced later are moves that place the king in check
 * in non-obvious ways, such as absolute pins.
 */
void generate_pseudolegal(const Position& pos, std::vector<Move>& moves);

}  // namespace altair::movegen