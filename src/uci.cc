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

#include "uci.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "log.h"
#include "search.h"
#include "thread.h"

namespace altair::uci {

static Position pos;

void position(const std::string& buf) {
  std::istringstream is(buf);
  std::string token;
  is >> token >> token;

  std::string fen;
  if (token == "startpos") {
    fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    is >> token;  // potentially "moves"
  } else if (token == "fen") {
    // FEN strings have spaces in them; concat them all together.
    while (is >> token && token != "moves") {
      fen += token + " ";
    }
  }

  pos = Position();
  pos.set(fen);

  // TODO(swgillespie) moves
}

void go(const std::string& buf) {
  std::istringstream is(buf);
  std::string token;

  SearchLimits limits;
  while (is >> token) {
    if (token == "perft") is >> limits.perft;
  }

  Threads::go(pos, limits);
}

void run_one(const std::string& buf);

void bench() {
  run_one("position startpos");
  run_one("go perft 4");
  Threads::wait_until_idle();
  run_one(
      "position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w "
      "KQkq - 0 1");
  run_one("go perft 5");
  Threads::wait_until_idle();
  run_one("quit");
}

void run_one(const std::string& buf) {
  std::istringstream is(buf);
  std::string command;
  is >> std::skipws >> command;
  if (command == "quit") {
    std::exit(0);
  } else if (command == "uci") {
    UCI() << "id name altair 0.1.0";
    UCI() << "id author Sean Gillespie <sean@swgillespie.me>";
    UCI() << "uciok";
  } else if (command == "isready") {
    Threads::wait_until_idle();
    UCI() << "readyok";
  } else if (command == "position") {
    position(buf);
  } else if (command == "go") {
    go(buf);
  } else if (command == "quit") {
    std::exit(0);
  } else if (command == "bench") {
    bench();
  }
}

void run(int argc, char* argv[]) {
  Threads::initialize();

  if (argc == 2 && argv[1] == std::string("bench")) {
    run_one("bench");
  }

  std::string buf;
  while (std::getline(std::cin, buf)) {
    run_one(buf);
  }
}

}  // namespace altair::uci
