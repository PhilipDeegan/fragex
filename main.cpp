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
// fragex [regex] [--size N] [--save out.png]
//
// Draws the regex fractal in a window. Each line read from stdin is a new regex, Up/Down
// step through the presets and earlier regexes, Escape quits. With --save the fractal is
// rendered offscreen, checked against the CPU image and written as a PNG.

#include <GL/glew.h>  // before GLFW

#include "mkn/gl/fw.hpp"

#include "fragex.hpp"
#include "fragex/png.hpp"

#include <cstdlib>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace {

char const* const VERTEX = R"(#version 330 core
out vec2 uv;
void main() {
  // one triangle covering the viewport
  vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
  uv = vec2(p.x, 1.0 - p.y);  // texture rows go top to bottom
  gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
)";

char const* const FRAGMENT = R"(#version 330 core
in vec2 uv;
out vec4 colour;
uniform sampler2D fractal;
void main() { colour = vec4(vec3(texture(fractal, uv).r), 1.0); }
)";

GLuint compile(GLenum const type, char const* const source) {
  auto const shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  GLint ok = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    char log[1024];
    glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
    throw std::runtime_error(std::string("shader compile failed: ") + log);
  }
  return shader;
}

GLuint program() {
  auto const vs = compile(GL_VERTEX_SHADER, VERTEX), fs = compile(GL_FRAGMENT_SHADER, FRAGMENT);
  auto const p = glCreateProgram();
  glAttachShader(p, vs);
  glAttachShader(p, fs);
  glLinkProgram(p);
  glDeleteShader(vs);
  glDeleteShader(fs);
  GLint ok = GL_FALSE;
  glGetProgramiv(p, GL_LINK_STATUS, &ok);
  if (!ok) throw std::runtime_error("shader link failed");
  return p;
}

struct Args {
  std::string regex = fragex::PRESETS[0];
  std::uint32_t size = 512;
  std::optional<std::string> save;
};

Args parse(int const argc, char* argv[]) {
  Args args;
  for (int i = 1; i < argc; ++i) {
    std::string_view const a = argv[i];
    if ((a == "--size" || a == "--save") && i + 1 >= argc)
      throw std::invalid_argument(std::string(a) + " needs a value");
    if (a == "--size")
      args.size = static_cast<std::uint32_t>(std::stoul(argv[++i]));
    else if (a == "--save")
      args.save = argv[++i];
    else if (a == "-h" || a == "--help") {
      std::cout << "fragex [regex] [--size N] [--save out.png]\n";
      std::exit(0);
    } else
      args.regex = a;
  }
  return args;
}

// regexes typed on stdin, handed to the render loop
struct Input {
  std::mutex mutex;
  std::vector<std::string> lines;
};

class App {
 public:
  App(fragex::Fractal const& fractal, bool const visible) : _fractal(fractal) {
    mkn::gl::fw::init();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);
    auto const side = static_cast<int>(fractal.side());
    _window = glfwCreateWindow(side, side, "fragex", nullptr, nullptr);
    if (!_window) {
      glfwTerminate();
      throw std::runtime_error("cannot create an OpenGL 3.3 window");
    }
    glfwMakeContextCurrent(_window);
    glfwSwapInterval(1);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) throw std::runtime_error("glewInit failed");
    glGetError();  // glew can leave GL_INVALID_ENUM on core profiles

    _program = program();
    glGenVertexArrays(1, &_vao);
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, side, side, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
  }

  ~App() {
    glDeleteTextures(1, &_texture);
    glDeleteVertexArrays(1, &_vao);
    glDeleteProgram(_program);
    glfwDestroyWindow(_window);
    glfwTerminate();
  }

  // false if the regex doesn't compile, the current one is kept
  bool show(std::string const& regex) {
    try {
      std::regex const re(regex);
      _pixels = _fractal.render(re);
    } catch (std::regex_error const& e) {
      std::cerr << "invalid regex '" << regex << "': " << e.what() << std::endl;
      return false;
    }
    auto const side = static_cast<GLsizei>(_fractal.side());
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, side, side, GL_RED, GL_UNSIGNED_BYTE, _pixels.data());
    glfwSetWindowTitle(_window, ("fragex - " + regex).c_str());
    _regex = regex;
    return true;
  }

  std::vector<std::uint8_t> const& pixels() const { return _pixels; }

  void draw(int const width, int const height) {
    auto const side = (std::min)(width, height);  // square, centred
    glViewport((width - side) / 2, (height - side) / 2, side, side);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(_program);
    glBindVertexArray(_vao);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glDrawArrays(GL_TRIANGLES, 0, 3);
  }

  // renders into a framebuffer of the fractal's size and reads it back, rows top to bottom
  std::vector<std::uint8_t> offscreen() {
    auto const side = static_cast<GLsizei>(_fractal.side());
    GLuint fbo = 0, target = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenTextures(1, &target);
    glBindTexture(GL_TEXTURE_2D, target);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, side, side, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      throw std::runtime_error("offscreen framebuffer incomplete");

    draw(side, side);
    std::vector<std::uint8_t> bottomUp(std::size_t(side) * side), out(bottomUp.size());
    glReadPixels(0, 0, side, side, GL_RED, GL_UNSIGNED_BYTE, bottomUp.data());
    for (GLsizei y = 0; y < side; ++y)
      std::copy_n(bottomUp.begin() + std::ptrdiff_t(side - 1 - y) * side, side,
                  out.begin() + std::ptrdiff_t(y) * side);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteTextures(1, &target);
    glDeleteFramebuffers(1, &fbo);
    return out;
  }

  void run(Input& input) {
    glfwSetWindowUserPointer(_window, this);
    glfwSetKeyCallback(_window, [](GLFWwindow* w, int key, int, int action, int) {
      if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
      auto& app = *static_cast<App*>(glfwGetWindowUserPointer(w));
      if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, GLFW_TRUE);
      if (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) app.step(key == GLFW_KEY_UP ? -1 : 1);
    });

    while (!glfwWindowShouldClose(_window)) {
      std::vector<std::string> lines;
      {
        std::lock_guard<std::mutex> lock(input.mutex);
        lines.swap(input.lines);
      }
      for (auto const& line : lines)
        if (!line.empty() && show(line)) remember(line);

      int width = 0, height = 0;
      glfwGetFramebufferSize(_window, &width, &height);
      draw(width, height);
      glfwSwapBuffers(_window);
      glfwWaitEvents();
    }
  }

  void remember(std::string const& regex) {
    std::erase(_history, regex);
    _history.push_back(regex);
    _at = _history.size() - 1;
  }

 private:
  fragex::Fractal const& _fractal;
  GLFWwindow* _window = nullptr;
  GLuint _program = 0, _vao = 0, _texture = 0;
  std::vector<std::uint8_t> _pixels;
  std::string _regex;
  std::vector<std::string> _history = fragex::PRESETS;
  std::size_t _at = 0;

  void step(int const by) {
    auto const n = static_cast<std::ptrdiff_t>(_history.size());
    _at = static_cast<std::size_t>(((static_cast<std::ptrdiff_t>(_at) + by) % n + n) % n);
    show(_history[_at]);
  }
};

}  // namespace

int main(int argc, char* argv[]) {
  try {
    auto const args = parse(argc, argv);
    fragex::Fractal const fractal(args.size);
    App app(fractal, !args.save);
    if (!app.show(args.regex)) return 1;

    if (args.save) {
      auto const image = app.offscreen();
      if (image != app.pixels()) {
        std::cerr << "OpenGL output differs from the CPU image" << std::endl;
        return 1;
      }
      fragex::png::write(*args.save, image, fractal.side(), fractal.side());
      return 0;
    }

    app.remember(args.regex);
    std::cout << "type a regex and press enter, Up/Down for earlier ones, Escape quits"
              << std::endl;
    Input input;
    std::thread([&input]() {
      for (std::string line; std::getline(std::cin, line);) {
        {
          std::lock_guard<std::mutex> lock(input.mutex);
          input.lines.push_back(line);
        }
        glfwPostEmptyEvent();
      }
    }).detach();  // blocked on stdin at exit
    app.run(input);
    return 0;
  } catch (std::exception const& e) {
    std::cerr << e.what() << std::endl;
    return 2;
  }
}
