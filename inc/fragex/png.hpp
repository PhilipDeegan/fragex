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
#ifndef FRAGEX_PNG_HPP
#define FRAGEX_PNG_HPP

// Minimal PNG writer for black and white images: 1 bit grayscale, stored (uncompressed)
// deflate blocks, no dependencies.

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fragex::png {

inline std::uint32_t crc(std::string const& bytes) {
  static auto const table = []() {
    std::array<std::uint32_t, 256> t{};
    for (std::uint32_t n = 0; n < 256; ++n) {
      auto c = n;
      for (int k = 0; k < 8; ++k) c = c & 1 ? 0xedb88320u ^ (c >> 1) : c >> 1;
      t[n] = c;
    }
    return t;
  }();
  std::uint32_t c = 0xffffffffu;
  for (unsigned char const b : bytes) c = table[(c ^ b) & 0xff] ^ (c >> 8);
  return c ^ 0xffffffffu;
}

inline void u32(std::string& out, std::uint32_t const v) {
  for (int s = 24; s >= 0; s -= 8) out += static_cast<char>((v >> s) & 0xff);
}

inline void chunk(std::string& out, std::string const& type, std::string const& data) {
  u32(out, static_cast<std::uint32_t>(data.size()));
  auto const body = type + data;
  out += body;
  u32(out, crc(body));
}

// zlib stream of stored deflate blocks
inline std::string zlib(std::string const& raw) {
  std::string z = "\x78\x01";
  std::size_t pos = 0;
  do {
    auto const n = (std::min)(raw.size() - pos, std::size_t{65535});
    z += static_cast<char>(pos + n == raw.size());  // BFINAL, BTYPE 00
    z += static_cast<char>(n & 0xff);
    z += static_cast<char>(n >> 8);
    z += static_cast<char>(~n & 0xff);
    z += static_cast<char>((~n >> 8) & 0xff);
    z.append(raw, pos, n);
    pos += n;
  } while (pos < raw.size());
  std::uint32_t a = 1, b = 0;
  for (unsigned char const c : raw) {
    a = (a + c) % 65521;
    b = (b + a) % 65521;
  }
  u32(z, (b << 16) | a);
  return z;
}

// pixels: one byte per pixel, rows top to bottom, 0 is black, anything else white
inline void write(std::string const& path, std::vector<std::uint8_t> const& pixels,
                  std::uint32_t const width, std::uint32_t const height) {
  std::string raw;
  for (std::uint32_t y = 0; y < height; ++y) {
    raw += '\0';  // filter: none
    for (std::uint32_t x = 0; x < width; x += 8) {
      std::uint8_t byte = 0;
      for (std::uint32_t i = 0; i < 8 && x + i < width; ++i)
        if (pixels[std::size_t{y} * width + x + i]) byte |= static_cast<std::uint8_t>(0x80 >> i);
      raw += static_cast<char>(byte);
    }
  }
  std::string ihdr;
  u32(ihdr, width);
  u32(ihdr, height);
  // bit depth 1, grayscale, deflate, no filter, no interlace
  ihdr += std::string("\x01\x00\x00\x00\x00", 5);

  std::string out = "\x89PNG\r\n\x1a\n";
  chunk(out, "IHDR", ihdr);
  chunk(out, "IDAT", zlib(raw));
  chunk(out, "IEND", "");

  std::ofstream f(path, std::ios::binary);
  if (!f.write(out.data(), static_cast<std::streamsize>(out.size())))
    throw std::runtime_error("cannot write " + path);
}

}  // namespace fragex::png

#endif /* FRAGEX_PNG_HPP */
