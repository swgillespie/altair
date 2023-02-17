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

#include <cstdlib>
#include <iostream>
#include <mutex>
#include <sstream>

#include "compiler.h"

namespace altair {

class LogMessage {
 public:
  LogMessage() : stream_() {}

  std::ostream& stream() { return stream_; }

  virtual ~LogMessage() {}

 protected:
  std::stringstream stream_;
};

class UciMessage : public LogMessage {
 public:
  virtual ~UciMessage() override {
    static std::mutex outputLock;

    const std::lock_guard<std::mutex> lock(outputLock);
    std::cout << stream_.str() << std::endl;
  }
};

class CheckMessage : public LogMessage {
 public:
  CheckMessage(const char* cond, const char* file, int line)
      : cond_(cond), file_(file), line_(line) {}

  MSVC_WARNING_DISABLE(4722)
  virtual ~CheckMessage() override {
    std::cerr << "[" << file_ << ":" << line_ << "] check \"" << cond_
              << "\" failed: " << stream_.str() << std::endl;
    std::cerr.flush();
    std::abort();
  }

 private:
  const char* cond_;
  const char* file_;
  int line_;
};

class VoidMessage {
 public:
  VoidMessage() {}

  void operator&(std::ostream&) {}
};

}  // namespace altair

#ifdef NDEBUG
#define CHECK_ENABLED() (false)
#else
#define CHECK_ENABLED() (true)
#endif /* NDEBUG */

#define UCI() (::altair::UciMessage().stream())
#define CHECK(expr)               \
  (!CHECK_ENABLED() || (expr))    \
      ? (void)0                   \
      : ::altair::VoidMessage() & \
            ::altair::CheckMessage(#expr, __FILE__, __LINE__).stream()
