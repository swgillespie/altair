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

#include "bitboard.h"
#include "types.h"

namespace altair {

/**
 * A "magic" entry, in the style of "fancy" magic bitboards
 * https://www.chessprogramming.org/Magic_Bitboards#Fancy
 *
 * There's one SquareMagic for each square on the board. Each square magic
 * perfectly hashes the occupancy of rook and bishop moves on that square into
 * the attack table (to which the attack_ptr member points to).
 */
struct SquareMagic {
  /**
   * Pointer into the attack table for this square.
   *
   * This square addresses the number of attack table entries equal to the
   * cardinality of the power set of the mask.
   */
  Bitboard* attack_ptr;

  /**
   * The mask that, when applied to the occupancy bitboard, selects the relevant
   * squares to produce an occupancy hash key.
   */
  Bitboard mask;

  /**
   * A magic number such that (occupancy & mask) * magic uniquely hashes all of
   * the occupancies addressed by this square. This magic number is found by
   * brute force for each square in gen_magics.cc.
   */
  uint64_t magic;

  /**
   * A shift to turn the result of the magic multiplication into a key while
   * discarding all other potentially garbage bytes.
   */
  unsigned shift;

  constexpr unsigned index(Bitboard occupancy) const {
    return ((occupancy & mask).bits() * magic) >> shift;
  }

  constexpr Bitboard attacks(Bitboard occupancy) const {
    return attack_ptr[index(occupancy)];
  }
};

namespace attacks {

Bitboard kings(Square sq);
Bitboard pawns(Square sq, Color side);
Bitboard knights(Square sq);
Bitboard bishops(Square sq, Bitboard occupancy);
Bitboard rooks(Square sq, Bitboard occupancy);

inline Bitboard queens(Square sq, Bitboard occupancy) {
  return bishops(sq, occupancy) | rooks(sq, occupancy);
}

template <PieceKind Kind>
Bitboard pieces(Square sq, Bitboard occupancy) {
  static_assert(Kind != kPawn, "pawns don't use normal attack tables");
  if constexpr (Kind == kKing) {
    return kings(sq);
  } else if constexpr (Kind == kKnight) {
    return knights(sq);
  } else if constexpr (Kind == kBishop) {
    return bishops(sq, occupancy);
  } else if constexpr (Kind == kRook) {
    return rooks(sq, occupancy);
  } else if constexpr (Kind == kQueen) {
    return queens(sq, occupancy);
  }
}

}  // namespace attacks

}  // namespace altair