/*
 *  This file is a part of Altair, a chess engine.
 *  Copyright (C) 2017-2024 Sean Gillespie <sean@swgillespie.me>.
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

#include <cstdint>

#include "types.h"

namespace altair::zobrist {

void modify_piece(uint64_t *hash, Square square, Piece piece);
void modify_side_to_move(uint64_t *hash);
void modify_kingside_castle(uint64_t *hash, Color color);
void modify_queenside_castle(uint64_t *hash, Color color);
void modify_en_passant(uint64_t *hash, Square oldSquare, Square newSquare);

}