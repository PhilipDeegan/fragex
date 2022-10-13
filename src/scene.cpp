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
#include "scene.hpp"

void Scene::setUp() {}

void Scene::tearDown() {}

bool Scene::keyDown(irr::EKEY_CODE keyCode) {
  return SceneKeyHandler::keyDown(device(), *this, keyCode);
}
bool Scene::keyUp(irr::EKEY_CODE keyCode) {
  return SceneKeyHandler::keyUp(device(), *this, keyCode);
}
bool Scene::OnEvent(SEvent const& event) {
  if (event.EventType == EET_MOUSE_INPUT_EVENT &&
      event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN) {
  }
  return false;
}

void Scene::draw() {
  if (device()->isWindowActive()) {
    irr::video::IVideoDriver* driver = device()->getVideoDriver();
    driver->beginScene(true, true, video::SColor(0, 255, 255, 255));

    uint fps = driver->getFPS();
    if (lFps != fps) {
      core::stringw c(L"Regex Fractals in irrlicht - [");
      c += driver->getName();
      c += L"] FPS: ";
      c += fps;
      device()->setWindowCaption(c.c_str());
    }
    if (terminal().shown()) {
      terminal().draw();
      device()->getGUIEnvironment()->drawAll();
    } else {
      for (Quadrant const& q : pixels)
        driver->drawPixel(q.x, q.y, irr::video::SColor(255, q.r, q.g, q.b));
    }
    driver->endScene();
  } else {
    device()->yield();
  }
}
