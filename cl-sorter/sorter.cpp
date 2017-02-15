#ifdef GLAD_SUPPORT
#include <glad/glad.h>
#else
#include <GL/glew.h>
#endif

#ifdef _WIN32
#include "windows.h"
#else
#include <X11/X.h>
#include <X11/Xlib.h>
#endif

#define USE_OPENCL 1
#include <vexcl/vexcl.hpp>
#include <vexcl/vector.hpp>
#include <map> 

#include "sorter.h"

static struct CLAPI {
    cl_context ctx;
    cl_device_id device_id;
    cl::CommandQueue queue;
} clapi;

std::map<GLuint, cl::Buffer> glclbuffers;
bool initialized = false;
bool failed = false;

void init() {
    if (initialized) return;
    initialized = true;

    std::vector<cl::Device> device = vex::backend::device_list(vex::Filter::GLSharing && vex::Filter::GPU);

    cl_context selected_context = 0;
    cl_device_id selected_device = 0;
    for (int i = 0; i < device.size();i++) {
#ifdef _WIN32
        cl_context_properties properties[] = {
            CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
            CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
            CL_CONTEXT_PLATFORM, (cl_context_properties)device[i].getInfo<CL_DEVICE_PLATFORM>(),
            0
        };
#else 
        cl_context_properties properties[] = {
            CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
            CL_WGL_HDC_KHR, (cl_context_properties)glXGetCurrentDisplay(),
            CL_CONTEXT_PLATFORM, (cl_context_properties)device[i].getInfo<CL_DEVICE_PLATFORM>(),
            0
        };
#endif

        cl_int ret = 0;
        cl_device_id device_id = device[i]();
        cl_context ctx = clCreateContext(properties, 1, &device_id, NULL, NULL, &ret);
        if (ret == 0) {
            selected_device = device_id;
            selected_context = ctx;
            break;
        }
    }

    cl_int ret = 0;
    cl_command_queue queue = clCreateCommandQueue(selected_context, selected_device, 0, &ret);
    if (ret != 0) {
        printf("Queue creation => OpenCL error: %d\n", ret);
        failed = true;
        return;
    }

    clapi.queue = cl::CommandQueue(queue);
    clapi.ctx = selected_context;
    clapi.device_id = selected_device;

}

void sortByKey(unsigned mortonBuffer, unsigned triangleCount, unsigned indicesBuffer) {
    init();

    if (failed) return;

    if (glclbuffers.find(mortonBuffer) == glclbuffers.end()) {
        glclbuffers[mortonBuffer] = cl::Buffer(clCreateFromGLBuffer(clapi.ctx, CL_MEM_READ_WRITE, mortonBuffer, 0));
    }

    if (glclbuffers.find(indicesBuffer) == glclbuffers.end()) {
        glclbuffers[indicesBuffer] = cl::Buffer(clCreateFromGLBuffer(clapi.ctx, CL_MEM_READ_WRITE, indicesBuffer, 0));
    }

    cl::Buffer &clMorton = glclbuffers[mortonBuffer];
    cl::Buffer &clIndices = glclbuffers[indicesBuffer];

    vex::vector<cl_uint> vexMorton = vex::vector<cl_uint>(clapi.queue, clMorton);
    vex::vector<cl_uint> vexIndices = vex::vector<cl_uint>(clapi.queue, clIndices);

    glFlush();
    clEnqueueAcquireGLObjects(clapi.queue(), 1, &clMorton(), 0, 0, 0);
    clEnqueueAcquireGLObjects(clapi.queue(), 1, &clIndices(), 0, 0, 0);
    vex::sort_by_key(vexMorton, vexIndices);
    clEnqueueReleaseGLObjects(clapi.queue(), 1, &clMorton(), 0, 0, 0);
    clEnqueueReleaseGLObjects(clapi.queue(), 1, &clIndices(), 0, 0, 0);
    clFinish(clapi.queue());
}

void sortPair(unsigned mortonBuffer, unsigned triangleCount){
}

void sortIndices(unsigned mortonBuffer, unsigned triangleCount) {
}