/**
Copyright (c) 2022, Philip Deegan.
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
#ifndef _FRAGEX_SCENE_HPP_
#define _FRAGEX_SCENE_HPP_

#include "mkn/kul/log.hpp"
#include "mkn/kul/string.hpp"

#include "irrlichtmt/irrlicht.h"
#include "irrlight.hpp"

#include <regex>

using namespace irr;
using namespace irr::gui;
using namespace irrlight;

class Scene;
class SceneKeyHandler;

class Quadrant {
 public:
  Quadrant(std::uint32_t const x, std::uint32_t const y, std::uint32_t const w,
           std::uint32_t const h, std::string const& i)
      : r(255), g(255), b(255), x(x), y(y), w(w), h(h), i(i) {}

  std::vector<Quadrant> solve() {
    std::vector<Quadrant> pixels;
    if (w == 1 && h == 1)
      pixels.emplace_back(*this);
    else {
      for (auto& p : Quadrant(x, y, w / 2, h / 2, i + "0").solve()) pixels.emplace_back(p);
      for (auto& p : Quadrant(x + w / 2, y, w / 2, h / 2, i + "1").solve()) pixels.emplace_back(p);
      for (auto& p : Quadrant(x, y + h / 2, w / 2, h / 2, i + "2").solve()) pixels.emplace_back(p);
      for (auto& p : Quadrant(x + w / 2, y + h / 2, w / 2, h / 2, i + "3").solve())
        pixels.emplace_back(p);
    }
    return pixels;
  }

  void red(std::uint32_t const r) { this->r = r; }
  void green(std::uint32_t const g) { this->g = g; }
  void blue(std::uint32_t const b) { this->b = b; }
  void rgb(std::uint32_t const r, std::uint32_t const g, std::uint32_t const b) {
    this->r = r;
    this->g = g;
    this->b = b;
  }

 private:
  std::uint32_t r, g, b;
  std::uint32_t const x;
  std::uint32_t const y;
  std::uint32_t const w;
  std::uint32_t const h;
  std::string const i;

  friend class Scene;
  friend class SceneKeyHandler;
};

class Scene : public irrlight::AScene {
 public:
  Scene(irr::IrrlichtDevice* d) : irrlight::AScene(d), t(d) {
    std::uint32_t const w = 512;
    std::uint32_t const h = 512;

    for (auto& p : Quadrant(0, 0, w / 2, h / 2, "0").solve()) pixels.emplace_back(p);
    for (auto& p : Quadrant(w / 2, 0, w / 2, h / 2, "1").solve()) pixels.emplace_back(p);
    for (auto& p : Quadrant(0, h / 2, w / 2, h / 2, "2").solve()) pixels.emplace_back(p);
    for (auto& p : Quadrant(w / 2, h / 2, w / 2, h / 2, "3").solve()) pixels.emplace_back(p);

    t.history().add(irr::core::stringw(".*1.*").c_str());
    t.history().add(irr::core::stringw(".*(12|21|03|30).*").c_str());
    t.history().add(irr::core::stringw("(1[103]|0[12]|2[10]|21)*").c_str());
  }
  ~Scene() {}
  void setUp();
  void tearDown();
  void draw();
  bool keyDown(irr::EKEY_CODE keyCode);
  bool keyUp(irr::EKEY_CODE keyCode);
  bool OnEvent(irr::SEvent const& event);
  auto& terminal() { return t; }

 private:
  std::vector<Quadrant> pixels;
  irrlight::Terminal t;
  std::string reg;
  std::uint32_t lFps = -1;
  friend class SceneKeyHandler;
};

class SceneKeyHandler {
 public:
  static bool keyUp(irr::IrrlichtDevice* device, Scene& scene, irr::EKEY_CODE keyCode) {
    if (keyCode == irr::KEY_OEM_3) {
      !scene.terminal().shown() ? scene.terminal().show() : scene.terminal().hide();
      return true;
    }
    if (scene.terminal().shown()) {
      if (keyCode == irr::KEY_RETURN) {
        try {
          std::regex re(irrlight::TerminalEnterKeyHandler::handle(scene.terminal(), device));
          for (auto& q : scene.pixels) q.rgb(255, 255, 255);

          for (auto& q : scene.pixels)
            if (std::regex_match(q.i, re)) q.rgb(0, 0, 0);

        } catch (std::regex_error const& e) {
          KLOG(ERR) << e.what();
        }
      } else
        return irrlight::TerminalKeyEntryHandler::keyUp(device, scene.terminal(), keyCode);
    }
    return false;
  }
  static bool keyDown(irr::IrrlichtDevice* device, Scene& scene, irr::EKEY_CODE keyCode) {
    if (scene.terminal().shown())
      return irrlight::TerminalKeyEntryHandler::keyDown(device, scene.terminal(), keyCode);

    return false;
  }
};

#endif /* _FRAGEX_SCENE_HPP_ */
