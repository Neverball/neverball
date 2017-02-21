#pragma once

//#define WIN32_LEAN_AND_MEAN 1
//#define WIN32_EXTRA_LEAN
//#include <windows.h>

#include <iostream>

#ifdef GLBINDING_SUPPORT
#include <glbinding/Binding.h>
#include <glbinding/gl45ext/gl.h>
#elif EPOXY_SUPPORT
#include <epoxy/gl.h>
#elif GLAD_SUPPORT
#include <glad/glad.h>
#elif GLEW_SUPPORT
#include <GL/glew.h>
#endif

//#include <GL/gl.h>
//#include <GL/glext.h>

//#pragma comment(lib, "opengl32.lib")

#ifndef ARRAYSIZE
#define IS_INDEXABLE(arg) (sizeof(arg[0]))
#define IS_ARRAY(arg) (IS_INDEXABLE(arg) && (((void *) &arg) == ((void *) arg)))
#define ARRAYSIZE(arr) (IS_ARRAY(arr) ? (sizeof(arr) / sizeof(arr[0])) : 0)
#endif

#define EACH(i, size) for (GLsizeiptr i = 0; i < size; i++)

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#define _STR(x) #x
#define STR(x) _STR(x)

#define GLSL(...) STR(__VA_ARGS__) "\n"
#define GLSL_DEFINE(name, ...) "#define " STR(name) " " GLSL(__VA_ARGS__)

/*
#define GL_FUNCTIONS(FUNCTION)                         \
  FUNCTION(BindBuffer,           BINDBUFFER)           \
  FUNCTION(BindBufferBase,       BINDBUFFERBASE)       \
  FUNCTION(BindBufferRange,      BINDBUFFERRANGE)      \
  FUNCTION(BindProgramPipeline,  BINDPROGRAMPIPELINE)  \
  FUNCTION(GetBufferParameteriv, GETBUFFERPARAMETERIV) \
  FUNCTION(GetBufferParameteri64v, GETBUFFERPARAMETERI64V) \
  FUNCTION(BufferData,           BUFFERDATA)           \
  FUNCTION(BufferSubData,        BUFFERSUBDATA)        \
  FUNCTION(CopyBufferSubData,    COPYBUFFERSUBDATA)    \
  FUNCTION(CreateShaderProgramv, CREATESHADERPROGRAMV) \
  FUNCTION(DebugMessageCallback, DEBUGMESSAGECALLBACK) \
  FUNCTION(DebugMessageInsert,   DEBUGMESSAGEINSERT)   \
  FUNCTION(DispatchCompute,      DISPATCHCOMPUTE)      \
  FUNCTION(GenBuffers,           GENBUFFERS)           \
  FUNCTION(GenProgramPipelines,  GENPROGRAMPIPELINES)  \
  FUNCTION(GetProgramInfoLog,    GETPROGRAMINFOLOG)    \
  FUNCTION(GetProgramiv,         GETPROGRAMIV)         \
  FUNCTION(MapBuffer,            MAPBUFFER)            \
  FUNCTION(MapBufferRange,       MAPBUFFERRANGE)       \
  FUNCTION(MemoryBarrier,        MEMORYBARRIER)        \
  FUNCTION(UnmapBuffer,          UNMAPBUFFER)          \
  FUNCTION(UseProgram,           USEPROGRAM)           \
  FUNCTION(UseProgramStages,     USEPROGRAMSTAGES)
*/

namespace parallel {
namespace rgl {

    using namespace gl;

struct GL {
//#define FUNCTION(name, NAME) \
//PFNGL ## NAME ## PROC name;
//  GL_FUNCTIONS(FUNCTION)
//#undef FUNCTION
  GLint alignment;
  GL & initialize(/*HDC device, GLDEBUGPROC debug_message_callback,*/ bool debug = false);
  void deinitialize();

  bool initialized = false;
  static GL & instance();
};

struct buffer
{
  GLuint id;
  template<GLenum USAGE>
  void allocate(GL const & gl, GLsizeiptr size) {
    glBindBuffer(GL_COPY_WRITE_BUFFER, id);
    glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, USAGE);
  }
  template<GLenum USAGE>
  GLsizeiptr allocate(GL const & gl, GLsizeiptr size, GLsizei count, bool aligned = false) {
    auto aligned_size = aligned
      ? ((size + gl.alignment - 1) / gl.alignment) * gl.alignment
      : size * count;
    glBindBuffer(GL_COPY_WRITE_BUFFER, id);
    glBufferData(GL_COPY_WRITE_BUFFER, aligned_size * count, nullptr, USAGE);
    return aligned_size;
  }
  void free(GL const & gl) {
    glBindBuffer(GL_COPY_WRITE_BUFFER, id);
    glBufferData(GL_COPY_WRITE_BUFFER, 0, nullptr, GL_DYNAMIC_COPY);
  }
  template<typename T>
  void sub_data(GL const & gl, T && data, GLsizeiptr offset) {
    glBindBuffer(GL_COPY_WRITE_BUFFER, id);
    glBufferSubData(GL_COPY_WRITE_BUFFER, offset, sizeof(T), &data);
  }
  template<typename T, size_t N>
  void sub_data(GL const & gl, T (&array)[N], GLsizeiptr alignment) {
    glBindBuffer(GL_COPY_WRITE_BUFFER, id);
    EACH (i, N)
      glBufferSubData(GL_COPY_WRITE_BUFFER, i * alignment, sizeof(T), &array[i]);
  }
  template<GLenum TARGET, GLenum ACCESS, typename T, typename F>
  void map(GL const & gl, F && f) {
    auto buffer_size = size(gl);
    glBindBuffer(TARGET, id);
    auto ptr = reinterpret_cast<T *>(glMapBuffer(TARGET, ACCESS));
    f(gl, ptr, buffer_size / sizeof(T));
    glUnmapBuffer(TARGET);
  }
  template<GLenum TARGET, GLenum ACCESS, typename T, typename F>
  void map(GL const & gl, GLsizeiptr offset, GLsizeiptr count, F && f) {
    glBindBuffer(TARGET, id);
    auto ptr = reinterpret_cast<T *>(glMapBufferRange(TARGET, offset * sizeof(T), count * sizeof(T), ACCESS));
    f(gl, ptr, count);
    glUnmapBuffer(TARGET);
  }
  template<GLenum TARGET>
  void bind(GL const & gl, GLuint index) { glBindBufferBase(TARGET, index, id); }
  template<GLenum TARGET>
  void bind(GL const & gl, GLuint index, GLsizeiptr offset, GLsizeiptr size) {
    glBindBufferRange(TARGET, index, id, offset, size);
  }
  GLsizeiptr size(GL const & gl) {
    if (sizeof(GLsizeiptr) == sizeof(GLint)) {
      GLint size = 0;
      glBindBuffer(GL_COPY_READ_BUFFER, id);
      glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &size);
      return (GLsizeiptr)size;
    } else {
      GLint64 size = 0;
      glBindBuffer(GL_COPY_READ_BUFFER, id);
      glGetBufferParameteri64v(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &size);
      return (GLsizeiptr)size;
    }
  }
  bool is_empty() { return id == 0; }
  static buffer empty() {
    return buffer{ 0 };
  }
  static void factory(GL const & gl, GLsizei count, buffer * buffers) {
    glGenBuffers(count, reinterpret_cast<GLuint *>(buffers));
  }
};

template<GLenum TYPE>
struct program  {
  GLuint id;
};

template<>
struct program<GL_COMPUTE_SHADER> {
  GLuint id;
  void dispatch(GL const & gl, GLuint x = 1, GLuint y = 1, GLuint z = 1) {
    glUseProgram(id);
    glDispatchCompute(x, y, z);
  }
};

template<GLenum TYPE, typename... Sources>
program<TYPE>
make_program(GL const & gl, Sources... sources) {
  GLchar const * array_sources[] = {
    "#version 450\n"
    GLSL(
    precision highp float;
    precision highp int;
    layout(std140, column_major) uniform;
    layout(std430, column_major) buffer;
    ), sources...
  };
  auto id = glCreateShaderProgramv(TYPE, ARRAYSIZE(array_sources), array_sources);
  auto status = GL_TRUE;
  glGetProgramiv(id, GL_LINK_STATUS, &status);
  if (GL_TRUE != status) {
    char info[1024];
    glGetProgramInfoLog(id, sizeof(info), nullptr, info);
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR,
      GL_LINK_STATUS, GL_DEBUG_SEVERITY_HIGH, -1, info);
    std::cerr << info << std::endl;
  }
  return program<TYPE> { id };
}

using vertex_program = program<GL_VERTEX_SHADER>;
using fragment_program = program<GL_FRAGMENT_SHADER>;
using compute_program = program<GL_COMPUTE_SHADER>;

struct pipeline {
  GLuint id;
  pipeline(GL const & gl) : id(make(gl)) {}
  pipeline & use(GL const & gl, program<GL_VERTEX_SHADER> const & program) { glUseProgramStages(id, GL_VERTEX_SHADER_BIT, program.id); return *this; }
  pipeline & use(GL const & gl, program<GL_FRAGMENT_SHADER> const & program) { glUseProgramStages(id, GL_FRAGMENT_SHADER_BIT, program.id); return *this; }
  void rect(GL const & gl) {
    glUseProgram(0);
    glBindProgramPipeline(id);
    glRects(-1, -1, 1, 1);
  }
private:
  static GLuint
  make(GL const & gl) {
    auto id = 0u;
    glGenProgramPipelines(1, &id);
    return id;
  }
};

}
}

#undef EACH
