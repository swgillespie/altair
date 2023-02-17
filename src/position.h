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

#include <array>
#include <string_view>
#include <vector>

#include "bitboard.h"
#include "move.h"
#include "types.h"

namespace altair {

/**
 * Bits of state that are lost when a move is made and can't be recovered from
 * the move alone.
 */
struct IrreversibleState {
  Square ep_square = kNoSquare;
  CastlingRights castling;
  int halfmove_clock;
  Piece captured_piece = kNoPiece;
};

/**
 * The representation of a board position in Altair.
 */
class Position {
 public:
  Position()
      : pieces_by_square_(),
        boards_by_piece_(),
        boards_by_color_(),
        side_to_move_(kWhite),
        states_(),
        ply_(0) {
    states_.emplace_back();
  }

  /**
   * Sets the position to the given FEN string.
   *
   * @throws std::invalid_argument if the FEN string is invalid.
   */
  void set(std::string_view fen_str);

  /**
   * Adds a piece to the given square on the board.
   */
  void add_piece(Piece piece, Square square);

  /**
   * Removes the piece at the given square on the board and returns it.
   */
  Piece remove_piece(Square square);

  /**
   * Retrieves the piece on the given square on the board, or kNoPiece if the
   * square is empty.
   */
  Piece piece_at(Square square) const;

  /**
   * Applies a move to the given position.
   */
  void make_move(Move mov);

  /**
   * Un-applies a move from the given position and restores the position to its
   * state before the move.
   */
  void unmake_move(Move mov);

  void set_en_passant_square(Square square);
  Square en_passant_square() const;
  void set_side_to_move(Color side);
  Color side_to_move() const;
  void set_castling_rights(CastlingRights rights);
  CastlingRights castling_rights() const;
  void set_halfmove_clock(int clock);
  int halfmove_clock() const;
  void set_ply(int ply);
  int ply() const;

  bool can_castle_kingside(Color side) const;
  bool can_castle_queenside(Color side) const;

  /**
   * Returns a bitboard of squares that are attacking the target square.
   */
  Bitboard squares_attacking(Square target, Color side) const;

  /**
   * Returns whether the given side is in check.
   */
  bool is_check(Color side) const;

  /**
   * Returns a bitboard of all pieces belonging to the given side.
   */
  Bitboard pieces(Color side) const;
  Bitboard pieces(Color side, PieceKind kind) const;

  /**
   * Returns a FEN representation of this position.
   */
  std::string fen() const;

 private:
  /**
   * Board representation.
   */
  std::array<Piece, kSquareLast> pieces_by_square_;
  std::array<Bitboard, 12> boards_by_piece_;
  std::array<Bitboard, 2> boards_by_color_;
  Color side_to_move_;
  std::vector<IrreversibleState> states_;
  int ply_;
};

inline void Position::set_en_passant_square(Square square) {
  states_.back().ep_square = square;
}

inline Square Position::en_passant_square() const {
  return states_.back().ep_square;
}

inline void Position::set_side_to_move(Color side) { side_to_move_ = side; }

inline Color Position::side_to_move() const { return side_to_move_; }

inline void Position::set_castling_rights(CastlingRights rights) {
  states_.back().castling = rights;
}

inline CastlingRights Position::castling_rights() const {
  return states_.back().castling;
}

inline void Position::set_halfmove_clock(int clock) {
  states_.back().halfmove_clock = clock;
}

inline int Position::halfmove_clock() const {
  return states_.back().halfmove_clock;
}

inline void Position::set_ply(int ply) { ply_ = ply; }

inline int Position::ply() const { return ply_; }

inline Bitboard Position::pieces(Color side) const {
  return boards_by_color_[side];
}

inline Bitboard Position::pieces(Color side, PieceKind kind) const {
  Piece piece = make_piece(kind, side);
  return boards_by_piece_[piece - 1];
}

inline bool Position::can_castle_kingside(Color side) const {
  CastlingRights mask =
      side == kWhite ? kCastleWhiteKingside : kCastleBlackKingside;
  return (castling_rights() & mask) == mask;
}

inline bool Position::can_castle_queenside(Color side) const {
  CastlingRights mask =
      side == kWhite ? kCastleWhiteQueenside : kCastleBlackQueenside;
  return (castling_rights() & mask) == mask;
}

}  // namespace altair
