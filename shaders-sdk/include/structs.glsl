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
    return vec2(a.x, a.y);
}

vec3 toVec3(in Stride3f a){
    return vec3(a.x, a.y, a.z);
}

vec4 toVec4(in Stride4f a){
    return vec4(a.x, a.y, a.z, a.w);
}

ivec3 toVec3(in Stride3i a){
    return ivec3(a.x, a.y, a.z);
}

Stride2f toStride2(in vec2 a){
    Stride2f o;
    o.x = a.x;
    o.y = a.y;
    return o;
}

Stride3f toStride3(in vec3 a){
    Stride3f o;
    o.x = a.x;
    o.y = a.y;
    o.z = a.z;
    return o;
}

Stride4f toStride4(in vec4 a){
    Stride4f o;
    o.x = a.x;
    o.y = a.y;
    o.z = a.z;
    o.w = a.w;
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
