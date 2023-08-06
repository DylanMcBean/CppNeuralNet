#ifndef CCSH_H
#define CCSH_H

#include <array>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

template <size_t Size = 512>
class CCSH {  // Counter ChaCha Stream Hash
 public:
  static_assert(Size == 64 || Size == 128 || Size == 256 || Size == 512,
                "Size must be 64, 128, 256, or 512.");
  BHF() : state{0}, counter(0), nonce(0), initialized(false) {}

  void start(const std::string& data) {
    start(reinterpret_cast<const uint8_t*>(data.c_str()), data.length());
  }

  void start(const uint8_t* data, size_t length) {
    state = std::array<uint32_t, 16>{0};
    counter = 0;
    nonce = 0;
    initialized = false;
    update(data, length);
  }

  void update(const std::string& data) {
    update(reinterpret_cast<const uint8_t*>(data.c_str()), data.length());
  }

  void update(const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i += 8) {
      std::array<uint32_t, 16> currState{0};
      // Set the Static "expand 32 byte k" value
      currState[0] = 0x65787061;  // expa
      currState[1] = 0x6E642033;  // nd 3
      currState[2] = 0x32206279;  // 2 by
      currState[3] = 0x7465206B;  // te k

      // Set the bytes values
      size_t bytesToCopy = (length - i) < 8 ? (length - i) : 8;
      for (size_t j = 0; j < bytesToCopy; j++) {
        counter += data[i + j];
        currState[4 + j] = data[i + j];
      }

      // Set the counter and nonce
      currState[12] = static_cast<uint32_t>(counter);
      currState[14] = static_cast<uint32_t>(nonce++);

      std::array<uint32_t, 16> outputBlock;
      chacha_block(outputBlock.data(), currState.data());
      if (!initialized) {
        state = outputBlock;
        initialized = true;
      } else {
        for (size_t j = 0; j < 16; j++) {
          state.at(j) ^= outputBlock.at(j);
        }
      }
    }
  }

  std::string hexdump() const {
    std::stringstream ss;
    for (size_t i = 0; i < Size / 32;
         ++i) {  // Size is in bits, so divide by 32 to get the number of
                 // uint32_t elements
      ss << std::hex << std::setw(8) << std::setfill('0') << state[i];
    }
    return ss.str();
  }

 private:
  static void chacha_block(uint32_t out[16], uint32_t const in[16]) {
    int i;
    uint32_t x[16];

    for (i = 0; i < 16; ++i) x[i] = in[i];
    for (i = 0; i < 10; i++) {
      // Odd round
      quarterRound(x[0], x[4], x[8], x[12]);   // column 0
      quarterRound(x[1], x[5], x[9], x[13]);   // column 1
      quarterRound(x[2], x[6], x[10], x[14]);  // column 2
      quarterRound(x[3], x[7], x[11], x[15]);  // column 3
      // Even round
      quarterRound(x[0], x[5], x[10], x[15]);  // diagonal 1
      quarterRound(x[1], x[6], x[11], x[12]);  // diagonal 2
      quarterRound(x[2], x[7], x[8], x[13]);   // diagonal 3
      quarterRound(x[3], x[4], x[9], x[14]);   // diagonal 4
    }
    for (i = 0; i < 16; ++i) out[i] = x[i] + in[i];
  }

  static uint32_t rotateLeft(uint32_t a, uint32_t b) {
    return ((a) << (b)) | ((a) >> (32 - (b)));
  }

  static void quarterRound(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d) {
    a += b; d ^= a; d = rotateLeft(d, 16);
    c += d; b ^= c; b = rotateLeft(b, 12);
    a += b; d ^= a; d = rotateLeft(d, 8);
    c += d; b ^= c; b = rotateLeft(b, 7);
  }

  std::array<uint32_t, 16> state;  // state of the hash
  uint64_t counter;                // increments all the byte values
  uint64_t nonce;    // increments each update loop, not every round
  bool initialized;  // flag to track initialization
};

#endif  // CCSH_H
