#pragma once

#include "includes.hpp"
#include "utils.hpp"

//#define TOL_SUPPORT

#ifdef TOL_SUPPORT
#include "tiny_obj_loader/tiny_obj_loader.h"
#endif

namespace PathTracer {
    class Mesh : public PTObject {
    public:
        Mesh(pgl::Context ctx) {
            context = ctx;
            vbo_triangle_ssbo = context->createBuffer<pgl::floatv>();
            norm_triangle_ssbo = context->createBuffer<pgl::floatv>();
            tex_triangle_ssbo = context->createBuffer<pgl::floatv>();
            mat_triangle_ssbo = context->createBuffer<pgl::intv>();
            vebo_triangle_ssbo = context->createBuffer<pgl::intv>();
            nebo_triangle_ssbo = context->createBuffer<pgl::intv>();
            tebo_triangle_ssbo = context->createBuffer<pgl::intv>();
        }

    private:
        pgl::Context context;
        pgl::Buffer<pgl::floatv> vbo_triangle_ssbo;
        pgl::Buffer<pgl::floatv>norm_triangle_ssbo;
        pgl::Buffer<pgl::floatv> tex_triangle_ssbo;
        pgl::Buffer<pgl::intv>  mat_triangle_ssbo;
        pgl::Buffer<pgl::intv> vebo_triangle_ssbo;
        pgl::Buffer<pgl::intv> nebo_triangle_ssbo;
        pgl::Buffer<pgl::intv> tebo_triangle_ssbo;

    public:
        glm::mat4 trans;
        pgl::intv materialID = 0;
        pgl::intv maxDepth = 4;
        pgl::uintv triangleCount = 0;
        pgl::uintv verticeCount = 0;
        pgl::intv unindexed = 0;
        pgl::intv offset = 0;
        pgl::floatv4 colormod = {1.0f, 1.0f, 1.0f, 0.0f};
        pgl::floatv voffset = 0;

        void setVerticeOffset(pgl::floatv voff) {
            voffset = voff;
        }

        void setColorModifier(pgl::floatv4 color) {
            colormod = color;
        }

        void setMaterialOffset(pgl::intv id) {
            materialID = id;
        }

        void setTransform(const glm::mat4 &t) {
            trans = t;
        }

        void setIndexed(const pgl::intv b) {
            unindexed = b == 0 ? 1 : 0;
        }

        void setVertices(const pgl::Buffer<pgl::floatv> &buf) {
            vbo_triangle_ssbo = buf;
        }

        void setNormals(const pgl::Buffer<pgl::floatv> &buf) {
            norm_triangle_ssbo = buf;
        }

        void setTexcoords(const pgl::Buffer<pgl::floatv> &buf) {
            tex_triangle_ssbo = buf;
        }

        void setIndices(const pgl::Buffer<pgl::intv> &buf, const pgl::boolv &all = true) {
            vebo_triangle_ssbo = buf;
            //triangleCount = buf->size() / 3;
            if (all) {
                nebo_triangle_ssbo = buf;
                tebo_triangle_ssbo = buf;
            }
        }

        void setNIndices(const pgl::Buffer<pgl::intv> &buf) {
            nebo_triangle_ssbo = buf;
        }

        void setTIndices(const pgl::Buffer<pgl::intv> &buf) {
            tebo_triangle_ssbo = buf;
        }

        void setLoadingOffset(const pgl::intv &off) {
            offset = off;
        }

#ifdef TOL_SUPPORT
        void loadMesh(std::vector<tinyobj::shape_t>& shape, tinyobj::attrib_t &attrib) {
            std::vector<pgl::floatv> vertices(0);
            std::vector<pgl::intv> vindices(0);
            std::vector<pgl::intv> nindices(0);
            std::vector<pgl::intv> tindices(0);
            std::vector<pgl::floatv> normals(0);
            std::vector<pgl::floatv> texcoords(0);
            std::vector<pgl::floatv> colors(0);
            std::vector<pgl::intv> material_ids(0);

            for (uint64_t m = 0;m < shape.size();m++) {
                uint64_t index_offset = 0;
                for (uint64_t f = 0; f < shape[m].mesh.num_face_vertices.size(); f++) {
                    uint64_t fv = shape[m].mesh.num_face_vertices[f];
                    for (uint64_t v = 0; v < fv; v++) {
                        tinyobj::index_t idx = shape[m].mesh.indices[index_offset + v];
                        vindices.push_back(idx.vertex_index);
                        nindices.push_back(idx.normal_index);
                        tindices.push_back(idx.texcoord_index);
                    }
                    index_offset += fv;
                    int matid = shape[m].mesh.material_ids[f];
                    material_ids.push_back(matid >= 0 ? matid : 0);
                }
            }

            normals = attrib.normals;
            texcoords = attrib.texcoords;
            vertices = attrib.vertices;
            colors.resize(vertices.size() * 4 / 3);

            triangleCount = vindices.size() / 3;
            verticeCount = vertices.size() / 3;

            //fill data
            vebo_triangle_ssbo->data(vindices);
            nebo_triangle_ssbo->data(nindices);
            tebo_triangle_ssbo->data(tindices);
            mat_triangle_ssbo->data(material_ids);
            vbo_triangle_ssbo->data(vertices);
            norm_triangle_ssbo->data(normals);
            tex_triangle_ssbo->data(texcoords);
            col_triangle_ssbo->data(colors);
        }
#endif

        void bind() {
            context->binding(9)->target(pgl::BufferTarget::ShaderStorage)->buffer(vbo_triangle_ssbo);
            context->binding(10)->target(pgl::BufferTarget::ShaderStorage)->buffer(norm_triangle_ssbo);
            context->binding(11)->target(pgl::BufferTarget::ShaderStorage)->buffer(tex_triangle_ssbo);
            context->binding(12)->target(pgl::BufferTarget::ShaderStorage)->buffer(mat_triangle_ssbo);
            context->binding(13)->target(pgl::BufferTarget::ShaderStorage)->buffer(vebo_triangle_ssbo);
            context->binding(14)->target(pgl::BufferTarget::ShaderStorage)->buffer(nebo_triangle_ssbo);
            context->binding(15)->target(pgl::BufferTarget::ShaderStorage)->buffer(tebo_triangle_ssbo);
        }
    };
}
