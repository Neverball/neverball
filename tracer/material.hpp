#pragma once

#include "includes.hpp"
#include "utils.hpp"
#include <map>

namespace PathTracer {
    class Material : public PTObject {
        friend class Tracer;

    public:

        struct Submat {
            pgl::floatv4 diffuse = pgl::floatv4(0.0f);
            pgl::floatv4 specular = pgl::floatv4(0.0f);
            pgl::floatv4 transmission = pgl::floatv4(0.0f);
            pgl::floatv4 emissive = pgl::floatv4(0.0f);
            pgl::floatv ior = 1.0f;
            pgl::floatv reflectivity = 0.0001f;
            pgl::floatv s1;
            pgl::floatv s2;

            pgl::uint64v diffusePart = 0xFFFFFFFFFFFFFFFF;
            pgl::uint64v specularPart = 0xFFFFFFFFFFFFFFFF;
            pgl::uint64v bumpPart = 0xFFFFFFFFFFFFFFFF;
            pgl::uint64v emissivePart = 0xFFFFFFFFFFFFFFFF;

            pgl::intv flags = 0;
            pgl::intv nkvd0 = 0;
            pgl::intv nkvd1 = 0;
            pgl::intv nkvd2 = 0;
        };

    private:
        CLTracerAPI clapi;

        pgl::Context context;
        pgl::intv materialID = 0;
        pgl::Buffer<Submat> mats;

        void init() {

        }
    public:


        std::map<std::string, pgl::int64v> texnames;
        std::vector<Submat> submats;
        std::vector<pgl::Texture2D> textures;
        pgl::uintv toffset = 0;


        Material(pgl::Context ctx, CLTracerAPI api) {
            context = ctx;
            clapi = api;
            init();

            mats = context->createBuffer<Submat>();
        }

        void loadToVGA() {
            mats->data(submats);
            context->flush();
        }

        void bindWithContext() {
            context->binding(0)->target(pgl::BufferTarget::ShaderStorage)->buffer(mats);
        }

        pgl::uint64v loadTextureUnstricted(pgl::Texture2D texture) {
            pgl::uint64v idx = texture->handleTexture();
            context->makeResidentTexture(idx);
            return idx;
        }

        pgl::uint64v loadTexture(std::string tex, pgl::Texture2D texture) {
            if (tex == "") {
                pgl::uint64v idx = texture->handleTexture();
                context->makeResidentTexture(idx);
                return idx;
            }
            if (texnames.find(tex) == texnames.end()) {
                pgl::uint64v idx = texture->handleTexture();
                context->makeResidentTexture(idx);
                texnames[tex] = idx;//(textures.size() - 1);
            }
            return texnames[tex];
        }

        /*
        pgl::uint64v loadTexture(std::string tex) {
            if (tex == "") return 0xFFFFFFFFFFFFFFFF;
            if (texnames.find(tex) == texnames.end()) {
                std::vector<unsigned char> image;
                pgl::uintv width, height;
                pgl::uintv error = lodepng::decode(image, width, height, tex);
                if (!error) {
                    pgl::TextureSubImageDescriptor desc;
                    pgl::TextureStorageDescriptor stor;

                    stor.internalFormat = pgl::InternalFormatConstructor::N8(4);
                    stor.size = glm::ivec3(width, height, 1);

                    desc.type(pgl::TypeDetector::internal(stor.internalFormat));
                    desc.data = image.data();
                    desc.size = stor.size;

                    pgl::Texture2D texture = context->createTexture2D();
                    texture->magFilter(pgl::TextureFilter::Nearest);
                    texture->minFilter(pgl::TextureFilter::Nearest);
                    texture->wrap<0>(pgl::TextureWrap::Repeat);
                    texture->wrap<1>(pgl::TextureWrap::Repeat);
                    texture->storage(stor);
                    texture->subImage(desc);
                    textures.push_back(texture);

                    pgl::uint64v idx = texture->handleTexture();
                    context->makeResidentTexture(idx);
                    texnames[tex] = idx;//(textures.size() - 1);
                }
                else {
                    texnames[tex] = 0xFFFFFFFFFFFFFFFF;
                }
            }
            return texnames[tex];
        }
        */
    };
}