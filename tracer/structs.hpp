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
        pgl::floatv4 mn;
        pgl::floatv4 mx;
    };

    struct Minmaxi {
        pgl::intv4 mn;
        pgl::intv4 mx;
    };

    struct Ray {
        pgl::intv idx;
        pgl::intv prev;
        pgl::intv texel;
        pgl::intv actived;
        pgl::floatv4 origin;
        pgl::floatv4 direct;
        pgl::floatv4 color;
        pgl::floatv4 final;
        pgl::intv4 params;
        pgl::intv4 extended;
    };

    struct Hit {
        pgl::floatv4 normal;
        pgl::floatv4 tangent;
        pgl::floatv2 texcoord;
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
        pgl::floatv2 sceneRes;
        pgl::intv samplecount;
        pgl::intv rayCount;

        pgl::intv iteration;
        pgl::intv phase;

        pgl::intv maxSamples;
        pgl::intv currentSample;
        pgl::intv maxFilters;
        pgl::intv currentRayLimit;
    };

    struct LightUniformStruct {
        pgl::floatv4 lightVector;
        pgl::floatv4 lightColor;
        pgl::floatv lightRadius;
        pgl::floatv lightDistance;
    };

    struct GeometryUniformStruct {
        glm::mat4 transform;
        glm::mat4 transformInv;
        pgl::floatv4 colormod = pgl::floatv4(0.0f);
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
        pgl::floatv4 eye;
        pgl::floatv4 view;
        pgl::floatv prob;
        pgl::intv enable360;
    };

    struct bbox
    {
        pgl::floatv4 pmin;
        pgl::floatv4 pmax;
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