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
#include "fragex.hpp"

#include <iostream>

namespace {

int failures = 0;

void check(bool const ok, std::string const& what) {
  std::cout << (ok ? "PASS " : "FAIL ") << what << std::endl;
  failures += !ok;
}

std::size_t black(std::vector<std::uint8_t> const& pixels) {
  return static_cast<std::size_t>(std::count(pixels.begin(), pixels.end(), 0));
}

}  // namespace

int main() {
  using fragex::address;
  check(address(0, 0, 9) == "000000000", "top left");
  check(address(511, 0, 9) == "111111111", "top right");
  check(address(0, 511, 9) == "222222222", "bottom left");
  check(address(511, 511, 9) == "333333333", "bottom right");
  check(address(256, 0, 9) == "100000000", "most significant digit first");
  check(address(1, 1, 9) == "000000003", "least significant digit last");

  fragex::Fractal const f(512);
  // addresses without a 1 are 3^depth
  check(black(f.render(std::regex(".*1.*"))) == 512 * 512 - 19683, ".*1.* count");
  auto const zeros = f.render(std::regex("0*"));
  check(black(zeros) == 1 && zeros[0] == 0, "0* is the top left pixel");
  check(black(f.render(std::regex("[0-3]*"))) == 512 * 512, "everything");

  bool threw = false;
  try {
    fragex::Fractal(500);
  } catch (std::invalid_argument const&) {
    threw = true;
  }
  check(threw, "size must be a power of two");

  return failures;
}
