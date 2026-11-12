/**
Copyright (c) 2026, Philip Deegan.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
    * Neither the name of Philip Deegan nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef FRAGEX_HPP
#define FRAGEX_HPP

// Regex fractals: a square of side 2^depth is split into quadrants, numbered
//   0 1
//   2 3
// recursively down to single pixels, so each pixel has an address of `depth` digits, most
// significant quadrant first. Pixels whose address matches a regex are drawn black.

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <regex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace fragex {

inline std::vector<std::string> const PRESETS{
    ".*1.*",
    ".*(12|21|03|30).*",
    "(1[103]|0[12]|2[10]|21)*",  // even length addresses only, e.g. --size 1024
    ".*(.)\\1.*",
    ".*(00|33).*",
    ".*0.*3.*",
};

// address of pixel (x, y), y downwards, in a square of side 2^depth
inline std::string address(std::uint32_t const x, std::uint32_t const y,
                           std::uint32_t const depth) {
  std::string a(depth, '0');
  for (std::uint32_t i = 0; i < depth; ++i) {
    auto const level = depth - 1 - i;
    a[i] = static_cast<char>('0' + ((x >> level) & 1) + 2 * ((y >> level) & 1));
  }
  return a;
}

class Fractal {
 public:
  // side must be a power of two
  explicit Fractal(std::uint32_t const side) : _side(side) {
    if (side < 2 || !std::has_single_bit(side))
      throw std::invalid_argument("fractal size must be a power of two, >= 2");
    _depth = static_cast<std::uint32_t>(std::countr_zero(side));
    _addresses.reserve(std::size_t{side} * side);
    for (std::uint32_t y = 0; y < side; ++y)
      for (std::uint32_t x = 0; x < side; ++x) _addresses.emplace_back(address(x, y, _depth));
  }

  std::uint32_t side() const { return _side; }
  std::uint32_t depth() const { return _depth; }

  // one byte per pixel, rows top to bottom: 0 where the address matches, else 255
  std::vector<std::uint8_t> render(std::regex const& re) const {
    std::vector<std::uint8_t> pixels(_addresses.size());
    auto const threads = std::clamp<std::size_t>(std::thread::hardware_concurrency(), 1, 64);
    auto const chunk = (pixels.size() + threads - 1) / threads;
    std::vector<std::jthread> workers;
    for (std::size_t t = 0; t < threads; ++t)
      workers.emplace_back([&, t]() {
        auto const end = (std::min)(pixels.size(), (t + 1) * chunk);
        for (auto i = t * chunk; i < end; ++i)
          pixels[i] = std::regex_match(_addresses[i], re) ? 0 : 255;
      });
    return pixels;
  }

 private:
  std::uint32_t _side, _depth;
  std::vector<std::string> _addresses;
};

}  // namespace fragex

#endif /* FRAGEX_HPP */
