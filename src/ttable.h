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

/**
 * Interface to Altair's transposition table.
 *
 * https://www.chessprogramming.org/Transposition_Table
 */

#include <atomic>
#include <cstdint>

#include "move.h"
#include "position.h"
#include "value.h"

namespace altair {

enum class NodeKind : uint8_t { PV, All, Cut };

struct TableEntry {
  uint64_t zobrist_key;
  Move move;
  Value value;
  uint8_t depth;
  NodeKind kind;
};

#ifdef __cpp_lib_hardware_interference_size
using std::hardware_destructive_interference_size;
#else
constexpr size_t hardware_destructive_interference_size = 64;
#endif

class alignas(hardware_destructive_interference_size) RawTableEntry {
 public:
  RawTableEntry() : lock(0), entry() {}

 private:
  std::atomic<uint8_t> lock;
  mutable TableEntry entry;

 public:
  template <typename F>
  auto with_lock(F func) -> decltype(func(entry)) {
    uint8_t expected = 0;
    while (!lock.compare_exchange_weak(expected, 1, std::memory_order_acquire,
                                       std::memory_order_relaxed)) {
      expected = 0;
    }

    auto result = func(entry);
    lock.store(0, std::memory_order_release);
    return result;
  }
};

static_assert(sizeof(RawTableEntry) == 64,
              "RawTableEntry should occupy one cache line");

namespace ttable {

extern std::atomic<RawTableEntry*> kTable;
extern std::atomic<size_t> kTableSize;

void initialize(uint64_t hashSize);
void destroy();
void record_pv(const Position& pos, Move best, unsigned depth, Value value);
void record_cut(const Position& pos, Move best, unsigned depth, Value value);
void record_all(const Position& pos, unsigned depth, Value value);

template <typename F>
auto query(const Position& pos, F func) -> decltype(func(TableEntry{})) {
  uint64_t key = pos.hash();
  RawTableEntry& entry =
      kTable.load(std::memory_order_relaxed)[key % kTableSize];
  return entry.with_lock([&](TableEntry& entry) { return func(entry); });
}

}  // namespace ttable

}  // namespace altair