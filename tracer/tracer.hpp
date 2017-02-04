#pragma once

#include "includes.hpp"
#include "utils.hpp"
#include "structs.hpp"
#include "intersector.hpp"
#include "material.hpp"
#include <cmath>

namespace PathTracer {

    class Tracer : public PTObject {
        
        CLTracerAPI clapi;

        pgl::Program renderProgram;
        pgl::Program matProgram;
        pgl::Program beginProgram;
        pgl::Program closeProgram;
        pgl::Program reclaimProgram;
        pgl::Program cameraProgram;
        pgl::Program clearProgram;
        pgl::Program samplerProgram;
        pgl::Program filterProgram;
        pgl::Program intersectionProgram;
        
        pgl::Context context;
        pgl::Buffer<pgl::intv> activenl;
        pgl::Buffer<pgl::intv> activel;
        pgl::Buffer<Ray> rays;
        pgl::Buffer<Hit> hits;
        pgl::Buffer<Texel> texels;
        pgl::Buffer<pgl::intv> arcounter;
        pgl::Buffer<pgl::intv> arcounterTemp;

        pgl::Buffer<RandomUniformStruct> randomUniform;
        pgl::Buffer<LightUniformStruct> lightUniform;
        pgl::Buffer<MaterialUniformStruct> materialUniform;
        pgl::Buffer<SamplerUniformStruct> samplerUniform;
        pgl::Buffer<CameraUniformStruct> cameraUniform;

        pgl::intv currentSample = 0;
        pgl::intv maxSamples = 4;
        pgl::intv maxFilters = 1;

        pgl::Texture2D presampled;
        pgl::Texture2D samples;
        pgl::Texture2D sampleflags;
        pgl::TextureCube cubeTex = nullptr;
        pgl::VertexArray vao;

        pgl::Buffer<pgl::floatv2> posBuf;
        pgl::Buffer<pgl::intv> idcBuf;
        pgl::Attribute<pgl::floatv2> posattr;

    public:

        //RadeonRays::Buffer * raysRAD;
        //RadeonRays::Buffer * hitsRAD;
        //RadeonRays::Buffer * activesRAD;

    private:
        void initShaderCompute(std::string str, pgl::Program& prog) {
            pgl::Shader comp = context->createShader(pgl::ShaderType::Compute);

            comp->sourceCompile(pgl::readSource(str));
            if (!comp->compileStatus()) std::cerr << comp->infoLog() << std::endl;
            prog = context->createProgram()->attachShaderLink({ comp });
            if (!prog->linkStatus()) std::cerr << prog->infoLog() << std::endl;
        }


        void initShaders() {
            initShaderCompute("./shaders/render/filter.comp", filterProgram);
            initShaderCompute("./shaders/render/testmat.comp", matProgram);
            initShaderCompute("./shaders/render/begin.comp", beginProgram);
            initShaderCompute("./shaders/render/close.comp", closeProgram);
            initShaderCompute("./shaders/render/reclaim.comp", reclaimProgram);
            initShaderCompute("./shaders/render/camera.comp", cameraProgram);
            initShaderCompute("./shaders/render/clear.comp", clearProgram);
            initShaderCompute("./shaders/render/sampler.comp", samplerProgram);
            initShaderCompute("./shaders/render/intersection.comp", intersectionProgram);

            {
                pgl::Shader vert = context->createShader(pgl::ShaderType::Vertex);
                pgl::Shader frag = context->createShader(pgl::ShaderType::Fragment);
                vert->sourceCompile(pgl::readSource("./shaders/render/render.vert"));
                frag->sourceCompile(pgl::readSource("./shaders/render/render.frag"));
                if (!vert->compileStatus()) std::cerr << vert->infoLog() << std::endl;
                if (!frag->compileStatus()) std::cerr << frag->infoLog() << std::endl;
                renderProgram = context->createProgram();
                renderProgram->attachShaderLink({ vert , frag });
                if (!renderProgram->linkStatus()) std::cerr << renderProgram->infoLog() << std::endl;
            }
        }

        void init() {
            initShaders();

            arcounter = context->createBuffer<pgl::intv>();
            arcounterTemp = context->createBuffer<pgl::intv>()->storage(1);
            randomUniform = context->createBuffer<RandomUniformStruct>()->storage(1);
            lightUniform = context->createBuffer<LightUniformStruct>()->storage(1);
            materialUniform = context->createBuffer<MaterialUniformStruct>()->storage(1);
            samplerUniform = context->createBuffer<SamplerUniformStruct>()->storage(1);
            cameraUniform = context->createBuffer<CameraUniformStruct>()->storage(1);

            posBuf = context->createBuffer<pgl::floatv2>()->data(std::vector<pgl::floatv2>({
                { -1.0f, -1.0f },{ 1.0f, -1.0f },{ -1.0f, 1.0f },{ 1.0f, 1.0f } 
            }));
            idcBuf = context->createBuffer<pgl::intv>()->data(std::vector<pgl::intv>({ 0, 1, 2, 3, 2, 1 }));

            posattr = renderProgram->attribute<pgl::floatv2>(0)->offset(0);
            vao = context->createVertexArray();
            vao->element(idcBuf)->binding(0)->buffer(posBuf, 0)->attribute(posattr);

            arcounter->storage(3);
            arcounterTemp->data(std::vector<pgl::intv>({ 0 }));

            lightUniformData.lightColor = lightColor;
            lightUniformData.lightVector = lightVector;
            lightUniformData.lightDistance = lightDistance;
            lightUniformData.lightRadius = lightRadius;
            samplerUniformData.currentSample = currentSample;
            samplerUniformData.maxSamples = maxSamples;
            samplerUniformData.maxFilters = maxFilters;
            cameraUniformData.enable360 = 0;

            //resize(width, height);
        }

    public:
        pgl::intv width = 800;
        pgl::intv height = 600;

        pgl::intv displayWidth = 800;
        pgl::intv displayHeight = 600;

        void switchMode() {
            clearRays();
            clearSampler();
            cameraUniformData.enable360 = cameraUniformData.enable360 == 1 ? 0 : 1;
        }

        void resize(const pgl::intv & w, const pgl::intv & h) {
            displayWidth = w;
            displayHeight = h;

            samples = context->createTexture2D();
            sampleflags = context->createTexture2D();
            presampled = context->createTexture2D();

            pgl::TextureStorageDescriptor sdesc;
            sdesc.internalFormat = pgl::InternalFormatConstructor::F32(4);
            sdesc.size = pgl::intv3(displayWidth, displayHeight, maxSamples);

            pgl::TextureStorageDescriptor sfdesc;
            sfdesc.internalFormat = pgl::InternalFormatConstructor::U32(1);
            sfdesc.size = pgl::intv3(displayWidth, displayHeight, maxSamples);

            samples
                ->wrap<0>(pgl::TextureWrap::ClampToEdge)
                ->wrap<1>(pgl::TextureWrap::ClampToEdge)
                ->magFilter(pgl::TextureFilter::Nearest)
                ->minFilter(pgl::TextureFilter::Nearest)
                ->storage(sdesc);

            presampled
                ->wrap<0>(pgl::TextureWrap::ClampToEdge)
                ->wrap<1>(pgl::TextureWrap::ClampToEdge)
                ->magFilter(pgl::TextureFilter::Nearest)
                ->minFilter(pgl::TextureFilter::Nearest)
                ->storage(sdesc);

            sampleflags
                ->wrap<0>(pgl::TextureWrap::ClampToEdge)
                ->wrap<1>(pgl::TextureWrap::ClampToEdge)
                ->magFilter(pgl::TextureFilter::Nearest)
                ->minFilter(pgl::TextureFilter::Nearest)
                ->storage(sfdesc);

            clearSampler();
        }

        void resizeBuffers(const pgl::intv & w, const pgl::intv & h) {
            width = w;
            height = h;

            pgl::intv wrsize = width * height;
            rays = context->createBuffer<Ray>()->storage(wrsize * 8);//new pgl::Buffer<Ray>(pgl::BufferTarget::ShaderStorage)->storage(wrsize * 8);
            hits = context->createBuffer<Hit>()->storage(wrsize * 8);//new pgl::Buffer<Hit>(pgl::BufferTarget::ShaderStorage)->storage(wrsize * 8);
            activel = context->createBuffer<pgl::intv>()->storage(wrsize * 8);
            activenl = context->createBuffer<pgl::intv>()->storage(wrsize * 8);
            texels = context->createBuffer<Texel>()->storage(wrsize);

            samplerUniformData.sceneRes = pgl::floatv2(width, height); 

            clearRays();
            syncUniforms();
        }

        void syncUniforms() {
            randomUniform->subdata(&randomUniformData);
            lightUniform->subdata(&lightUniformData);
            materialUniform->subdata(&materialUniformData);
            samplerUniform->subdata(&samplerUniformData);
            cameraUniform->subdata(&cameraUniformData);
        }

        pgl::intv worksize = 64;
        RandomUniformStruct randomUniformData;
        LightUniformStruct lightUniformData;
        MaterialUniformStruct materialUniformData;
        SamplerUniformStruct samplerUniformData;
        CameraUniformStruct cameraUniformData;

        bool raycountCacheClear = false;
        pgl::intv raycountCache = 0;
        pgl::intv qraycountCache = 0;

        glm::vec4 lightColor = glm::vec4(glm::vec3(3000.0f), 1.0f);
        glm::vec4 lightVector = glm::vec4(0.1f, 1.0f, 0.1f, 1.0f);
        pgl::floatv lightDistance = 50.0f;
        pgl::floatv lightRadius = 5.0f;

        Tracer(pgl::Context ctx, CLTracerAPI api) { 
            context = ctx;  
            clapi = api;
            init();
        }

        void includeCubemap(pgl::TextureCube cube) { cubeTex = cube; }

        void bindUniforms() {
            context->binding(0)->target(pgl::BufferTarget::Uniform)->buffer(randomUniform);
            context->binding(2)->target(pgl::BufferTarget::Uniform)->buffer(materialUniform);
            context->binding(3)->target(pgl::BufferTarget::Uniform)->buffer(samplerUniform);
            context->binding(4)->target(pgl::BufferTarget::Uniform)->buffer(lightUniform);
            context->binding(8)->target(pgl::BufferTarget::Uniform)->buffer(cameraUniform);
        }

        void bind() {
            context->binding(0)->target(pgl::BufferTarget::AtomicCounter)->buffer(arcounter);
            context->binding(8)->target(pgl::BufferTarget::ShaderStorage)->buffer(rays);
            context->binding(9)->target(pgl::BufferTarget::ShaderStorage)->buffer(hits);
            context->binding(10)->target(pgl::BufferTarget::ShaderStorage)->buffer(texels);
            context->binding(13)->target(pgl::BufferTarget::ShaderStorage)->buffer(activel);
            bindUniforms();
        }

        void clearRays() {
            pgl::BufferCopyDataDescriptor cdesc;
            cdesc.readOffset = 0;
            cdesc.writeOffset = 0;
            cdesc.readSize = 1;

            arcounterTemp->copydata(arcounter, cdesc);

            cdesc.writeOffset = 4;
            arcounterTemp->copydata(arcounter, cdesc);

            cdesc.writeOffset = 8;
            arcounterTemp->copydata(arcounter, cdesc);
        }

        void resetHits() {
            pgl::intv rsize = getRayCount();
            if (rsize <= 0) return;

            bind();
            context->useProgram(beginProgram)->dispatchCompute(tiled(rsize, worksize));
            context->flush();
        }

        void sample() {
            bind();
            syncUniforms();

            pgl::ImageUnitDescriptor unid;
            unid.access = pgl::ImageAccess::WriteOnly;
            unid.layer = 0;
            unid.level = 0;

            context->imageUnit(0)->texture(samples, unid);
            context->imageUnit(1)->texture(sampleflags, unid);
            context->imageUnit(2)->texture(presampled, unid);
            context->useProgram(samplerProgram)->dispatchCompute(tiled(displayWidth * displayHeight, worksize));
            context->flush();

            currentSample = (currentSample + 1) % maxSamples;
            samplerUniformData.currentSample = currentSample;
        }

        void presample() {
            bind();
            syncUniforms();

            pgl::ImageUnitDescriptor unid;
            unid.access = pgl::ImageAccess::WriteOnly;
            unid.layer = 0;
            unid.level = 0;

            context->imageUnit(0)->texture(samples, unid);
            context->imageUnit(1)->texture(sampleflags, unid);
            context->imageUnit(2)->texture(presampled, unid);
            context->useProgram(samplerProgram)->dispatchCompute(tiled(displayWidth * displayHeight, worksize));
            context->flush();
        }

        void camera(const glm::vec3 &eye, const glm::vec3 &view) {
            clearRays();
            bind();

            randomUniformData.time = frandom();
            cameraUniformData.camInv = glm::inverse(glm::lookAt(eye, view, glm::vec3(0.0f, 1.0f, 0.0f)));
            cameraUniformData.camInv2 = glm::inverse(glm::lookAt(eye, eye - (view - eye), glm::vec3(0.0f, 1.0f, 0.0f)));
            cameraUniformData.eye = glm::vec4(eye, 1.0f);
            cameraUniformData.view = glm::vec4(view, 1.0f);
            cameraUniformData.projInv = glm::inverse(glm::perspective(((float)M_PI / 3.0f), (float)displayWidth / (float)displayHeight, 0.1f, 1000.0f));
            cameraUniformData.prob = 1.0f;
            syncUniforms();

            context->useProgram(cameraProgram)->dispatchCompute(tiled(width * height, worksize));
            context->flush();
            raycountCacheClear = true;
            //sortIndices(activel->glID(), getRayCount());
        }

        void camera(const glm::vec3 &eye, const glm::vec3 &view, const glm::mat4 &persp) {
            clearRays();
            bind();

            randomUniformData.time = frandom();
            cameraUniformData.camInv = glm::inverse(glm::lookAt(eye, view, glm::vec3(0.0f, 1.0f, 0.0f)));
            cameraUniformData.camInv2 = glm::inverse(glm::lookAt(eye, eye - (view - eye), glm::vec3(0.0f, 1.0f, 0.0f)));
            cameraUniformData.eye = glm::vec4(eye, 1.0f);
            cameraUniformData.view = glm::vec4(view, 1.0f);
            cameraUniformData.projInv = glm::inverse(persp);
            cameraUniformData.prob = 1.0f;
            syncUniforms();

            context->useProgram(cameraProgram)->dispatchCompute(tiled(width * height, worksize));
            context->flush();
            raycountCacheClear = true;
            //sortIndices(activel->glID(), getRayCount());
        }

        void clearSampler() {
            //for (int i = 0;i < maxSamples;i++) {
                samplerUniformData.samplecount = displayWidth * displayHeight;
                samplerUniformData.currentSample = 0;
                samplerUniformData.maxSamples = maxSamples;
                syncUniforms();

                pgl::ImageUnitDescriptor unid;
                unid.access = pgl::ImageAccess::WriteOnly;
                unid.layer = 0;
                unid.level = 0;

                bind();
                context->imageUnit(0)->texture(sampleflags, unid);
                context->useProgram(clearProgram)->dispatchCompute(tiled(displayWidth * displayHeight, worksize));
                context->flush();
            //}
        }

        void missing() {
            raycountCacheClear = true;
            pgl::intv rsize = getRayCount();
            if (rsize <= 0) return;

            pgl::BufferCopyDataDescriptor cdesc;
            cdesc.readOffset = 0;
            cdesc.writeOffset = 0;
            cdesc.readSize = 1;
            arcounterTemp->copydata(arcounter, cdesc);

            bind();
            if (cubeTex) {
                context->textureUnit(0)->texture(cubeTex);
            }
            context->binding(14)->target(pgl::BufferTarget::ShaderStorage)->buffer(activenl);
            context->useProgram(closeProgram)->dispatchCompute(tiled(rsize, worksize));
            context->flush();

            raycountCacheClear = true;
            rsize = getRayCount();
            if (rsize <= 0) return;

            cdesc.readSize = rsize;
            activenl->copydata(activel, cdesc);
            //sortIndices(activel->glID(), rsize);
        }

        void reclaim() {
            raycountCacheClear = true;
            pgl::intv rsize = getRayCount();
            if (rsize <= 0) return;

            pgl::BufferCopyDataDescriptor cdesc;
            cdesc.readOffset = 0;
            cdesc.writeOffset = 0;
            cdesc.readSize = 1;
            arcounterTemp->copydata(arcounter, cdesc);

            bind();
            context->binding(14)->target(pgl::BufferTarget::ShaderStorage)->buffer(activenl);
            context->useProgram(reclaimProgram)->dispatchCompute(tiled(rsize, worksize));
            context->flush();

            raycountCacheClear = true;
            rsize = getRayCount();
            if (rsize <= 0) return;

            cdesc.readSize = rsize;
            activenl->copydata(activel, cdesc);
            //sortIndices(activel->glID(), rsize);
        }


        void prerender() {
            context->scissor({ 0, 0 }, { displayWidth, displayHeight });
            context->viewport({ 0, 0 }, { displayWidth, displayHeight });

            context->disable(pgl::Feature::DepthTest);
            context->disable(pgl::Feature::Blend);
            context->cullFace(pgl::Cullface::Back);
            context->textureUnit(0)->texture(presampled);

            bind();

            context->useProgram(renderProgram);
            context->bindVertexArray(vao);
        }


        void render() {
            context->scissor({ 0, 0 }, { displayWidth, displayHeight });
            context->viewport({ 0, 0 }, { displayWidth, displayHeight });

            context->disable(pgl::Feature::DepthTest);
            context->disable(pgl::Feature::Blend);
            context->cullFace(pgl::Cullface::Back);
            context->textureUnit(0)->texture(presampled);

            bind();

            pgl::DrawDescriptor ddesc;
            ddesc.primitiveType = pgl::PrimitiveType::Triangles;
            ddesc.indexCount = 6;

            context->useProgram(renderProgram);
            context->bindVertexArray(vao);
            context->drawElements(ddesc);

            //context->flush();
        }

        void intersection(Intersector * obj) {
            pgl::intv rsize = getRayCount();
            if (rsize <= 0 || obj->triangleCount <= 0) return;

            if (obj->isDirty()) obj->build();

            obj->bind();
            obj->bindBVH();
            obj->bindUniforms();
            bind();
            context->useProgram(intersectionProgram)->dispatchCompute(tiled(rsize, worksize));
            context->flush();

            obj->resolve();
        }

        void shade(Material * mat) {
            pgl::intv rsize = getRayCount();
            if (rsize <= 0) return;

            mat->bindWithContext();
            samplerUniformData.rayCount = rsize;
            randomUniformData.time = frandom();
            syncUniforms();
            bind();

            context->useProgram(matProgram)->dispatchCompute(tiled(rsize, worksize));
            context->flush();
        }

        pgl::intv getRayCount() {
            if (raycountCacheClear) {
                raycountCache = arcounter->subdata( 0, 1 )[0];
                if (raycountCache < 0) raycountCache = 0; //don't need trace with few rays
                raycountCacheClear = false;
                samplerUniformData.rayCount = raycountCache;
                syncUniforms();
            }
            return raycountCache;
        }
    };
}

