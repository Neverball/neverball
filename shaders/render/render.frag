#version 450
#extension GL_GOOGLE_include_directive : enable
#line 1 "./render/render.frag"


#extension GL_ARB_separate_shader_objects : require
#extension GL_ARB_shading_language_420pack : require

#line 1 "./render/../include/structs.glsl"
struct Minmax {
    vec4 mn;
    vec4 mx;
};

struct Thash {
    int triangle;
    int previous;
};

struct Voxel {
    int depthLeaf;
    int parent;
    int last;
    int suboff;
    int coord;
    int count;
    int _t0;
};

struct Ray {
    int idx;
    int prev;
    int texel;
    int actived;
    vec4 origin;
    vec4 direct;
    vec4 color;
    vec4 final;
    ivec4 params;
    ivec4 extended;
};

struct Hit {
    vec4 normal;
    vec4 tangent;
    vec2 texcoord;
    float dist;
    float _t0;
    int triangle;
    int materialID;
    int shaded;
    int _t2;
};

struct Texel {
    int last;
    int count;
    float x;
    float y;
};

struct Stride2f {
    float x;
    float y;
};

struct Stride3f {
    float x;
    float y;
    float z;
};

struct Stride3i {
    int x;
    int y;
    int z;
};

struct Stride4f {
    float x;
    float y;
    float z;
    float w;
};

vec2 toVec2(in Stride2f a){
    return vec2(a . x, a . y);
}

vec3 toVec3(in Stride3f a){
    return vec3(a . x, a . y, a . z);
}

vec4 toVec4(in Stride4f a){
    return vec4(a . x, a . y, a . z, a . w);
}

ivec3 toVec3(in Stride3i a){
    return ivec3(a . x, a . y, a . z);
}

Stride2f toStride2(in vec2 a){
    Stride2f o;
    o . x = a . x;
    o . y = a . y;
    return o;
}

Stride3f toStride3(in vec3 a){
    Stride3f o;
    o . x = a . x;
    o . y = a . y;
    o . z = a . z;
    return o;
}

Stride4f toStride4(in vec4 a){
    Stride4f o;
    o . x = a . x;
    o . y = a . y;
    o . z = a . z;
    o . w = a . w;
    return o;
}

struct bbox {
    vec4 pmin;
    vec4 pmax;
};

struct Leaf {
    bbox box;
    int parent;
    int _t0;
    int _t1;
    int _t2;
};

struct HlbvhNode {
    bbox box;
    ivec2 range;
    int parent;
    int left;
    int right;
    int _t0;
    int _t1;
    int _t2;
};
#line 7 "./render/render.frag"
#line 1 "./render/../include/constants.glsl"
const int LONGEST = 0xFFFFFFFF;
const float INFINITY = 10000.0f;
const float GAMMA = 1.0f;
const float PI = 3.1415926535897932384626422832795028841971f;
const float TWO_PI = 6.2831853071795864769252867665590057683943f;
const float SQRT_OF_ONE_THIRD = 0.5773502691896257645091487805019574556476f;
const float E = 2.7182818284590452353602874713526624977572f;


#line 8 "./render/render.frag"
#line 1 "./render/../include/uniforms.glsl"
struct RandomUniformStruct {
    float time;
};

struct MinmaxUniformStruct {
    int heap;
    float prec;
};

struct MaterialUniformStruct {
    int materialID;
    int doubleSide;
    float reflectivity;
    float dissolve;
    float illumPower;
    float ior;
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
};

struct CameraUniformStruct {
    mat4 projInv;
    mat4 camInv;
    mat4 camInv2;
    vec4 eye;
    vec4 view;
    float prob;
    int enable360;
};

layout(std140, binding = 0)uniform ubo0 { RandomUniformStruct randomUniform;};
layout(std140, binding = 1)uniform ubo1 { MinmaxUniformStruct minmaxUniform;};
layout(std140, binding = 2)uniform ubo2 { MaterialUniformStruct materialUniform;};
layout(std140, binding = 3)uniform ubo3 { SamplerUniformStruct samplerUniform;};
layout(std140, binding = 4)uniform ubo4 { LightUniformStruct lightUniform;};
layout(std140, binding = 5)uniform ubo5 { GeometryUniformStruct geometryUniform;};
layout(std140, binding = 6)uniform ubo6 { HelperUniformStruct helperUniform;};
layout(std140, binding = 7)uniform ubo7 { OctreeUniformStruct octreeUniform;};
layout(std140, binding = 8)uniform ubo8 { CameraUniformStruct cameraUniform;};
#line 9 "./render/render.frag"
#line 1 "./render/../include/fast_math.glsl"
int pow2(in int d){
    return d <= 0 ? 1 :(2 <<(d - 1));
}

float pow2(in float d){
    return exp2(d);
}
#line 10 "./render/render.frag"
#line 1 "./render/../include/rays.glsl"
 layout(std430, binding = 8)buffer ssbo8 { Ray rays[];};
layout(std430, binding = 9)buffer ssbo9 { Hit hits[];};
layout(std430, binding = 10)buffer ssbo10 { Texel texelInfo[];};
layout(std430, binding = 13)buffer ssbo13 { int actived[];};
layout(std430, binding = 14)buffer ssbo14 { int qrays[];};
layout(binding = 0)uniform atomic_uint acounter;
layout(binding = 0, offset = 4)uniform atomic_uint rcounter;
layout(binding = 0, offset = 8)uniform atomic_uint qcounter;

void storeHit(in int hitIndex, inout Hit hit){
    hits[hitIndex]= hit;
}

void storeHit(inout Ray ray, inout Hit hit){
    storeHit(ray . idx, hit);
}

void storeRay(in int rayIndex, inout Ray ray){
    ray . idx = rayIndex;
    rays[rayIndex]= ray;
}

void storeRay(in Ray current, inout Ray ray){
    storeRay(current . prev, ray);
}

void storeRay(inout Ray ray){
    storeRay(ray . idx, ray);
}

void restoreRay(inout Ray ray){
    if(ray . actived < 1){
        ray . actived = 1;
        rays[ray . idx]= ray;
        uint act = atomicCounterIncrement(acounter);
        actived[act]= ray . idx;
    }
}

int createRayStrict(inout Ray original, in int idx, in int rayIndex){
    int prev = atomicExchange(texelInfo[idx]. last, rayIndex);
    atomicAdd(texelInfo[idx]. count, 1);

    Ray ray = original;
    ray . idx = rayIndex;
    ray . prev = prev;
    ray . texel = idx;

    Hit hit;
    if(original . idx != LONGEST){
        hit = hits[original . idx];
    } else {
        hit . triangle = LONGEST;
        hit . normal = vec4(0.0f);
        hit . tangent = vec4(0.0f);
        hit . materialID = LONGEST;
        hit . dist = INFINITY;
        hit . shaded = 0;
    }

    hits[rayIndex]= hit;
    rays[rayIndex]= ray;

    if(ray . actived > 0){
        uint act = atomicCounterIncrement(acounter);
        actived[act]= rayIndex;

    }
    return rayIndex;
}

int createRayStrict(inout Ray original, in int rayIndex){
    return createRayStrict(original, original . texel, rayIndex);
}

int createRayAccount(inout Ray original, in int idx, in int rayIndex){
    atomicCounterIncrement(rcounter);
    return createRayStrict(original, idx, rayIndex);
}

int createRay(inout Ray original, in int idx){
    int rayIndex = int(atomicCounterIncrement(rcounter));
    return createRayStrict(original, idx, rayIndex);
}

int createRay(in Ray original){
    return createRay(original, original . texel);
}

int createRay(in int idx){
    Ray newRay;
    return createRay(newRay, idx);
}

Ray fetchRayDirect(in int texel){
    return rays[texel];
}

Hit fetchHitDirect(in int texel){
    return hits[texel];
}

int fetchRayIdx(in int texel){
    return texelInfo[texel]. last;
}

int fetchHitIdx(in int texel){
    return texelInfo[texel]. last;
}

Ray fetchRay(in int texel){
    return fetchRayDirect(fetchRayIdx(texel));
}

Hit fetchHit(in int texel){
    return fetchHitDirect(fetchHitIdx(texel));
}

Hit fetchHit(in Ray ray){
    return hits[ray . idx];
}

Ray nextRay(in Ray ray){
    return rays[ray . prev];
}

int getRayCount(in int texel){
    return texelInfo[texel]. count;
}

bool haveChain(in Ray ray){
    return ray . prev != LONGEST;
}
#line 11 "./render/render.frag"
#line 1 "./render/../include/random.glsl"
 float counter = 0.0f;
int globalInvocationSMP = 0;

int hash(int x){
    x +=(x << 10);
    x ^=(x >> 6);
    x +=(x << 3);
    x ^=(x >> 11);
    x +=(x << 15);
    return x;
}

int hash(ivec2 v){ return hash(v . x ^ hash(v . y));}
int hash(ivec3 v){ return hash(v . x ^ hash(v . y)^ hash(v . z));}
int hash(ivec4 v){ return hash(v . x ^ hash(v . y)^ hash(v . z)^ hash(v . w));}

float floatConstruct(int m){
    const int ieeeMantissa = 0x007FFFFF;
    const int ieeeOne = 0x3F800000;
    m &= ieeeMantissa;
    m |= ieeeOne;
    float f = intBitsToFloat(m);
    return f - 1.0f;
}

float random(float x){ return floatConstruct(hash(floatBitsToInt(x)));}
float random(vec2 v){ return floatConstruct(hash(floatBitsToInt(v)));}
float random(vec3 v){ return floatConstruct(hash(floatBitsToInt(v)));}
float random(vec4 v){ return floatConstruct(hash(floatBitsToInt(v)));}

float random(){
    float x = float(globalInvocationSMP);
    float y = 10.0f + counter + randomUniform . time;
    float r = random(vec2(x, y));
    counter += 1.0f;
    return r;
}

vec3 randomCosine(in vec3 normal){
    float up = sqrt(random());
    float over = sqrt(- fma(up, up, - 1.0f));
    float around = random()* TWO_PI;

    vec3 directionNotNormal;
    if(abs(normal . x)< SQRT_OF_ONE_THIRD){
        directionNotNormal = vec3(1, 0, 0);
    } else if(abs(normal . y)< SQRT_OF_ONE_THIRD){
        directionNotNormal = vec3(0, 1, 0);
    } else {
        directionNotNormal = vec3(0, 0, 1);
    }
    vec3 perpendicular1 = normalize(cross(normal, directionNotNormal));
    vec3 perpendicular2 = cross(normal, perpendicular1);
    return normalize(
        fma(normal, vec3(up),
            fma(perpendicular1, vec3(cos(around))* over,
                 perpendicular2 * vec3(sin(around))* over
            )
        )
    );
}

vec3 randomDirectionInSphere(){
    float up = random()* 2.0f - 1.0f;
    float over = sqrt(1.0f - up * up);
    float around = random()* TWO_PI;
    return normalize(vec3(up, cos(around)* over, sin(around)* over));
}
#line 12 "./render/render.frag"

layout(location = 0)out vec4 outFragColor;
layout(location = 0)uniform sampler2D samples;

uniform vec2 viewport;
in vec2 texcoord;

void main(){
    vec2 screensizep = textureSize(samples, 0);
    vec2 samplesizep = ivec2(samplerUniform . sceneRes . xy);

    ivec2 screencoord = ivec2(texcoord * textureSize(samples, 0));
    vec2 rates = screensizep / samplesizep;
    ivec2 samplecount = ivec2(ceil(rates));

    float empty = 0.0f;
    vec4 color = texture(samples, texcoord);
    outFragColor = vec4(pow(clamp(color . xyz, vec3(0.0f), vec3(1.0f)), vec3(1.0f / GAMMA)), color . w);

}