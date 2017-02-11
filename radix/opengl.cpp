#include "./opengl.hh"
//#include "GL/wglext.h"

namespace parallel {
namespace gl {

static GL gl;

GL & GL::instance() { return gl; }

//static PIXELFORMATDESCRIPTOR pfd = { 0 };
GL & GL::initialize(bool debug) {
    return *this;
}

void GL::deinitialize() {
}

}
}
