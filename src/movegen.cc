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

#include "movegen.h"

#include "attacks.h"

namespace altair::movegen {

namespace {

template <Color Us>
void generate_pawn_moves(const Position& pos, std::vector<Move>& moves) {
  constexpr Color Them = !Us;
  constexpr Bitboard StartRank = Us == kWhite ? kBBRank2 : kBBRank7;
  constexpr Bitboard PromoRank = Us == kWhite ? kBBRank8 : kBBRank1;
  constexpr Direction Up = Us == kWhite ? kDirectionNorth : kDirectionSouth;
  constexpr Direction Down = Us == kWhite ? kDirectionSouth : kDirectionNorth;
  constexpr Bitboard SeventhRank = shift<Down>(PromoRank);
  constexpr Bitboard ThirdRank = shift<Up>(StartRank);

  Bitboard allied_pieces = pos.pieces(Us);
  Bitboard enemy_pieces = pos.pieces(Them);
  Bitboard pieces = allied_pieces | enemy_pieces;
  Bitboard empty_squares = ~pieces;
  Bitboard pawns = pos.pieces(Us, kPawn);

  // Pawns on the seventh rank move with promotion and are handled separately
  // from all other pawns on the board.
  Bitboard pawns_on_seventh = pawns & SeventhRank;
  Bitboard pawns_not_on_seventh = pawns & ~SeventhRank;

  // 1) Non-capture non-promo moves.
  // Pawns move one square up from anywhere on the board, if unimpeded.
  // Pawns on the start rank can move twice, also if unimpeded.
  Bitboard advance = shift<Up>(pawns_not_on_seventh) & empty_squares;
  Bitboard double_advance = shift<Up>(advance & ThirdRank) & empty_squares;
  while (!advance.empty()) {
    Square dest = advance.pop();
    moves.push_back(Move::quiet(towards(dest, Down), dest));
  }
  while (!double_advance.empty()) {
    Square dest = double_advance.pop();
    moves.push_back(Move::double_pawn_push(towards(dest, Down + Down), dest));
  }

  // 2) Capture non-promo moves.
  Bitboard captures_left =
      shift<Up + kDirectionWest>(pawns_not_on_seventh) & enemy_pieces;
  Bitboard captures_right =
      shift<Up + kDirectionEast>(pawns_not_on_seventh) & enemy_pieces;
  while (!captures_left.empty()) {
    Square dest = captures_left.pop();
    moves.push_back(Move::capture(towards(dest, Down + kDirectionEast), dest));
  }
  while (!captures_right.empty()) {
    Square dest = captures_right.pop();
    moves.push_back(Move::capture(towards(dest, Down + kDirectionWest), dest));
  }

  // 3) Promotion moves.
  if (!pawns_on_seventh.empty()) {
    Bitboard promo_advance = shift<Up>(pawns_on_seventh) & empty_squares;
    Bitboard promo_capture_left =
        shift<Up + kDirectionWest>(pawns_on_seventh) & enemy_pieces;
    Bitboard promo_capture_right =
        shift<Up + kDirectionEast>(pawns_on_seventh) & enemy_pieces;

    while (!promo_advance.empty()) {
      Square dest = promo_advance.pop();
      for (PieceKind kind : {kKnight, kBishop, kRook, kQueen}) {
        moves.push_back(Move::promotion(towards(dest, Down), dest, kind));
      }
    }

    while (!promo_capture_left.empty()) {
      Square dest = promo_capture_left.pop();
      for (PieceKind kind : {kKnight, kBishop, kRook, kQueen}) {
        moves.push_back(Move::promotion_capture(
            towards(dest, Down + kDirectionEast), dest, kind));
      }
    }

    while (!promo_capture_right.empty()) {
      Square dest = promo_capture_right.pop();
      for (PieceKind kind : {kKnight, kBishop, kRook, kQueen}) {
        moves.push_back(Move::promotion_capture(
            towards(dest, Down + kDirectionWest), dest, kind));
      }
    }
  }

  // 4) En-passant.
  if (pos.en_passant_square() != kNoSquare) {
    Bitboard attackers = attacks::pawns(pos.en_passant_square(), !Us) & pawns;
    while (!attackers.empty()) {
      Square attacker = attackers.pop();
      moves.push_back(Move::en_passant(attacker, pos.en_passant_square()));
    }
  }
}

template <PieceKind Kind, Color Us>
void generate_moves(const Position& pos, std::vector<Move>& moves) {
  constexpr Color Them = !Us;
  Bitboard moving_pieces = pos.pieces(Us, Kind);
  Bitboard allied_pieces = pos.pieces(Us);
  Bitboard enemy_pieces = pos.pieces(Them);
  Bitboard pieces = allied_pieces | enemy_pieces;

  while (!moving_pieces.empty()) {
    Square piece = moving_pieces.pop();
    Bitboard destinations = attacks::pieces<Kind>(piece, pieces);
    while (!destinations.empty()) {
      Square target = destinations.pop();
      if (enemy_pieces.test(target)) {
        moves.push_back(Move::capture(piece, target));
      } else if (!allied_pieces.test(target)) {
        moves.push_back(Move::quiet(piece, target));
      }
    }

    if constexpr (Kind == kKing) {
      constexpr Piece rook = Us == kWhite ? kWhiteRook : kBlackRook;

      // Here we consider kingside and queenside castles, if the side to move
      // has the right to perform them.
      if (pos.is_check(Us)) {
        // Cannot castle out of check.
        return;
      }

      Square king = pos.pieces(Us, kKing).expect_one();
      if (pos.can_castle_kingside(Us)) {
        constexpr Square starting_rook = Us == kWhite ? H1 : H8;

        if (pos.piece_at(starting_rook) == rook) {
          // Castling moves the king and rook across two squares. Neither of
          // them can be checked.
          Square one = towards(king, kDirectionEast);
          Square two = towards(one, kDirectionEast);
          if (!pieces.test(one) && !pieces.test(two) &&
              pos.squares_attacking(one, !Us).empty() &&
              pos.squares_attacking(two, !Us).empty()) {
            moves.push_back(Move::kingside_castle(king, two));
          }
        }
      }

      if (pos.can_castle_queenside(Us)) {
        constexpr Square starting_rook = Us == kWhite ? A1 : A8;
        if (pos.piece_at(starting_rook) == rook) {
          // Queenside castling moves the king across two squares and the rook
          // one square. The first two squares are where the king moves and must
          // not be checked; the third square is where the rook moves and can be
          // checked, but can't be occupied.
          Square one = towards(king, kDirectionWest);
          Square two = towards(one, kDirectionWest);
          Square three = towards(two, kDirectionWest);
          if (!pieces.test(one) && !pieces.test(two) && !pieces.test(three) &&
              pos.squares_attacking(one, !Us).empty() &&
              pos.squares_attacking(two, !Us).empty()) {
            moves.push_back(Move::queenside_castle(king, two));
          }
        }
      }
    }
  }
}

}  // namespace

void generate_pseudolegal(const Position& pos, std::vector<Move>& moves) {
  if (pos.side_to_move() == kWhite) {
    generate_pawn_moves<kWhite>(pos, moves);
    generate_moves<kKnight, kWhite>(pos, moves);
    generate_moves<kBishop, kWhite>(pos, moves);
    generate_moves<kRook, kWhite>(pos, moves);
    generate_moves<kQueen, kWhite>(pos, moves);
    generate_moves<kKing, kWhite>(pos, moves);
  } else {
    generate_pawn_moves<kBlack>(pos, moves);
    generate_moves<kKnight, kBlack>(pos, moves);
    generate_moves<kBishop, kBlack>(pos, moves);
    generate_moves<kRook, kBlack>(pos, moves);
    generate_moves<kQueen, kBlack>(pos, moves);
    generate_moves<kKing, kBlack>(pos, moves);
  }
}

}  // namespace altair::movegen
