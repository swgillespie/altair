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

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "position.h"
#include "search.h"

namespace altair {

/**
 * A worker thread, to which Altair delegates search work.
 */
class Thread {
 public:
  explicit Thread(unsigned id);

  Thread& operator=(const Thread&) = delete;
  Thread(const Thread&) = delete;

  void start();
  void stop();
  void wait_until_idle();
  void thread_loop();
  void set_position(const Position& pos);
  void set_limits(const SearchLimits& pos);

  unsigned id() const { return id_; }

 private:
  /**
   * The root position, from which all searches will be performed.
   */
  unsigned id_;
  Position pos_;
  SearchLimits limits_;
  std::atomic_bool idle_;
  std::atomic_bool stop_;
  std::condition_variable idle_cv_;
  std::mutex idle_lock_;
};

/**
 * UCI-facing interface for thread management; encapsulates all creation,
 * destruction, suspension, and management of threads of execution.
 */
class Threads {
 public:
  /**
   * Begin thinking and return immediately. Launched by the UCI "go" command.
   */
  static void go(const Position& pos, const SearchLimits& limits);

  /**
   * Stop thinking and return immediately.
   */
  static void stop();

  /**
   * Block until all worker threads are idle.
   */
  static void wait_until_idle();

  /**
   * Initialize the global thread pool.
   */
  static void initialize();

 private:
  static std::once_flag init_flag_;
  static std::vector<std::shared_ptr<Thread>> threads_;
};

};  // namespace altair