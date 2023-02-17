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

#include <cstdint>

#include "compiler.h"
#include "types.h"

namespace altair {

/** An efficient representation of a set of squares on the chessboard.
 */
class Bitboard {
 public:
  /**
   * Constructs a new bitboard with the initial set of bits.
   */
  explicit constexpr Bitboard(uint64_t bits) : bits_(bits) {}

  /**
   * Constructs a new empty bitboard.
   */
  constexpr Bitboard() : Bitboard(0) {}

  /**
   * Tests the given square for membership in this bitboard.
   */
  constexpr bool test(Square sq) const {
    return (bits_ & (1ULL << static_cast<uint8_t>(sq))) != 0;
  }

  /**
   * Sets a square in the given bitboard.
   */
  constexpr void set(Square sq) { bits_ |= (1ULL << static_cast<uint8_t>(sq)); }

  /**
   * Unsets a square in the given bitboard.
   */
  constexpr void unset(Square sq) {
    bits_ &= ~(1ULL << static_cast<uint8_t>(sq));
  }

  /**
   * Returns true if this bitboard contains no squares.
   */
  constexpr bool empty() const { return bits_ == 0; }

  /**
   * Returns the number of squares set in this bitboard.
   */
  int size() const { return popcount64(bits_); }

  /**
   * Efficiently pops a square from this bitboard.
   */
  constexpr Square pop() {
    if (bits_ == 0) {
      return kNoSquare;
    }

    uint8_t next = count_trailing_zeroes(bits_);
    bits_ &= bits_ - 1;
    return static_cast<Square>(next);
  }

  /**
   * Intersection of two bitboards.
   */
  constexpr Bitboard operator&(const Bitboard& other) const {
    return Bitboard(bits_ & other.bits_);
  }

  /**
   * Union of two bitboards.
   */
  constexpr Bitboard operator|(const Bitboard& other) const {
    return Bitboard(bits_ | other.bits_);
  }

  constexpr Bitboard& operator|=(const Bitboard& other) {
    bits_ |= other.bits_;
    return *this;
  }

  /**
   * Exclusive union of two bitboards.
   */
  constexpr Bitboard operator^(const Bitboard& other) const {
    return Bitboard(bits_ ^ other.bits_);
  }

  /**
   * Bitwise negation of a bitboard.
   */
  constexpr Bitboard operator~() const { return Bitboard(~bits_); }

  /**
   * Bitboard equality.
   */
  constexpr bool operator==(Bitboard other) const {
    return bits_ == other.bits();
  }

  /**
   * Returns the raw bits of this bitboard.
   */
  constexpr uint64_t bits() const { return bits_; }

  /**
   * Assumes that this bitboard contains a single square and extracts the single
   * square from it.
   */
  Square expect_one() && {
    CHECK(size() == 1) << "expected_one called on a bitboard of size "
                       << size();

    Square sq = pop();
    return sq;
  }

  /**
   * Dumps an ASCII representation of this bitboard to standard error.
   */
  void dump() const;

 private:
  uint64_t bits_;
};

constexpr Bitboard kBBRank1 = Bitboard(0x00000000000000FFULL);
constexpr Bitboard kBBRank2 = Bitboard(0x000000000000FF00ULL);
constexpr Bitboard kBBRank3 = Bitboard(0x0000000000FF0000ULL);
constexpr Bitboard kBBRank4 = Bitboard(0x00000000FF000000ULL);
constexpr Bitboard kBBRank5 = Bitboard(0x000000FF00000000ULL);
constexpr Bitboard kBBRank6 = Bitboard(0x0000FF0000000000ULL);
constexpr Bitboard kBBRank7 = Bitboard(0x00FF000000000000ULL);
constexpr Bitboard kBBRank8 = Bitboard(0xFF00000000000000ULL);

constexpr Bitboard kBBFileA = Bitboard(0x0101010101010101ULL);
constexpr Bitboard kBBFileB = Bitboard(0x0202020202020202ULL);
constexpr Bitboard kBBFileC = Bitboard(0x0404040404040404ULL);
constexpr Bitboard kBBFileD = Bitboard(0x0808080808080808ULL);
constexpr Bitboard kBBFileE = Bitboard(0x1010101010101010ULL);
constexpr Bitboard kBBFileF = Bitboard(0x2020202020202020ULL);
constexpr Bitboard kBBFileG = Bitboard(0x4040404040404040ULL);
constexpr Bitboard kBBFileH = Bitboard(0x8080808080808080ULL);

constexpr Bitboard kBBFileAB = kBBFileA | kBBFileB;
constexpr Bitboard kBBFileGH = kBBFileG | kBBFileH;

constexpr Bitboard kBBRank12 = kBBRank1 | kBBRank2;
constexpr Bitboard kBBRank78 = kBBRank7 | kBBRank8;

template <Direction D>
constexpr Bitboard shift(Bitboard base) {
  if constexpr (D == kDirectionNorth) {
    return Bitboard(base.bits() << 8);
  } else if constexpr (D == kDirectionNorthEast) {
    return Bitboard((base & ~kBBFileH).bits() << 9);
  } else if constexpr (D == kDirectionEast) {
    return Bitboard((base & ~kBBFileH).bits() << 1);
  } else if constexpr (D == kDirectionSouthEast) {
    return Bitboard((base & ~kBBFileH).bits() >> 7);
  } else if constexpr (D == kDirectionSouth) {
    return Bitboard(base.bits() >> 8);
  } else if constexpr (D == kDirectionSouthWest) {
    return Bitboard((base & ~kBBFileA).bits() >> 9);
  } else if constexpr (D == kDirectionWest) {
    return Bitboard((base & ~kBBFileA).bits() >> 1);
  } else if constexpr (D == kDirectionNorthWest) {
    return Bitboard((base & ~kBBFileA).bits() << 7);
  } else {
    CHECK(false) << "unimplemented direction pattern for shift";
  }
}

constexpr Bitboard rank_bb(Rank rank) {
  switch (rank) {
    case kRank1:
      return kBBRank1;
    case kRank2:
      return kBBRank2;
    case kRank3:
      return kBBRank3;
    case kRank4:
      return kBBRank4;
    case kRank5:
      return kBBRank5;
    case kRank6:
      return kBBRank6;
    case kRank7:
      return kBBRank7;
    case kRank8:
      return kBBRank8;
    default:
      CHECK(false) << "invalid rank";
      return Bitboard();
  }
}

constexpr Bitboard file_bb(File file) {
  switch (file) {
    case kFileA:
      return kBBFileA;
    case kFileB:
      return kBBFileB;
    case kFileC:
      return kBBFileC;
    case kFileD:
      return kBBFileD;
    case kFileE:
      return kBBFileE;
    case kFileF:
      return kBBFileF;
    case kFileG:
      return kBBFileG;
    case kFileH:
      return kBBFileH;
    default:
      CHECK(false) << "invalid file";
      return Bitboard();
  }
}

/**
 * Given a piece kind, produces all sliding attacks of that piece originating at
 * the given square.
 */
template <PieceKind Kind>
constexpr Bitboard sliding_attack(Square sq, Bitboard occupancy) {
  static_assert(Kind == kRook || Kind == kBishop,
                "Rooks and Bishops are the core sliders");
  constexpr Direction RookDirections[4] = {kDirectionNorth, kDirectionSouth,
                                           kDirectionEast, kDirectionWest};
  constexpr Direction BishopDirections[4] = {
      kDirectionNorthEast, kDirectionNorthWest, kDirectionSouthEast,
      kDirectionSouthWest};
  constexpr Bitboard BishopEdges[4] = {
      kBBRank8 | kBBFileH,
      kBBRank8 | kBBFileA,
      kBBRank1 | kBBFileH,
      kBBRank1 | kBBFileA,
  };
  constexpr Bitboard RookEdges[4] = {
      kBBRank8,
      kBBRank1,
      kBBFileH,
      kBBFileA,
  };

  Bitboard attacks;
  for (int i = 0; i < 4; i++) {
    Direction d = (Kind == kRook ? RookDirections : BishopDirections)[i];
    Bitboard edge = (Kind == kRook ? RookEdges : BishopEdges)[i];

    // For a given direction, cast a ray; stop if we run off of the board or if
    // we hit something in the occupancy board.
    //
    // The first occupied square is included in the attack set, since it might
    // be a capture if it's an enemy piece.
    Square cursor = sq;
    while (!edge.test(cursor)) {
      cursor = towards(cursor, d);
      if (cursor >= kSquareLast) {
        break;
      }

      attacks.set(cursor);
      if (occupancy.test(cursor)) {
        break;
      }
    }
  }

  return attacks;
}

}  // namespace altair