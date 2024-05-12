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

#include "value.h"

#include <limits>
#include <sstream>

namespace altair {

namespace {

const int16_t kValueMated = std::numeric_limits<int16_t>::min() / 2 + 1;
const int16_t kValueMate = std::numeric_limits<int16_t>::max() / 2;
const int16_t kMateDistanceMax = 50;

}  // namespace

Value Value::mate_in(unsigned ply) {
  CHECK(ply < kMateDistanceMax) << "ply too deep for mate";
  return Value(kValueMate + kMateDistanceMax - ply);
}

Value Value::mated_in(unsigned ply) {
  CHECK(ply < kMateDistanceMax) << "ply too deep for mated";
  return Value(kValueMated - kMateDistanceMax + ply);
}

Value Value::operator+(const Value& other) const {
  CHECK(centipawns_ > kValueMated && centipawns_ < kValueMate);
  int16_t next = centipawns_ + other.centipawns_;
  if (next <= kValueMated || next >= kValueMate) [[unlikely]] {
    if (next <= kValueMated) {
      next = kValueMated + 1;
    } else {
      next = kValueMate - 1;
    }
  }

  return Value(next);
}

Value Value::operator-(const Value& other) const { return *this + -other; }

Value Value::operator-() const { return Value(-centipawns_); }

Value& Value::operator+=(const Value& other) {
  *this = *this + other;
  return *this;
}

std::string Value::as_uci() const {
  std::ostringstream ss;

  if (centipawns_ > kValueMate) {
    int16_t moves = kValueMate + kMateDistanceMax - centipawns_;
    ss << "mate " << moves;
  } else if (centipawns_ < kValueMated) {
    int16_t moves = centipawns_ - kValueMated + kMateDistanceMax;
    ss << "mate -" << moves;
  } else {
    ss << "cp " << centipawns_;
  }

  return ss.str();
}

}  // namespace altair