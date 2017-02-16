struct RandomUniformStruct {
    float time;
};

struct MinmaxUniformStruct {
    int heap;
    float prec;
};

struct MaterialUniformStruct {
/*
    int materialID;
    int doubleSide;
    float reflectivity;
    float dissolve;
    float illumPower;
    float ior;
    */
    
    int materialID;
    int f_shadows;
    int f_reflections;
    int unk0;
};

struct SamplerUniformStruct {
    vec2 sceneRes;
    int samplecount;
    int rayCount;
    
    int iteration;
    int phase;
    
    int maxSamples;
    int currentSample;
    int maxFilters;
    int currentRayLimit;
};

struct LightUniformStruct {
    vec4 lightVector;
    vec4 lightColor;
    float lightRadius;
    float lightDistance;
};

struct GeometryUniformStruct {
    mat4 transform;
    mat4 transformInv;
    vec4 colormod;
    float offset;
    
    int materialID;
    int triangleCount;
    int triangleOffset;
    int unindexed;
    int loadOffset;
    int NB_mode;
    int unk1;
};

struct HelperUniformStruct {
    ivec4 range;
};

struct OctreeUniformStruct {
    mat4 project;
    mat4 unproject;
    int maxDepth;
    int currentDepth;
    int nodeCount;
    int unk0;
};

struct CameraUniformStruct {
    mat4 projInv;
    mat4 camInv;
    mat4 camInv2;
    vec4 eye;
    vec4 view;
    float prob;
    int enable360;
    int unk0;
    int unk1;
};

layout ( std140, binding=0) uniform ubo0 {RandomUniformStruct randomUniform;};
layout ( std140, binding=1) uniform ubo1 {MinmaxUniformStruct minmaxUniform;};
layout ( std140, binding=2) uniform ubo2 {MaterialUniformStruct materialUniform;};
layout ( std140, binding=3) uniform ubo3 {SamplerUniformStruct samplerUniform;};
layout ( std140, binding=4) uniform ubo4 {LightUniformStruct lightUniform;};
layout ( std140, binding=5) uniform ubo5 {GeometryUniformStruct geometryUniform;};
layout ( std140, binding=6) uniform ubo6 {HelperUniformStruct helperUniform;};
layout ( std140, binding=7) uniform ubo7 {OctreeUniformStruct octreeUniform;};
layout ( std140, binding=8) uniform ubo8 {CameraUniformStruct cameraUniform;};
