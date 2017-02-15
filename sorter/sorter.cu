#ifdef GLAD_SUPPORT
#include <glad/glad.h>
#else
#include <GL/glew.h>
#endif

#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <thrust/device_ptr.h>
#include <thrust/sort.h>
#include <thrust/device_vector.h>
#include "sorter.h"

void sortByKey(unsigned mortonBuffer, unsigned triangleCount, unsigned indicesBuffer) {
    cudaGraphicsResource *mortonRes;
    cudaGraphicsResource *indicesRes;
    unsigned * devPtr;
    unsigned * idcPtr;
    size_t size;

    cudaGraphicsGLRegisterBuffer(&mortonRes, mortonBuffer, cudaGraphicsMapFlagsNone);
    cudaGraphicsMapResources(1, &mortonRes, 0);
    cudaGraphicsResourceGetMappedPointer((void **)&devPtr, &size, mortonRes);

    cudaGraphicsGLRegisterBuffer(&indicesRes, indicesBuffer, cudaGraphicsMapFlagsNone);
    cudaGraphicsMapResources(1, &indicesRes, 0);
    cudaGraphicsResourceGetMappedPointer((void **)&idcPtr, &size, indicesRes);

    thrust::device_ptr<unsigned> dptr = thrust::device_pointer_cast(devPtr);
    thrust::device_ptr<unsigned> iptr = thrust::device_pointer_cast(idcPtr);
    thrust::sort_by_key(dptr, dptr + triangleCount, iptr);

    cudaGraphicsUnmapResources(1, &mortonRes, 0);
    cudaGraphicsUnregisterResource(mortonRes);

    cudaGraphicsUnmapResources(1, &indicesRes, 0);
    cudaGraphicsUnregisterResource(indicesRes);
}

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