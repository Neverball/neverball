#pragma once

#include "includes.hpp"
#include "utils.hpp"
#include "mesh.hpp"

namespace PathTracer {

    class Intersector : public PTObject {
    private:
        CLTracerAPI clapi;

        pgl::boolv dirty = false;
        pgl::uintv maxt = 1024 * 1024 * 1;

        pgl::Context context;

        pgl::Program geometryLoaderProgram2;
        pgl::Program buildProgramH;
        pgl::Program aabbMakerProgramH;
        pgl::Program fresetProgramH;
        pgl::Program refitProgramH;
        pgl::Program resortProgramH;
        pgl::Program minmaxProgram2;
        //pgl::Program sortProgram;

        pgl::Buffer<pgl::floatv> temp_vbo = nullptr;
        pgl::Buffer<pgl::intv> temp_srt = nullptr;
        pgl::Buffer<pgl::floatv> temp_rvbo = nullptr;
        pgl::Buffer<pgl::intv> ebo_triangle_ssbo = nullptr;
        pgl::Buffer<pgl::floatv> vbo_triangle_ssbo = nullptr;
        pgl::Buffer<pgl::floatv> norm_triangle_ssbo = nullptr;
        pgl::Buffer<pgl::floatv> tex_triangle_ssbo = nullptr;
        pgl::Buffer<pgl::intv> mat_triangle_ssbo = nullptr;

        pgl::Buffer<MinmaxUniformStruct> minmaxUniform = nullptr;
        pgl::Buffer<HelperUniformStruct> helperUniform = nullptr;
        pgl::Buffer<OctreeUniformStruct> octreeUniform = nullptr;
        pgl::Buffer<GeometryUniformStruct> geometryUniform = nullptr;
        //pgl::VertexArray vao;

        pgl::Buffer<pgl::uintv> nodeCounter = nullptr;
        pgl::Buffer<pgl::uintv2> numBuffer = nullptr;
        pgl::Buffer<Leaf> leafBuffer = nullptr;
        pgl::Buffer<Leaf> leafBufferSorted = nullptr;
        pgl::Buffer<HlbvhNode> bvhnodesBuffer = nullptr;
        pgl::Buffer<pgl::uintv> mortonBuffer = nullptr;
        pgl::Buffer<pgl::uintv> mortonBufferIndex = nullptr;
        pgl::Buffer<pgl::uintv> mortonBufferSorted = nullptr;

        pgl::Buffer<pgl::uintv> bvhflagsBuffer = nullptr;
        pgl::Buffer<pgl::intv> lscounterTemp = nullptr;
        pgl::Buffer<Minmaxi> minmaxBuf = nullptr;
        pgl::Buffer<Minmaxi> minmaxBufRef = nullptr;
        pgl::Uniform<pgl::intv> fresetRangeUniform = nullptr;

    private:
        void initShaderCompute(std::string str, pgl::Program& prog) {
            pgl::Shader comp = context->createShader(pgl::ShaderType::Compute);

            comp->sourceCompile(pgl::readSource(str));
            if (!comp->compileStatus()) std::cerr << comp->infoLog() << std::endl;
            prog = context->createProgram()->attachShaderLink({ comp });
            if (!prog->linkStatus()) std::cerr << prog->infoLog() << std::endl;
        }

        void initShaders() {
            //initShaderCompute("./shaders/hlbvh/sort.comp", sortProgram);
            initShaderCompute("./shaders/hlbvh/resort.comp", resortProgramH);
            initShaderCompute("./shaders/hlbvh/refit.comp", refitProgramH);
            initShaderCompute("./shaders/hlbvh/flag.comp", fresetProgramH);
            initShaderCompute("./shaders/hlbvh/build.comp", buildProgramH);
            initShaderCompute("./shaders/hlbvh/aabbmaker.comp", aabbMakerProgramH);
            initShaderCompute("./shaders/hlbvh/minmax.comp", minmaxProgram2);
            initShaderCompute("./shaders/tools/loader.comp", geometryLoaderProgram2);
            fresetRangeUniform = fresetProgramH->uniform<pgl::intv>("flagLen");
        }

    public:
        void init() {
            initShaders();

            //vao = context->createVertexArray();
            pgl::BufferStorageDescriptor desc;
            desc.storageFlags = pgl::BufferStorageFlags::Dynamic;

            minmaxBufRef = context->createBuffer<Minmaxi>()->storage(1, desc);
            minmaxBuf = context->createBuffer<Minmaxi>()->storage(1, desc);
            lscounterTemp = context->createBuffer<pgl::intv>()->storage(1, desc)->subdata(std::vector<pgl::intv>({ 0 }));

            minmaxUniform = context->createBuffer<MinmaxUniformStruct>()->storage(1, desc);
            helperUniform = context->createBuffer<HelperUniformStruct>()->storage(1, desc);
            octreeUniform = context->createBuffer<OctreeUniformStruct>()->storage(1, desc);
            geometryUniform = context->createBuffer<GeometryUniformStruct>()->storage(1, desc);

            Minmaxi bound;
            bound.mn.x = 10000000;
            bound.mn.y = 10000000;
            bound.mn.z = 10000000;
            bound.mn.w = 10000000;
            bound.mx.x = -10000000;
            bound.mx.y = -10000000;
            bound.mx.z = -10000000;
            bound.mx.w = -10000000;
            minmaxBufRef->subdata(&bound);
            minmaxBuf->subdata(&bound);
        }

    public:

        void syncUniforms() {
            minmaxUniform->subdata(&minmaxUniformData);
            helperUniform->subdata(&helperUniformData);
            octreeUniform->subdata(&octreeUniformData);
            geometryUniform->subdata(&geometryUniformData);
        }

        MinmaxUniformStruct minmaxUniformData;
        HelperUniformStruct helperUniformData;
        OctreeUniformStruct octreeUniformData;
        GeometryUniformStruct geometryUniformData;
        pgl::intv worksize = 64;

        Intersector(pgl::Context ctx, CLTracerAPI api = CLTracerAPI()) {
            context = ctx;
            clapi = api;
            init();
        }

        pgl::floatv3 offset;
        pgl::floatv3 scale;
        pgl::intv materialID = 0;
        pgl::intv maxDepth = 4;
        pgl::uintv triangleCount = 0;
        pgl::uintv verticeCount = 0;

#ifdef OPENCL_SUPPORT
        cl_mem clMorton;
        vex::vector<cl_ulong> * vexMorton;
#endif

        void allocate(pgl::intv count, pgl::intv d = 1) {
            maxDepth = d;
            pgl::uintv size_r = pow(2, maxDepth - 1);
            maxt = count;
            octreeUniformData.maxDepth = maxDepth - 1;

            //Geometry
            ebo_triangle_ssbo = context->createBuffer<pgl::intv>()->storage(maxt * 3);
            vbo_triangle_ssbo = context->createBuffer < pgl::floatv >()->storage(maxt * 9);
            norm_triangle_ssbo = context->createBuffer < pgl::floatv >()->storage(maxt * 9);
            tex_triangle_ssbo = context->createBuffer < pgl::floatv >()->storage(maxt * 6);
            mat_triangle_ssbo = context->createBuffer<pgl::intv>()->storage(maxt);

            nodeCounter = context->createBuffer<pgl::uintv>()->storage(1);
            nodeCounter->subdata(std::vector<pgl::uintv>({ 0 }), 0);
            numBuffer = context->createBuffer<pgl::uintv2>()->storage(1);
            numBuffer->subdata(std::vector<pgl::uintv2>({ { 0, 1 } }), 0);

            mortonBuffer = context->createBuffer<pgl::uintv>()->storage(maxt);
            mortonBufferIndex = context->createBuffer<pgl::uintv>()->storage(maxt);
            mortonBufferSorted = context->createBuffer<pgl::uintv>()->storage(maxt);

            bvhnodesBuffer = context->createBuffer<HlbvhNode>()->storage(maxt * 3);
            bvhflagsBuffer = context->createBuffer<pgl::uintv>()->storage(maxt * 3);
            leafBufferSorted = context->createBuffer<Leaf>()->storage(maxt);
            leafBuffer = context->createBuffer<Leaf>()->storage(maxt);

#ifdef OPENCL_SUPPORT
            clMorton = clCreateFromGLBuffer(clapi.ctx, CL_MEM_READ_WRITE, mortonBuffer->glID(), 0);
            vexMorton = new vex::vector<cl_ulong>(cl::CommandQueue(clapi.queue), cl::Buffer(clMorton));
#endif
        }

        void setMaterialID(pgl::intv id) {
            materialID = id;
        }

        void bindUniforms() {
            context->binding(1)->target(pgl::BufferTarget::Uniform)->buffer(minmaxUniform);
            context->binding(5)->target(pgl::BufferTarget::Uniform)->buffer(geometryUniform);
            context->binding(6)->target(pgl::BufferTarget::Uniform)->buffer(helperUniform);
            context->binding(7)->target(pgl::BufferTarget::Uniform)->buffer(octreeUniform);
        }

        void bind() {
            context->binding(3)->target(pgl::BufferTarget::ShaderStorage)->buffer(vbo_triangle_ssbo);
            context->binding(4)->target(pgl::BufferTarget::ShaderStorage)->buffer(ebo_triangle_ssbo);
            context->binding(5)->target(pgl::BufferTarget::ShaderStorage)->buffer(norm_triangle_ssbo);
            context->binding(6)->target(pgl::BufferTarget::ShaderStorage)->buffer(tex_triangle_ssbo);
            context->binding(7)->target(pgl::BufferTarget::ShaderStorage)->buffer(mat_triangle_ssbo);
        }

        void bindBVH() {
            context->binding(0)->target(pgl::BufferTarget::ShaderStorage)->buffer(bvhnodesBuffer);
            context->binding(1)->target(pgl::BufferTarget::ShaderStorage)->buffer(mortonBuffer);
            context->binding(2)->target(pgl::BufferTarget::ShaderStorage)->buffer(mortonBufferIndex);
        }

        void clearTribuffer() {
            triangleCount = 0;
            verticeCount = 0;
            markDirty();
        }

        void loadMesh(Mesh * gobject) {
            if (gobject->triangleCount <= 0 || gobject == nullptr) return;

            pgl::intv trioff = triangleCount;
            triangleCount += gobject->triangleCount;
            verticeCount = triangleCount * 3;

            geometryUniformData.unindexed = gobject->unindexed;
            geometryUniformData.loadOffset = gobject->offset;
            geometryUniformData.materialID = gobject->materialID;
            geometryUniformData.triangleOffset = trioff;
            geometryUniformData.triangleCount = gobject->triangleCount;
            geometryUniformData.transform = gobject->trans;
            geometryUniformData.transformInv = glm::inverse(gobject->trans);
            geometryUniformData.colormod = gobject->colormod;
            geometryUniformData.offset = gobject->voffset;

            bind();
            gobject->bind();
            bindUniforms();
            syncUniforms();

            context->useProgram(geometryLoaderProgram2)->dispatchCompute(tiled(gobject->triangleCount, worksize))->flush();
            markDirty();
        }

        bool isDirty() const {
            return dirty;
        }

        void markDirty() {
            dirty = true;
        }

        void resolve() {
            dirty = false;
        }

        void build() {
            if (triangleCount <= 0 || mortonBuffer->size() <= 0) return;

            bind();
            bindUniforms();
            syncUniforms();

            pgl::BufferCopyDataDescriptor cdesc;
            cdesc.readOffset = 0;
            cdesc.writeOffset = 0;
            cdesc.readSize = 1;

            const pgl::floatv prec = 10000.0f;
            minmaxBufRef->copydata(minmaxBuf, cdesc);
            geometryUniformData.triangleOffset = 0;
            geometryUniformData.triangleCount = triangleCount;
            minmaxUniformData.prec = prec;
            syncUniforms();

            context->binding(10)->target(pgl::BufferTarget::ShaderStorage)->buffer(minmaxBuf);
            context->useProgram(minmaxProgram2)->dispatchCompute(tiled(triangleCount, worksize))->flush();

            Minmaxi bound = minmaxBuf->subdata(0, 1)[0];
            pgl::floatv3 mn = pgl::floatv3(bound.mn.x, bound.mn.y, bound.mn.z) / prec - pgl::floatv3(0.001f);
            pgl::floatv3 mx = pgl::floatv3(bound.mx.x, bound.mx.y, bound.mx.z) / prec + pgl::floatv3(0.001f);
            pgl::floatv3 full = mx - mn;//pgl::floatv3(glm::compMax(mx - mn));
            scale = full;
            offset = mn;

            glm::mat4 mat(1.0f);
            mat = glm::scale(mat, 1.0f / scale);
            mat = glm::translate(mat, -offset);

            octreeUniformData.project = mat;
            octreeUniformData.unproject = glm::inverse(mat);
            octreeUniform->subdata(&octreeUniformData);

            context->binding(0)->target(pgl::BufferTarget::ShaderStorage)->buffer(leafBuffer);
            context->binding(1)->target(pgl::BufferTarget::ShaderStorage)->buffer(mortonBuffer);
            context->binding(2)->target(pgl::BufferTarget::ShaderStorage)->buffer(mortonBufferIndex);
            //context->binding(3)->target(pgl::BufferTarget::ShaderStorage)->buffer(leafBufferSorted);
            //context->binding(4)->target(pgl::BufferTarget::ShaderStorage)->buffer(mortonBufferSorted);
            context->useProgram(aabbMakerProgramH)->dispatchCompute(tiled(triangleCount, worksize))->flush();

            pgl::BufferCopyDataDescriptor ccd;
            ccd.readOffset = 0;
            ccd.readSize = triangleCount;
            ccd.writeOffset = 0;

            //mortonBuffer->copydata(mortonBufferSorted, ccd);

#ifdef OPENCL_SUPPORT
            context->flush();
            clEnqueueAcquireGLObjects(clapi.queue, 1, &clMorton, 0, 0, 0);
            vex::sort(*vexMorton);
            clEnqueueReleaseGLObjects(clapi.queue, 1, &clMorton, 0, 0, 0);
            clFinish(clapi.queue);
#else 
            //sortPair(mortonBuffer->glID(), triangleCount);
            //parallel::gl::radix_sort(parallel::gl::GL::instance(), { mortonBufferIndex->glID() }, triangleCount );
            parallel::gl::radix_sort(parallel::gl::GL::instance(), { mortonBuffer->glID() }, triangleCount, { mortonBufferIndex->glID() });
#endif
            context->flush();

            context->binding(0)->target(pgl::BufferTarget::ShaderStorage)->buffer(leafBuffer);
            context->binding(1)->target(pgl::BufferTarget::ShaderStorage)->buffer(mortonBuffer);
            context->binding(2)->target(pgl::BufferTarget::ShaderStorage)->buffer(mortonBufferIndex);
            context->binding(3)->target(pgl::BufferTarget::ShaderStorage)->buffer(leafBufferSorted);
            context->binding(4)->target(pgl::BufferTarget::ShaderStorage)->buffer(nullptr);
            context->useProgram(resortProgramH)->dispatchCompute(tiled(triangleCount, worksize))->flush();
            //mortonBufferSorted->copydata(mortonBuffer, ccd);

            //std::vector<pgl::uintv> debugidc = mortonBuffer->subdata(0, triangleCount);

            context->binding(0)->target(pgl::BufferTarget::AtomicCounter)->buffer(nodeCounter);
            context->binding(0)->target(pgl::BufferTarget::ShaderStorage)->buffer(numBuffer);
            context->binding(1)->target(pgl::BufferTarget::ShaderStorage)->buffer(mortonBuffer);
            context->binding(2)->target(pgl::BufferTarget::ShaderStorage)->buffer(mortonBufferIndex);
            context->binding(3)->target(pgl::BufferTarget::ShaderStorage)->buffer(leafBufferSorted);
            context->binding(4)->target(pgl::BufferTarget::ShaderStorage)->buffer(bvhnodesBuffer);
            context->binding(5)->target(pgl::BufferTarget::ShaderStorage)->buffer(bvhflagsBuffer);

            lscounterTemp->copydata(nodeCounter, cdesc);
            pgl::intv2 range = { 0, 1 };

            context->useProgram(buildProgramH);
            for (pgl::intv i = 1;i < 200;i++) {
                numBuffer->subdata(std::vector<pgl::intv2>({ range }), 0);
                octreeUniformData.currentDepth = i;
                octreeUniform->subdata(&octreeUniformData);

                context->dispatchCompute(tiled(range.y - range.x, worksize))->flush();
                range.x = range.y;
                range.y = 1 + nodeCounter->subdata(0, 1)[0] * 2;
                if (range.y <= range.x) break;
            }

            *fresetRangeUniform = range.y;
            context->useProgram(fresetProgramH)->dispatchCompute(tiled(range.y, worksize))->flush();
            context->useProgram(refitProgramH)->dispatchCompute(tiled(triangleCount, worksize))->flush();
        }
    };
}
