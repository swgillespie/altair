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

#include "thread.h"

#include <memory>
#include <mutex>
#include <thread>

#include "search.h"

namespace altair {

Thread::Thread(unsigned id)
    : id_(id), pos_(), idle_(true), stop_(false), idle_cv_(), idle_lock_() {}

void Thread::start() {
  std::lock_guard<std::mutex> lock(idle_lock_);
  idle_.store(false, std::memory_order_relaxed);
  idle_cv_.notify_all();
}

void Thread::stop() { stop_.store(true, std::memory_order_release); }

void Thread::wait_until_idle() {
  if (idle_.load(std::memory_order_relaxed)) {
    return;
  }

  std::unique_lock<std::mutex> lock(idle_lock_);
  idle_cv_.wait(lock, [&]() { return idle_.load(std::memory_order_relaxed); });
}

void Thread::thread_loop() {
  while (true) {
    std::unique_lock<std::mutex> lock(idle_lock_);
    idle_cv_.wait(lock,
                  [&]() { return !idle_.load(std::memory_order_relaxed); });

    Searcher searcher(pos_, limits_);
    searcher.search();
    stop_.store(false, std::memory_order_release);
    idle_.store(true, std::memory_order_relaxed);
    idle_cv_.notify_all();
  }
}

void Thread::set_position(const Position& pos) { pos_ = pos; }
void Thread::set_limits(const SearchLimits& limits) { limits_ = limits; }

void Threads::go(const Position& pos, const SearchLimits& limits) {
  for (auto& thread : threads_) {
    thread->set_position(pos);
    thread->set_limits(limits);
    thread->start();
  }
}

void Threads::stop() {
  for (auto& thread : threads_) {
    thread->stop();
  }
}

void Threads::wait_until_idle() {
  for (auto& thread : threads_) {
    thread->wait_until_idle();
  }
}

void Threads::initialize() {
  std::call_once(init_flag_, []() {
    threads_.emplace_back(std::make_shared<Thread>(0));
    for (auto& thread : threads_) {
      std::thread tr([=]() { thread->thread_loop(); });
      tr.detach();
    }
  });
}

std::once_flag Threads::init_flag_;
std::vector<std::shared_ptr<Thread>> Threads::threads_;

}  // namespace altair
