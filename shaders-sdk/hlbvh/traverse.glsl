layout( std430, binding = 0 ) buffer NodesBlock {
    HlbvhNode Nodes[];
};

layout( std430, binding = 1 ) buffer MortoncodesBlock {
    uint Mortoncodes[];
};

layout( std430, binding = 2 ) buffer IndicesBlock {
    int MortoncodesIndices[];
};

#define LEAFNODE(x) (((x).left) == ((x).right))

TResult testIntersection(inout TResult res, in vec3 orig, in vec3 dir, in int tri) {
    if (tri != LONGEST && res.triangle != tri && tri >=0 && tri < geometryUniform.triangleCount) {
        vec3 triverts[3];
        for (int x=0;x<3;x++) {
            int j = indics[tri * 3 + x];
            triverts[x] = toVec3(verts[j]);
        }
        vec2 uv = vec2(0.0f);
        vec3 n = vec3(0.0f);
        float _d = intersectTriangle(orig, dir, triverts, uv, n);
        if (_d < INFINITY && _d >= 0.0f && _d <= res.dist) {
            res.dist = _d;
            res.triangle = tri;
            res.uv = uv;
            res.normal = n;
        }
    }
    return res;
}

vec3 projectVoxels(in vec3 orig) {
    return (octreeUniform.project * vec4(orig, 1.0f)).xyz;
}

vec3 unprojectVoxels(in vec3 orig) {
    return (octreeUniform.unproject * vec4(orig, 1.0f)).xyz;
}

float intersectCubeSingle(in vec3 origin, in vec3 ray, in vec3 cubeMin, in vec3 cubeMax, inout float near) {
    vec3 dr = vec3(1.0f) / ray;
    vec3 tMin = (cubeMin - origin) * dr;
    vec3 tMax = (cubeMax - origin) * dr;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar  = min(min(t2.x, t2.y), t2.z);
    float t = (tFar > tNear && tFar > 0.f) ? (tNear > 0.f ? min(tNear, tFar) : max(tNear, tFar)) : INFINITY;
    near    = (tFar > tNear) ? max(tNear, 0.f) : INFINITY;
    return t > 0.f ? t : INFINITY;
}

const float precerr = 0.0001f;




const int leftMask = 1;
const int rightMask = 2;
const int leftNearMask = 4;
const int unknownMask = 8;

bool ism(in int bits, in int mask){
    return bits >= 0 ? ((bits & mask) > 0) : false;
}

const int STACK_SIZE = 32;
shared int stack[256][32];
//shared int ptr[256];

TResult traverse(in float distn, in vec3 origin, in vec3 direct){
    TResult lastRes;
    lastRes.triangle = LONGEST;
    lastRes.dist = distn;
    lastRes.materialID = LONGEST;
    float prevdist = distn;
    
    vec3 torig = projectVoxels(origin);
    vec3 dirproj = (vec4(direct, 0.0) * octreeUniform.project).xyz;
    float dirlen = length(direct) / length(dirproj);
    dirproj = normalize(dirproj);
    float near = 0.0f;
    float d = intersectCubeSingle(torig, dirproj, vec3(0.0f), vec3(1.0f), near);
    float dst = near * dirlen;

    const uint l = gl_LocalInvocationID.x;
    
    if (
        (d > 0.0f && d < INFINITY) && 
        (lastRes.dist > 0.0f) && 
        (lastRes.dist >= dst)
    ) {
        int ptr = 0;
        stack[l][ptr] = -1;
        
        HlbvhNode node;
        int idx = 0;
        
        for(int i=0;i<2048;i++) {
            if (lastRes.dist <= 0.0f) break;
            node = Nodes[idx];
            
            if (LEAFNODE(node)) {
                float cecheddist = lastRes.dist;
                testIntersection(lastRes, origin, direct, MortoncodesIndices[node.left]);
                if (lastRes.dist < cecheddist) {
                    prevdist = cecheddist;
                }
            } else {
                bool leftOverlap = false, rightOverlap = false, selfOverlap = false;
                float lefthit = 0.0f, righthit = 0.0f;

                // check if nearest changed
                selfOverlap = prevdist > lastRes.dist ? false : true;

                // check for nearest box before testing
                if (!selfOverlap) {
                    float near = 0.0f;
                    bbox lbox = node.box;
                    float selfhit  = intersectCubeSingle(torig, dirproj, lbox.pmin.xyz, lbox.pmax.xyz, near);
                    selfOverlap = (  selfhit < INFINITY && selfhit  > 0.0f) && (near * dirlen < lastRes.dist);
                }
                
                if (selfOverlap) {
                    {
                        float near = 0.0f;
                        bbox lbox = Nodes[node.left].box;
                        lefthit  = intersectCubeSingle(torig, dirproj, lbox.pmin.xyz, lbox.pmax.xyz, near);
                        leftOverlap = (  lefthit < INFINITY && lefthit  > 0.0f) && (near * dirlen < lastRes.dist);
                    }
                    
                    {
                        float near = 0.0f;
                        bbox rbox = Nodes[node.right].box;
                        righthit = intersectCubeSingle(torig, dirproj, rbox.pmin.xyz, rbox.pmax.xyz, near);
                        rightOverlap = (righthit < INFINITY && righthit > 0.0f) && (near * dirlen < lastRes.dist);
                    }
                }
                
                if (leftOverlap && rightOverlap) {
                    bool leftNear = lefthit <= righthit;
                    if (ptr < STACK_SIZE) {
                        stack[l][ptr++] = leftNear ? node.right : node.left;
                    }
                    idx = leftNear ? node.left : node.right;
                    continue;
                } else 
                if (leftOverlap || rightOverlap) {
                    idx = leftOverlap ? node.left : node.right;
                    continue;
                }
                
            }
            
            {
                int pt = --ptr;
                if (pt >= 0 && pt < STACK_SIZE) {
                    idx = stack[l][pt];
                    stack[l][pt] = -1;
                }
                if ( idx < 0 || pt < 0 ) break;
            }
        }
    }
    return loadInfo(lastRes);
}
