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

#include "ttable.h"

#include <atomic>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <thread>

namespace altair::ttable {

std::atomic<RawTableEntry*> kTable;
std::atomic<size_t> kTableSize;

void initialize(uint64_t hashSize) {
  size_t entry_count = (hashSize * 1024 * 1024) / sizeof(RawTableEntry);
  RawTableEntry* table = new RawTableEntry[entry_count];
  kTable.store(table, std::memory_order_relaxed);
  kTableSize.store(entry_count, std::memory_order_relaxed);
}

void destroy() {
  delete[] kTable.load(std::memory_order_relaxed);
  kTable.store(nullptr, std::memory_order_relaxed);
  kTableSize.store(0, std::memory_order_relaxed);
}

void record_pv(const Position& pos, Move best, unsigned depth, Value value) {
  uint64_t key = pos.hash();
  RawTableEntry& entry =
      kTable.load(std::memory_order_relaxed)[key % kTableSize];
  entry.with_lock([&](TableEntry& entry) {
    std::memset(&entry, 0, sizeof(TableEntry));
    entry.zobrist_key = key;
    entry.move = best;
    entry.value = value;
    entry.depth = depth;
    entry.kind = NodeKind::PV;
    return 0;
  });
}

void record_cut(const Position& pos, Move best, unsigned depth, Value value) {
  uint64_t key = pos.hash();
  RawTableEntry& entry =
      kTable.load(std::memory_order_relaxed)[key % kTableSize];
  entry.with_lock([&](TableEntry& entry) {
    entry.zobrist_key = key;
    entry.move = best;
    entry.value = value;
    entry.depth = depth;
    entry.kind = NodeKind::Cut;
    return 0;
  });
}

void record_all(const Position& pos, unsigned depth, Value value) {
  uint64_t key = pos.hash();
  RawTableEntry& entry =
      kTable.load(std::memory_order_relaxed)[key % kTableSize];
  entry.with_lock([&](TableEntry& entry) {
    if (entry.kind == NodeKind::All && entry.depth >= depth) {
      return 0;
    }

    entry.zobrist_key = key;
    entry.move = Move::null();
    entry.depth = depth;
    entry.kind = NodeKind::All;
    return 0;
  });
}

}  // namespace altair::ttable