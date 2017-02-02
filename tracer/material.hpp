#pragma once

#include "includes.hpp"
#include "utils.hpp"

namespace PathTracer {
    class Material : public PTObject {
        friend class Tracer;

    public:

        struct Texpart {
            pgl::intv offset = -1;
            pgl::intv width = 0;
            pgl::intv height = 0;
            pgl::intv type = 1; //0 is UINT32, 1 is RGBA8, 2 is RGBA16
        };

        struct Submat {
            pgl::floatv4 diffuse = glm::vec4(0.0f);
            pgl::floatv4 specular = glm::vec4(0.0f);
            pgl::floatv4 transmission = glm::vec4(0.0f);
            pgl::floatv4 emissive = glm::vec4(0.0f);
            pgl::floatv ior = 1.0f;
            pgl::floatv reflectivity = 0.0001f;
            pgl::floatv s1;
            pgl::floatv s2;

            pgl::uint64v diffusePart = -1;
            pgl::uint64v specularPart = -1;
            pgl::uint64v bumpPart = -1;
            pgl::uint64v emissivePart = -1;

            int flags = 0;
            int nkvd0 = 0;
            int nkvd1 = 0;
            int nkvd2 = 0;
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
        }

        void bindWithContext() {
            context->binding(0)->target(pgl::BufferTarget::ShaderStorage)->buffer(mats);
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

        pgl::uint64v loadTexture(std::string tex) {
            if (tex == "") return -1;
            if (texnames.find(tex) == texnames.end()) {
                std::vector<unsigned char> image;
                unsigned width, height;
                unsigned error = lodepng::decode(image, width, height, tex);
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

    };
}
