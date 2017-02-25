#pragma once

#include "./opengl.hh"

namespace parallel {
namespace rgl {

void radix_sort(GL & gl, buffer key, GLsizeiptr size = 0, buffer index = buffer::empty(),
  bool descending = false, bool is_signed = false, bool is_float = false);

}
}

