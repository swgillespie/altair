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

/**
 * Interface to Altair's transposition table.
 *
 * https://www.chessprogramming.org/Transposition_Table
 */

#include "move.h"
#include "position.h"
#include "value.h"

namespace altair::ttable {

void initialize();
void query(const Position& pos);
void record_pv(const Position& pos, Move best, unsigned depth, Value value);
void record_cut(const Position& pos, Move best, unsigned depth, Value value);
void record_all(const Position& pos, unsigned depth, Value value);

}  // namespace altair::ttable