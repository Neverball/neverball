#include <GL/glew.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <thrust/device_ptr.h>
#include <thrust/sort.h>
#include <thrust/device_vector.h>
#include "sorter.h"

void sortPair(unsigned mortonBuffer, unsigned triangleCount){
    cudaGraphicsResource *mortonRes;
    uint64_t * devPtr;
    size_t size;
    cudaGraphicsGLRegisterBuffer(&mortonRes, mortonBuffer, cudaGraphicsMapFlagsNone);
    cudaGraphicsMapResources(1, &mortonRes, 0);
    cudaGraphicsResourceGetMappedPointer((void **)&devPtr, &size, mortonRes);
    thrust::device_ptr<uint64_t> dptr = thrust::device_pointer_cast(devPtr);
    thrust::sort(dptr, dptr + triangleCount);
    cudaGraphicsUnmapResources(1, &mortonRes, 0);
    cudaGraphicsUnregisterResource(mortonRes);
}

void sortIndices(unsigned mortonBuffer, unsigned triangleCount) {
    cudaGraphicsResource *mortonRes;
    unsigned * devPtr;
    size_t size;
    cudaGraphicsGLRegisterBuffer(&mortonRes, mortonBuffer, cudaGraphicsMapFlagsNone);
    cudaGraphicsMapResources(1, &mortonRes, 0);
    cudaGraphicsResourceGetMappedPointer((void **)&devPtr, &size, mortonRes);
    thrust::device_ptr<unsigned> dptr = thrust::device_pointer_cast(devPtr);
    thrust::sort(dptr, dptr + triangleCount);
    cudaGraphicsUnmapResources(1, &mortonRes, 0);
    cudaGraphicsUnregisterResource(mortonRes);
}