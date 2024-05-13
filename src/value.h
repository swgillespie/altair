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

#include "log.h"

namespace altair {

class Value {
 public:
  constexpr Value(int16_t centipawns) : centipawns_(centipawns) {}

  constexpr Value() : Value(0) {}

  static Value mated_in(unsigned ply);
  static Value mate_in(unsigned ply);

  Value operator+(const Value& other) const;
  Value operator-(const Value& other) const;
  Value operator-() const;
  Value& operator+=(const Value& other);
  Value& operator-=(const Value& other);
  bool operator==(const Value& other) const;

  std::string as_uci() const;

 private:
  int16_t centipawns_;
};

}  // namespace altair
