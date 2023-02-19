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
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <shared_mutex>

namespace altair::ttable {

namespace {

enum class NodeKind { PV, All, Cut };

struct TableEntry {
  std::mutex lock;
  uint64_t zobrist_key;
  Move best;
  unsigned depth;
  NodeKind kind;
  Value value;
};

std::shared_mutex kResizeLock;

std::atomic<TableEntry*> kTable = nullptr;
std::atomic_uint64_t kTableLength = 0;

}  // namespace

void initialize(uint64_t hashSize) {
  std::unique_lock<std::shared_mutex> lock(kResizeLock);

  uint64_t buckets = hashSize / sizeof(TableEntry);
  kTableLength.store(buckets, std::memory_order_relaxed);
  TableEntry* old =
      kTable.exchange(new TableEntry[kTableLength], std::memory_order_relaxed);
  if (old) {
    delete[] old;
  }
}

void query(const Position& pos) {
  std::shared_lock<std::shared_mutex> table_lock(kResizeLock);
}

void record_pv(const Position& pos, Move best, unsigned depth, Value value) {
  std::shared_lock<std::shared_mutex> table_lock(kResizeLock);

  uint64_t key = 0; /* TODO pos.zobrist_hash() */
  TableEntry& entry = kTable.load(
      std::memory_order_relaxed)[key %
                                 kTableLength.load(std::memory_order_relaxed)];

  std::scoped_lock<std::mutex> entry_lock(entry.lock);
  entry.best = best;
  entry.depth = depth;
  entry.kind = NodeKind::PV;
  entry.value = value;
}

void record_cut(const Position& pos, Move best, unsigned depth, Value value) {
  std::shared_lock<std::shared_mutex> table_lock(kResizeLock);

  uint64_t key = 0; /* TODO pos.zobrist_hash() */
  TableEntry& entry = kTable.load(
      std::memory_order_relaxed)[key %
                                 kTableLength.load(std::memory_order_relaxed)];

  std::scoped_lock<std::mutex> entry_lock(entry.lock);
  entry.best = best;
  entry.depth = depth;
  entry.kind = NodeKind::Cut;
  entry.value = value;
}

void record_all(const Position& pos, unsigned depth, Value value) {
  std::shared_lock<std::shared_mutex> table_lock(kResizeLock);

  uint64_t key = 0; /* TODO pos.zobrist_hash() */
  TableEntry& entry = kTable.load(
      std::memory_order_relaxed)[key %
                                 kTableLength.load(std::memory_order_relaxed)];

  std::scoped_lock<std::mutex> entry_lock(entry.lock);
  entry.best = Move::null();
  entry.depth = depth;
  entry.kind = NodeKind::All;
  entry.value = value;
}

}  // namespace altair::ttable