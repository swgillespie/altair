#include "zobrist.h"

#include <array>

namespace altair::zobrist {

namespace {

const uint64_t kZobristHashSeed = 0xf68e34a4e8ccf09a;
const uint64_t kZobristEntryCount = 781;
const uint64_t kZobristSideToMoveEntry = 768;
const uint64_t kZobristCastlingRightsEntry = 769;
const uint64_t kZobristEnPassantEntry = 770;

class XorShift64 {
 public:
  explicit constexpr XorShift64(uint64_t seed) : state_(seed) {}

  constexpr uint64_t next() {
    uint64_t x = state_;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    state_ = x;
    return state_;
  }

 private:
  uint64_t state_;
};

constexpr std::array<uint64_t, kZobristEntryCount> kMagicHashes = []() {
  XorShift64 rng(kZobristHashSeed);
  std::array<uint64_t, kZobristEntryCount> hashes;
  for (int i = 0; i < kZobristEntryCount; i++) {
    hashes[i] = rng.next();
  }

  return hashes;
}();

}  // namespace

void modify_piece(uint64_t *hash, Square square, Piece piece) {
  // The layout of this table is:
  // [square]
  //   0 white pawn hash
  //   1 white knight hash
  //   ...
  //   5 white king hash
  //   6 black pawn hash
  //   7 black knight hash
  //   ...
  //   11 black king hash
  //
  // So, the square base is 12 * square, since the table is laid out one
  // square after another.
  uintptr_t offset = 12 * static_cast<uint8_t>(square);
  uintptr_t color_offset = 6 * static_cast<uint8_t>(color_of(piece));
  uintptr_t piece_offset = static_cast<uint8_t>(kind_of(piece));
  *hash ^= kMagicHashes[offset + color_offset + piece_offset];
}

void modify_side_to_move(uint64_t *hash) {
  *hash ^= kMagicHashes[kZobristSideToMoveEntry];
}

void modify_kingside_castle(uint64_t *hash, Color color) {
  uintptr_t offset = 2 * static_cast<uint8_t>(color);
  *hash ^= kMagicHashes[kZobristCastlingRightsEntry + offset];
}

void modify_queenside_castle(uint64_t *hash, Color color) {
  uintptr_t offset = 2 * static_cast<uint8_t>(color) + 1;
  *hash ^= kMagicHashes[kZobristCastlingRightsEntry + offset];
}

void modify_en_passant(uint64_t *hash, Square oldSquare, Square newSquare) {
  if (oldSquare != kNoSquare) {
    *hash ^= kMagicHashes[kZobristEnPassantEntry +
                          static_cast<uintptr_t>(oldSquare)];
  }

  if (newSquare != kNoSquare) {
    *hash ^= kMagicHashes[kZobristEnPassantEntry +
                          static_cast<uintptr_t>(newSquare)];
  }
}

}  // namespace altair::zobrist