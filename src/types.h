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
#include <string>
#include <utility>

#include "log.h"

namespace altair {

enum Square : uint8_t {
  A1,
  B1,
  C1,
  D1,
  E1,
  F1,
  G1,
  H1,
  A2,
  B2,
  C2,
  D2,
  E2,
  F2,
  G2,
  H2,
  A3,
  B3,
  C3,
  D3,
  E3,
  F3,
  G3,
  H3,
  A4,
  B4,
  C4,
  D4,
  E4,
  F4,
  G4,
  H4,
  A5,
  B5,
  C5,
  D5,
  E5,
  F5,
  G5,
  H5,
  A6,
  B6,
  C6,
  D6,
  E6,
  F6,
  G6,
  H6,
  A7,
  B7,
  C7,
  D7,
  E7,
  F7,
  G7,
  H7,
  A8,
  B8,
  C8,
  D8,
  E8,
  F8,
  G8,
  H8,
  kSquareLast,
  kNoSquare,
};

enum Rank : uint8_t {
  kRank1,
  kRank2,
  kRank3,
  kRank4,
  kRank5,
  kRank6,
  kRank7,
  kRank8,
  kRankLast,
  kNoRank,
};

constexpr char rank_char(Rank r) {
  char chars[] = {'1', '2', '3', '4', '5', '6', '7', '8'};
  return chars[static_cast<uint8_t>(r)];
}

constexpr Rank rank_from_char(char c) {
  Rank r = static_cast<Rank>(c - '1');
  if (r > kRank8) {
    return kNoRank;
  }

  return r;
}

enum File {
  kFileA,
  kFileB,
  kFileC,
  kFileD,
  kFileE,
  kFileF,
  kFileG,
  kFileH,
  kFileLast,
  kNoFile
};

constexpr char file_char(File f) {
  char chars[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
  return chars[static_cast<uint8_t>(f)];
}

constexpr File file_from_char(char c) {
  File f = static_cast<File>(c - 'a');
  if (f > kFileH) {
    return kNoFile;
  }

  return f;
}

enum Color : uint8_t { kWhite, kBlack, kColorLast };

constexpr Color operator!(Color other) {
  return static_cast<Color>(1 - static_cast<uint8_t>(other));
}

enum CastlingRights : uint8_t {
  kNoCastle,
  kCastleWhiteKingside = 1 << 0,
  kCastleWhiteQueenside = 1 << 1,
  kCastleBlackKingside = 1 << 2,
  kCastleBlackQueenside = 1 << 3,

  kCastleWhite = kCastleWhiteKingside | kCastleWhiteQueenside,
  kCastleBlack = kCastleBlackKingside | kCastleBlackQueenside,
};

constexpr CastlingRights operator|(CastlingRights left, CastlingRights right) {
  return static_cast<CastlingRights>(static_cast<uint8_t>(left) |
                                     static_cast<uint8_t>(right));
}

constexpr CastlingRights& operator|=(CastlingRights& left,
                                     CastlingRights right) {
  left = left | right;
  return left;
}

constexpr CastlingRights operator&(CastlingRights left, CastlingRights right) {
  return static_cast<CastlingRights>(static_cast<uint8_t>(left) &
                                     static_cast<uint8_t>(right));
}

constexpr CastlingRights& operator&=(CastlingRights& left,
                                     CastlingRights right) {
  left = left & right;
  return left;
}

constexpr CastlingRights operator~(CastlingRights op) {
  return static_cast<CastlingRights>(~static_cast<uint8_t>(op));
}

enum PieceKind : uint8_t {
  kPawn,
  kKnight,
  kBishop,
  kRook,
  kQueen,
  kKing,
  kPieceKindLast
};

enum Piece : uint8_t {
  kNoPiece,
  kWhitePawn,
  kWhiteKnight,
  kWhiteBishop,
  kWhiteRook,
  kWhiteQueen,
  kWhiteKing,
  kBlackPawn,
  kBlackKnight,
  kBlackBishop,
  kBlackRook,
  kBlackQueen,
  kBlackKing,
  kPieceLast,
};

inline Piece piece_from_char(char c) {
  switch (c) {
    case 'P':
      return kWhitePawn;
    case 'p':
      return kBlackPawn;
    case 'N':
      return kWhiteKnight;
    case 'n':
      return kBlackKnight;
    case 'B':
      return kWhiteBishop;
    case 'b':
      return kBlackBishop;
    case 'R':
      return kWhiteRook;
    case 'r':
      return kBlackRook;
    case 'Q':
      return kWhiteQueen;
    case 'q':
      return kBlackQueen;
    case 'K':
      return kWhiteKing;
    case 'k':
      return kBlackKing;
    default:
      return kNoPiece;
  }
}

constexpr char piece_char(Piece p) {
  CHECK(p != kNoPiece) << "kNoPiece in piece_char";
  CHECK(p < kPieceLast) << "invalid piece in piece_char";
  const char chars[] = " PNBRQKpnbrqk";
  return chars[static_cast<uint8_t>(p)];
}

constexpr PieceKind kind_of(Piece piece) {
  return piece >= 7 ? static_cast<PieceKind>(piece - kBlackPawn)
                    : static_cast<PieceKind>(piece - kWhitePawn);
}

constexpr Piece make_piece(PieceKind kind, Color color) {
  uint8_t kind_value = static_cast<uint8_t>(kind);
  uint8_t piece_value =
      color == kWhite ? kind_value + kWhitePawn : kind_value + kBlackPawn;
  return static_cast<Piece>(piece_value);
}

constexpr Color color_of(Piece piece) { return piece >= 7 ? kBlack : kWhite; }

constexpr Rank rank_of(Square sq) {
  return static_cast<Rank>(static_cast<uint8_t>(sq) >> 3);
}

constexpr File file_of(Square sq) {
  return static_cast<File>(static_cast<uint8_t>(sq) & 7);
}

constexpr Square square_of(File file, Rank rank) {
  return static_cast<Square>(static_cast<uint8_t>(rank) * 8 +
                             static_cast<int>(file));
}

/**
 * Returns the square corresponding to this square when mirrored horizontally
 * across the center of the board.
 */
constexpr Square horizontal_flip(Square sq) {
  return square_of(file_of(sq), static_cast<Rank>(static_cast<uint8_t>(kRank8) -
                                                  rank_of(sq)));
}

inline std::string square_string(Square sq) {
  std::string s;
  s.reserve(2);
  s.push_back(file_char(file_of(sq)));
  s.push_back(rank_char(rank_of(sq)));
  return s;
}

enum Direction : int8_t {
  kDirectionNorth = 8,
  kDirectionEast = 1,
  kDirectionSouth = -8,
  kDirectionWest = -1,

  kDirectionNorthEast = kDirectionNorth + kDirectionEast,
  kDirectionNorthWest = kDirectionNorth + kDirectionWest,
  kDirectionSouthEast = kDirectionSouth + kDirectionEast,
  kDirectionSouthWest = kDirectionSouth + kDirectionWest
};

constexpr Direction operator+(Direction lhs, Direction rhs) {
  return static_cast<Direction>(static_cast<int8_t>(lhs) +
                                static_cast<int8_t>(rhs));
}

constexpr Square towards(Square sq, Direction dir) {
  return static_cast<Square>(static_cast<uint8_t>(sq) +
                             static_cast<int8_t>(dir));
}

}  // namespace altair
