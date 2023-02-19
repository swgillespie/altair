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

#include <functional>

#include "log.h"
#include "types.h"

namespace altair {

class Move {
 public:
  friend struct std::hash<Move>;

  Move() : bits_(0) {}

  static Move quiet(Square src, Square dst) { return Move(src, dst); }

  static Move capture(Square src, Square dst) {
    Move mov(src, dst);
    mov.bitset_.capture_bit_ = 1;
    return mov;
  }

  static Move en_passant(Square src, Square dst) {
    Move mov = capture(src, dst);
    mov.bitset_.special_1_bit_ = 1;
    return mov;
  }

  static Move double_pawn_push(Square src, Square dst) {
    Move mov = quiet(src, dst);
    mov.bitset_.special_1_bit_ = 1;
    return mov;
  }

  static Move promotion(Square src, Square dst, PieceKind kind) {
    Move mov = quiet(src, dst);
    mov.bitset_.promotion_bit_ = 1;
    switch (kind) {
      case kBishop:
        mov.bitset_.special_1_bit_ = 1;
        break;
      case kRook:
        mov.bitset_.special_0_bit_ = 1;
        break;
      case kQueen:
        mov.bitset_.special_0_bit_ = 1;
        mov.bitset_.special_1_bit_ = 1;
        break;
      default:
        break;
    }
    return mov;
  }

  static Move promotion_capture(Square src, Square dst, PieceKind kind) {
    Move mov = promotion(src, dst, kind);
    mov.bitset_.capture_bit_ = 1;
    return mov;
  }

  static Move kingside_castle(Square src, Square dst) {
    Move mov = quiet(src, dst);
    mov.bitset_.special_0_bit_ = 1;
    return mov;
  }

  static Move queenside_castle(Square src, Square dst) {
    Move mov = quiet(src, dst);
    mov.bitset_.special_0_bit_ = 1;
    mov.bitset_.special_1_bit_ = 1;
    return mov;
  }

  static Move null() { return quiet(Square::A1, Square::A1); }

  Square source() const { return static_cast<Square>(bitset_.source_); }

  Square destination() const { return static_cast<Square>(bitset_.dest_); }

  PieceKind promotion_piece() const {
    CHECK(bitset_.promotion_bit_)
        << "PromotionPiece only valid on promotion moves";
    if (bitset_.special_0_bit_ && bitset_.special_1_bit_) {
      return kQueen;
    }
    if (bitset_.special_0_bit_) {
      return kRook;
    }
    if (bitset_.special_1_bit_) {
      return kBishop;
    }
    return kKnight;
  }

  bool is_quiet() const {
    return bitset_.promotion_bit_ == 0 && bitset_.capture_bit_ == 0 &&
           bitset_.special_0_bit_ == 0 && bitset_.special_1_bit_ == 0;
  }

  bool is_capture() const { return bitset_.capture_bit_; }

  bool is_null() const { return bits_ == 0; }

  bool is_kingside_castle() const {
    return !bitset_.promotion_bit_ && !bitset_.capture_bit_ &&
           bitset_.special_0_bit_ && !bitset_.special_1_bit_;
  }

  bool is_queenside_castle() const {
    return !bitset_.promotion_bit_ && !bitset_.capture_bit_ &&
           bitset_.special_0_bit_ && bitset_.special_1_bit_;
  }

  bool is_castle() const {
    return is_kingside_castle() || is_queenside_castle();
  }

  bool is_promotion() const { return bitset_.promotion_bit_; }

  bool is_double_pawn_push() const {
    return !bitset_.promotion_bit_ && !bitset_.capture_bit_ &&
           !bitset_.special_0_bit_ && bitset_.special_1_bit_;
  }

  bool is_en_passant() const {
    return !bitset_.promotion_bit_ && bitset_.capture_bit_ &&
           !bitset_.special_0_bit_ && bitset_.special_1_bit_;
  }

  std::string as_uci() const {
    if (is_null()) {
      return "0000";
    }

    std::stringstream str;
    str << square_string(source());
    str << square_string(destination());
    if (is_promotion()) {
      switch (promotion_piece()) {
        case kBishop:
          str << "b";
          break;
        case kKnight:
          str << "n";
          break;
        case kRook:
          str << "r";
          break;
        case kQueen:
          str << "q";
          break;
        default:
          break;
      }
    }
    return str.str();
  }

  bool operator==(const Move other) const { return other.bits_ == bits_; }

 private:
  Move(Square src, Square dst) {
    this->bitset_.promotion_bit_ = 0;
    this->bitset_.capture_bit_ = 0;
    this->bitset_.source_ = static_cast<uint8_t>(src);
    this->bitset_.special_0_bit_ = 0;
    this->bitset_.special_1_bit_ = 0;
    this->bitset_.dest_ = static_cast<uint8_t>(dst);
  }

  struct Bitset {
    bool promotion_bit_ : 1;
    bool capture_bit_ : 1;
    uint8_t source_ : 6;
    bool special_0_bit_ : 1;
    bool special_1_bit_ : 1;
    uint8_t dest_ : 6;
  };

  static_assert(sizeof(Bitset) == sizeof(uint16_t));
  union {
    Bitset bitset_;
    uint16_t bits_;
  };
};

static_assert(sizeof(Move) == sizeof(uint16_t));

}  // namespace altair

namespace std {

template <>
struct hash<altair::Move> {
  size_t operator()(const altair::Move& mov) const {
    return hash<int>{}(mov.bits_);
  }
};

}  // namespace std