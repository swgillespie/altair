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

#ifdef _MSC_VER
#include <intrin.h>
#endif /* _MSC_VER */

/**
 * Counts the number of 1 bits set in the given 64-bit unsigned integer.
 * uint64_t popcount64(uint64_t value)
 */
#if defined(_MSC_VER)
#define popcount64(value) __popcnt64((value))
#elif defined(__GNUC__) || defined(__clang__)
#define popcount64(value) __builtin_popcountll((value))
#else
#error "popcount64 not implemented for this compiler
#endif

/**
 * Counts the number of trailing zeroes before the first 1 bit is found,
 * starting with the least significant bit.
 * uint64_t count_trailing_zeroes(uint64_t value)
 */
#if defined(_MSC_VER)
#define count_trailing_zeroes(value) _tzcnt_u64((value))
#elif defined(__GNUC__) || defined(__clang__)
#define count_trailing_zeroes(value) __builtin_ctzll((value))
#else
#error "count_trailing_zeroes not implemented for this compiler"
#endif

/**
 * Hint to the compiler that this line of code is unreachable.
 * [[noreturn]] void unreachable()
 */
#if defined(_MSC_VER)
#define unreachable() __assume(false)
#elif defined(__GNUC__) || defined(__clang__)
#define unreachable() __builtin_unreachable()
#else
#error "unreachable not implemented for this compiler"
#endif
