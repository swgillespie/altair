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

#include "position.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

#include "attacks.h"
#include "zobrist.h"

namespace altair {

namespace {

/**
 * A parser for FEN.
 */
class FenParser {
 public:
  explicit FenParser(std::string_view fen)
      : it_(fen.cbegin()), end_(fen.cend()) {}

  void parse(Position& pos) {
    for (int rank = kRank8; rank >= kRank1; rank--) {
      int file = kFileA;
      while (file <= kFileH) {
        char c = *it_;
        if (std::isdigit(c)) {
          if (c < '1' || c > '8') {
            throw std::invalid_argument("invalid digit in FEN string");
          }

          file += static_cast<int>(c - 48);
          if (file > kFileLast) {
            throw std::invalid_argument("file sum does not sum to 8");
          }

          it_++;
          continue;
        }

        Piece p = piece_from_char(c);
        if (p == kNoPiece) {
          throw std::invalid_argument("unknown piece character");
        }

        Square sq = square_of(static_cast<File>(file), static_cast<Rank>(rank));
        pos.add_piece(p, sq);
        it_++;
        file++;
      }

      if (rank != kRank1) {
        eat('/');
      }
    }

    eat(' ');
    // Side to move
    switch (*it_++) {
      case 'w':
        pos.set_side_to_move(kWhite);
        break;
      case 'b':
        pos.set_side_to_move(kBlack);
        break;
      default:
        throw std::invalid_argument("unknown side-to-move character");
    }
    eat(' ');

    // Castle status
    CastlingRights rights = kNoCastle;
    if (peek() == '-') {
      it_++;
    } else {
      bool seen_space = false;
      for (int i = 0; i < 4 && !seen_space; i++) {
        switch (peek()) {
          case 'K':
            rights |= kCastleWhiteKingside;
            break;
          case 'k':
            rights |= kCastleBlackKingside;
            break;
          case 'Q':
            rights |= kCastleWhiteQueenside;
            break;
          case 'q':
            rights |= kCastleBlackQueenside;
            break;
          case ' ':
            seen_space = true;
            break;
          default:
            throw std::invalid_argument("unknown castling character");
        }

        if (!seen_space) {
          it_++;
        }
      }
    }

    pos.set_castling_rights(rights);
    eat(' ');

    // En passant
    if (peek() == '-') {
      it_++;
    } else {
      File f = file_from_char(peek());
      if (f == kNoFile) {
        throw std::invalid_argument("invalid ep file");
      }

      it_++;
      Rank r = rank_from_char(peek());
      if (r == kNoRank) {
        throw std::invalid_argument("invalid ep rank");
      }
      it_++;
      pos.set_en_passant_square(square_of(f, r));
    }

    if (!peek_eof()) {
      return;
    }

    eat(' ');
    // Halfmove clock
    std::stringstream halfmove_stream;
    while (true) {
      char c = peek();
      if (!std::isdigit(c)) {
        break;
      }

      halfmove_stream << c;
      it_++;
    }

    pos.set_halfmove_clock(std::stoi(halfmove_stream.str()));
    eat(' ');

    // Fullmove clock
    std::stringstream fullmove_stream;
    while (true) {
      auto c = peek_eof();
      if (!c || !std::isdigit(*c)) {
        break;
      }

      fullmove_stream << *c;
      it_++;
    }

    int fullmove = std::stoi(fullmove_stream.str());
    int ply = std::max(2 * (fullmove - 1), 0) +
              (pos.side_to_move() == kBlack ? 1 : 0);
    pos.set_ply(ply);
  }

  void eat(char c) {
    if (peek() != c) {
      throw std::invalid_argument("unexpected character");
    }

    it_++;
  }

  char peek() {
    auto next = peek_eof();
    if (!next.has_value()) {
      throw std::invalid_argument("unexpected EOF while reading FEN string");
    }

    return *next;
  }

  std::optional<char> peek_eof() {
    if (it_ == end_) {
      return {};
    }

    return *it_;
  }

 private:
  std::string_view::const_iterator it_;
  std::string_view::const_iterator end_;
};

}  // anonymous namespace

void Position::set(std::string_view fen_str) {
  FenParser parser(fen_str);
  parser.parse(*this);
}

void Position::add_piece(Piece piece, Square square) {
  CHECK(piece_at(square) == kNoPiece) << "adding piece to non-empty square";
  pieces_by_square_[square] = piece;
  boards_by_piece_[piece - 1].set(square);
  boards_by_color_[color_of(piece)].set(square);
  zobrist::modify_piece(&hash_, square, piece);
}

Piece Position::remove_piece(Square square) {
  CHECK(piece_at(square) != kNoPiece) << "removing piece from empty square";
  Piece p = pieces_by_square_[square];
  pieces_by_square_[square] = kNoPiece;
  boards_by_piece_[p - 1].unset(square);
  boards_by_color_[color_of(p)].unset(square);
  zobrist::modify_piece(&hash_, square, p);
  return p;
}

Piece Position::piece_at(Square square) const {
  return pieces_by_square_[square];
}

std::string Position::fen() const {
  std::stringstream ss;
  for (int rank = kRank8; rank >= kRank1; rank--) {
    int empty_squares = 0;
    for (int file = kFileA; file < kFileLast; file++) {
      Square sq = square_of(static_cast<File>(file), static_cast<Rank>(rank));
      Piece p = piece_at(sq);
      if (p != kNoPiece) {
        if (empty_squares != 0) {
          ss << empty_squares;
        }
        ss << piece_char(p);
        empty_squares = 0;
      } else {
        empty_squares++;
      }
    }

    if (empty_squares != 0) {
      ss << empty_squares;
    }

    if (rank != kRank1) {
      ss << '/';
    }
  }

  ss << ' ';
  if (side_to_move_ == kWhite) {
    ss << 'w';
  } else {
    ss << 'b';
  }
  ss << ' ';

  CastlingRights rights = castling_rights();
  if (rights == kNoCastle) {
    ss << '-';
  } else {
    if ((rights & kCastleWhiteKingside) == kCastleWhiteKingside) {
      ss << 'K';
    }
    if ((rights & kCastleWhiteQueenside) == kCastleWhiteQueenside) {
      ss << 'Q';
    }
    if ((rights & kCastleBlackKingside) == kCastleBlackKingside) {
      ss << 'k';
    }
    if ((rights & kCastleBlackQueenside) == kCastleBlackQueenside) {
      ss << 'q';
    }
  }

  if (en_passant_square() != kNoSquare) {
    ss << ' ' << square_string(en_passant_square()) << ' ';
  } else {
    ss << ' ' << '-' << ' ';
  }
  ss << halfmove_clock() << ' '
     << 1 + (ply_ - (1 ? side_to_move_ == kBlack : 0)) / 2;
  return ss.str();
}

void Position::make_move(Move mov) {
  Color us = side_to_move_;
  Square to = mov.destination();
  Square from = mov.source();
  Piece p = remove_piece(from);
  CHECK(p != kNoPiece) << "no piece at source square";
  CHECK(color_of(p) == us) << "moving piece that does not belong to us";

  states_.emplace_back();
  const IrreversibleState& old_state = states_[states_.size() - 2];
  IrreversibleState& new_state = states_.back();
  if (mov.is_capture()) {
    Square target_square = to;
    if (mov.is_en_passant()) {
      // En passant is always the annoying exception; here it is the only move
      // where the piece being captured does not lie on the move destination
      // square.
      Direction down = us == kWhite ? kDirectionSouth : kDirectionNorth;
      target_square = towards(to, down);
    }

    Piece capture = remove_piece(target_square);
    CHECK(capture != kNoPiece) << "no piece at capture square";
    CHECK(kind_of(capture) != kKing) << "illegal king capture";
    CHECK(color_of(capture) == !us)
        << "captured piece does not belong to opponent";
    new_state.captured_piece = capture;
  }

  if (mov.is_castle()) {
    // Castles are encoded as king moves to the destination squares, so moving
    // the king to the correct location will be handled later. Here, though, we
    // must ensure that the rook ends up in the right place post-castle.
    Square kingside_rook = us == kWhite ? H1 : H8;
    Square queenside_rook = us == kWhite ? A1 : A8;
    Square rook_destination = mov.is_kingside_castle()
                                  ? towards(to, kDirectionWest)
                                  : towards(to, kDirectionEast);
    Square moving_rook =
        mov.is_kingside_castle() ? kingside_rook : queenside_rook;
    Piece rook = remove_piece(moving_rook);
    CHECK(kind_of(rook) == kRook && color_of(rook) == us) << "invalid castle";
    add_piece(rook, rook_destination);
  }

  if (mov.is_promotion()) {
    // Promotions add the new piece to the board at the target square and not a
    // pawn.
    p = make_piece(mov.promotion_piece(), us);
  }

  add_piece(p, to);
  ply_++;
  new_state.castling = old_state.castling;
  new_state.halfmove_clock = old_state.halfmove_clock + 1;
  if (kind_of(p) == kPawn || mov.is_capture() || mov.is_promotion()) {
    new_state.halfmove_clock = 0;
  }

  if (kind_of(p) == kKing) {
    // King moves invalidate all castling rights.
    CastlingRights mask = us == kWhite ? kCastleWhite : kCastleBlack;
    new_state.castling &= ~mask;
    zobrist::modify_kingside_castle(&hash_, us);
    zobrist::modify_queenside_castle(&hash_, us);
  } else if (kind_of(p) == kRook) {
    // Rook moves invalidate castling rights on the side of the board that the
    // rook originated.
    Square kingside_rook = us == kWhite ? H1 : H8;
    Square queenside_rook = us == kWhite ? A1 : A8;
    if (can_castle_kingside(us) && mov.source() == kingside_rook) {
      CastlingRights mask =
          us == kWhite ? kCastleWhiteKingside : kCastleBlackKingside;
      new_state.castling &= ~mask;
      zobrist::modify_kingside_castle(&hash_, us);
    } else if (can_castle_queenside(us) && mov.source() == queenside_rook) {
      CastlingRights mask =
          us == kWhite ? kCastleWhiteQueenside : kCastleBlackQueenside;
      new_state.castling &= ~mask;
      zobrist::modify_queenside_castle(&hash_, us);
    }
  }

  side_to_move_ = !side_to_move_;
  zobrist::modify_side_to_move(&hash_);
  if (mov.is_double_pawn_push()) {
    Direction down = us == kWhite ? kDirectionSouth : kDirectionNorth;
    new_state.ep_square = towards(mov.destination(), down);
  }
}

void Position::unmake_move(Move mov) {
  IrreversibleState state = states_.back();
  states_.pop_back();
  ply_--;
  Color us = !side_to_move();
  Square to = mov.destination();
  Square from = mov.source();
  Piece p = remove_piece(to);
  if (mov.is_promotion()) {
    // For promotions, we must put a pawn on the originating square.
    p = make_piece(kPawn, us);
  }
  add_piece(p, from);
  if (mov.is_capture()) {
    Square target_square = to;
    if (mov.is_en_passant()) {
      // Like in make_move, en passant is the only case in which we need to put
      // the captured piece on a differnt square than the destination of the
      // move.
      Direction up = us == kWhite ? kDirectionSouth : kDirectionNorth;
      target_square = towards(to, up);
    }

    add_piece(state.captured_piece, target_square);
  }

  if (mov.is_castle()) {
    // As in make_move, castle moves are encoded as king movement, so we only
    // have to take care of getting the rook to the right spot here.
    Square kingside_rook = us == kWhite ? H1 : H8;
    Square queenside_rook = us == kWhite ? A1 : A8;
    Square rook_location = mov.is_kingside_castle()
                               ? towards(to, kDirectionWest)
                               : towards(to, kDirectionEast);
    Square rook_destination =
        mov.is_kingside_castle() ? kingside_rook : queenside_rook;
    Piece rook = remove_piece(rook_location);
    CHECK(kind_of(rook) == kRook && color_of(rook) == us) << "invalid castle";
    add_piece(rook, rook_destination);
  }

  side_to_move_ = !side_to_move_;
}

Bitboard Position::squares_attacking(Square target, Color side) const {
  Bitboard occupancy = pieces(side) | pieces(!side);
  Bitboard pawns = pieces(side, kPawn);
  Bitboard knights = pieces(side, kKnight);
  Bitboard bishops = pieces(side, kBishop);
  Bitboard rooks = pieces(side, kRook);
  Bitboard queens = pieces(side, kQueen);
  Bitboard king = pieces(side, kKing);

  Bitboard attackers;
  attackers |= attacks::pawns(target, !side) & pawns;
  attackers |= attacks::knights(target) & knights;
  attackers |= attacks::bishops(target, occupancy) & bishops;
  attackers |= attacks::rooks(target, occupancy) & rooks;
  attackers |= attacks::queens(target, occupancy) & queens;
  attackers |= attacks::kings(target) & king;
  return attackers;
}

bool Position::is_check(Color side) const {
  Square king = pieces(side, kKing).expect_one();
  return !squares_attacking(king, !side).empty();
}

}  // namespace altair