#pragma once

#include "includes.hpp"
#include "utils.hpp"

namespace PathTracer {
    struct Thash {
        pgl::intv triangle;
        pgl::intv previous;
    };

    struct Voxel {
        pgl::intv depthLeaf;
        pgl::intv parent;
        pgl::intv last;
        pgl::intv suboff;
        pgl::intv coord;
        pgl::intv count;
        pgl::intv _t0;
    };

    struct Minmax {
        glm::vec4 mn;
        glm::vec4 mx;
    };

    struct Minmaxi {
        glm::ivec4 mn;
        glm::ivec4 mx;
    };

    struct Ray {
        pgl::intv idx;
        pgl::intv prev;
        pgl::intv texel;
        pgl::intv actived;
        glm::vec4 origin;
        glm::vec4 direct;
        glm::vec4 color;
        glm::vec4 final;
        glm::ivec4 params;
        glm::ivec4 extended;
    };

    struct Hit {
        glm::vec4 normal;
        glm::vec4 tangent;
        glm::vec4 color;
        glm::vec2 texcoord;
        pgl::floatv dist;
        pgl::floatv _t0;
        pgl::intv triangle;
        pgl::intv materialID;
        pgl::intv _t1;
        pgl::intv _t2;
    };

    struct Texel {
        pgl::intv last;
        pgl::intv count;
        pgl::floatv x;
        pgl::floatv y;
    };

    struct RandomUniformStruct {
        pgl::floatv time;
    };

    struct MinmaxUniformStruct {
        pgl::intv heap;
        pgl::floatv prec;
    };

    struct MaterialUniformStruct {
        pgl::intv materialID;
        pgl::intv doubleSide;
        pgl::floatv reflectivity;
        pgl::floatv dissolve;
        pgl::floatv illumPower;
        pgl::floatv ior;
    };

    struct SamplerUniformStruct {
        glm::vec2 sceneRes;
        pgl::intv samplecount;
        pgl::intv rayCount;
        pgl::intv iteration;
        pgl::intv phase;
        pgl::intv maxSamples;
        pgl::intv currentSample;
        pgl::intv maxFilters;
    };

    struct LightUniformStruct {
        glm::vec4 lightVector;
        glm::vec4 lightColor;
        pgl::floatv lightRadius;
        pgl::floatv lightDistance;
    };

    struct GeometryUniformStruct {
        glm::mat4 transform;
        glm::mat4 transformInv;
        glm::vec4 colormod = glm::vec4(0.0f);
        pgl::floatv offset = 0.0f;
        pgl::intv materialID = 0;
        pgl::intv triangleCount = 1;
        pgl::intv triangleOffset = 0;
        pgl::intv unindexed = 0;
        pgl::intv loadOffset = 0;
        pgl::intv NB_mode;
        pgl::intv unk1;
    };

    struct HelperUniformStruct {
        glm::ivec4 range;
    };

    struct OctreeUniformStruct {
        glm::mat4 project;
        glm::mat4 unproject;
        pgl::intv maxDepth;
        pgl::intv currentDepth;
        pgl::intv nodeCount;
    };

    struct CameraUniformStruct {
        glm::mat4 projInv;
        glm::mat4 camInv;
        glm::mat4 camInv2;
        glm::vec4 eye;
        glm::vec4 view;
        pgl::floatv prob;
        pgl::intv enable360;
    };

    struct bbox
    {
        glm::vec4 pmin;
        glm::vec4 pmax;
    };

    struct Leaf {
        bbox box;
        pgl::intv parent;
        pgl::intv _t0;
        pgl::intv _t1;
        pgl::intv _t2;
    };

    struct HlbvhNode {
        bbox box;
        pgl::intv2 range;
        pgl::intv parent;
        pgl::intv left;
        pgl::intv right;
        pgl::intv _t0;
        pgl::intv _t1;
        pgl::intv _t2;
    };

}