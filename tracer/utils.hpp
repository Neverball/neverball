#pragma once

#include "includes.hpp"

namespace PathTracer {
    const int kW = 0;
    const int kA = 1;
    const int kS = 2;
    const int kD = 3;
    const int kQ = 4;
    const int kE = 5;
    const int kSpc = 6;
    const int kSft = 7;
    const int kC = 8;
    const int kK = 9;

    class PTObject {};
    class Tracer;
    class Intersector;

    const pgl::uintv _zero = 0;
    const pgl::uintv _one = 1;

    struct CLTracerAPI {
        //RadeonRays::IntersectionApi* api;
#ifdef OPENCL_SUPPORT
        cl_context ctx;
        cl_command_queue queue;
        cl_device_id device_id;
#endif
    };

    static std::string cubefaces[6] = {
        "./cubemap/+x.png",
        "./cubemap/-x.png",
        "./cubemap/+y.png",
        "./cubemap/-y.png",
        "./cubemap/+z.png",
        "./cubemap/-z.png"
    };

    /*
    static pgl::TextureCube initCubeMap(pgl::Context &glcontext)
    {
        std::vector<unsigned char> image;
        unsigned width = 1, height = 1;
        unsigned error = lodepng::decode(image, width, height, cubefaces[0]);

        pgl::TextureCube cubemap = glcontext->createTextureCube();
        cubemap->magFilter(pgl::TextureFilter::Linear);
        cubemap->minFilter(pgl::TextureFilter::Linear);
        cubemap->wrap<0>(pgl::TextureWrap::ClampToEdge);
        cubemap->wrap<1>(pgl::TextureWrap::ClampToEdge);

        pgl::TextureSubImageDescriptor desc;
        pgl::TextureStorageDescriptor stor;

        stor.internalFormat = pgl::InternalFormatConstructor::N8(4);
        stor.size = glm::ivec3(width, height, 1);

        desc.type(pgl::TypeDetector::internal(stor.internalFormat));
        desc.size = stor.size;

        cubemap->storage(stor);
        for (int i = 0;i < 6;i++) {
            lodepng::decode(image, width, height, cubefaces[i]);
            desc.data = image.data();
            desc.offset.z = i;
            cubemap->subImage(desc);
            image.clear();
        }

        return cubemap;
    }
    */

    static pgl::intv tiled(pgl::intv sz, pgl::intv gmaxtile) {
        return (pgl::intv)ceil((double)sz / (double)gmaxtile);
    }

    static double milliseconds() {
        auto duration = std::chrono::high_resolution_clock::now();
        double millis = std::chrono::duration_cast<std::chrono::nanoseconds>(duration.time_since_epoch()).count() / 1000000.0;
        return millis;
    }


    /*
    static pgl::Texture2D loadWithDefault(pgl::Context &glcontext, const std::string& tex, const glm::vec4& def = glm::vec4(0.0f)) {
        pgl::Texture2D texture = glcontext->createTexture2D();
        texture->magFilter(pgl::TextureFilter::Nearest);
        texture->minFilter(pgl::TextureFilter::Nearest);
        texture->wrap<0>(pgl::TextureWrap::Repeat);
        texture->wrap<1>(pgl::TextureWrap::Repeat);

        std::vector<unsigned char> image;
        unsigned width, height;
        unsigned error = lodepng::decode(image, width, height, tex);

        pgl::TextureSubImageDescriptor desc;
        pgl::TextureStorageDescriptor stor;

        if (error) {
            stor.internalFormat = pgl::InternalFormatConstructor::F32(4);
            stor.size = glm::ivec3(1, 1, 1);

            desc.type(pgl::TypeDetector::internal(stor.internalFormat));
            desc.data = (void *)glm::value_ptr(def);
            desc.size = stor.size;
        }
        else {
            stor.internalFormat = pgl::InternalFormatConstructor::N8(4);
            stor.size = glm::ivec3(width, height, 1);

            desc.type(pgl::TypeDetector::internal(stor.internalFormat));
            desc.data = image.data();
            desc.size = stor.size;
        }

        texture->storage(stor);
        texture->subImage(desc);
        return texture;
    }


    static pgl::Texture2D  loadBump(pgl::Context &glcontext, std::string tex) {
        return loadWithDefault(glcontext, tex, glm::vec4(0.5f, 0.5f, 1.0f, 1.0f));
    }

    static pgl::Texture2D loadDiffuse(pgl::Context &glcontext, std::string tex) {
        return loadWithDefault(glcontext, tex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    static pgl::Texture2D loadSpecular(pgl::Context &glcontext, std::string tex) {
        return loadWithDefault(glcontext, tex, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }
    */

    static pgl::floatv frandom() {
        std::ranlux24 eng{ std::random_device{}() };
        std::uniform_real_distribution<pgl::floatv> dist(0.0f, 1.0f);
        return dist(eng);
    }
}