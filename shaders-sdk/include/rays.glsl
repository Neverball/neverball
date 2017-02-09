layout ( std430, binding=8) buffer ssbo8 {Ray rays[];};
layout ( std430, binding=9) buffer ssbo9 {Hit hits[];};
layout ( std430, binding=10) buffer ssbo10 {Texel texelInfo[];};
layout ( std430, binding=13) buffer ssbo13 {int actived[];};
layout ( std430, binding=14) buffer ssbo14 {int qrays[];};
layout ( binding=0) uniform atomic_uint acounter;
layout ( binding=0, offset=4) uniform atomic_uint rcounter;
layout ( binding=0, offset=8) uniform atomic_uint qcounter;

void storeHit(in int hitIndex, inout Hit hit) {
    if (hitIndex == -1 || hitIndex == LONGEST || hitIndex >= samplerUniform.currentRayLimit) {
        return;
    }
    hits[hitIndex] = hit;
}

void storeHit(inout Ray ray, inout Hit hit) {
    storeHit(ray.idx, hit);
}

void storeRay(in int rayIndex, inout Ray ray) {
    if (rayIndex == -1 || rayIndex == LONGEST || rayIndex >= samplerUniform.currentRayLimit) {
        return;
    }
    ray.idx = rayIndex;
    rays[rayIndex] = ray;
}

void storeRay(in Ray current, inout Ray ray) {
    storeRay(current.prev, ray);
}

void storeRay(inout Ray ray) {
    storeRay(ray.idx, ray);
}

void restoreRay(inout Ray ray){
    if(ray.actived < 1){
        ray.actived = 1;
        rays[ray.idx] = ray;
        uint act = atomicCounterIncrement(acounter);
        actived[act] = ray.idx;
    }
}

int createRayStrict(inout Ray original, in int idx, in int rayIndex) {
    if (rayIndex == -1 || rayIndex == LONGEST || rayIndex >= samplerUniform.currentRayLimit) {
        return rayIndex;
    }
    
    int prev = atomicExchange(texelInfo[idx].last, rayIndex);
    atomicAdd(texelInfo[idx].count, 1);
    
    Ray ray = original;
    ray.idx = rayIndex;
    ray.prev = prev;
    ray.texel = idx;
    
    Hit hit;
    if (original.idx != LONGEST) {
        hit = hits[original.idx];
    } else {
        hit.triangle = LONGEST;
        hit.normal = vec4(0.0f);
        hit.tangent = vec4(0.0f);
        hit.materialID = LONGEST;
        hit.dist = INFINITY;
        hit.shaded = 0;
    }
    
    hits[rayIndex] = hit;
    rays[rayIndex] = ray;

    if (ray.actived > 0) {
        uint act = atomicCounterIncrement(acounter);
        actived[act] = rayIndex;
        //actived[rayIndex] = rayIndex;
    }
    return rayIndex;
}

int createRayStrict(inout Ray original, in int rayIndex) {
    return createRayStrict(original, original.texel, rayIndex);
}

int createRayAccount(inout Ray original, in int idx, in int rayIndex) {
    atomicCounterIncrement(rcounter);
    return createRayStrict(original, idx, rayIndex);
}

int createRay(inout Ray original, in int idx) {
    int rayIndex = int(atomicCounterIncrement(rcounter));
    return createRayStrict(original, idx, rayIndex);
}

int createRay(in Ray original) {
    return createRay(original, original.texel);
}

int createRay(in int idx) {
    Ray newRay;
    return createRay(newRay, idx);
}

Ray fetchRayDirect(in int texel) {
    return rays[texel];
}

Hit fetchHitDirect(in int texel) {
    return hits[texel];
}

int fetchRayIdx(in int texel) {
    return texelInfo[texel].last;
}

int fetchHitIdx(in int texel) {
    return texelInfo[texel].last;
}

Ray fetchRay(in int texel){
    return fetchRayDirect(fetchRayIdx(texel));
}

Hit fetchHit(in int texel){
    return fetchHitDirect(fetchHitIdx(texel));
}

Hit fetchHit(in Ray ray){
    return hits[ray.idx];
}

Ray nextRay(in Ray ray){
    return rays[ray.prev];
}

int getRayCount(in int texel){
    return texelInfo[texel].count;
}

bool haveChain(in Ray ray){
    return ray.prev != LONGEST;
}
