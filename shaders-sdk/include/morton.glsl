uint part1By2_64(in uint a){
    uint x = a & 0x000003ff;
    x = (x | (x << 16)) & 0xff0000ff;
    x = (x | (x <<  8)) & 0x0300f00f;
    x = (x | (x <<  4)) & 0x030c30c3;
    x = (x | (x <<  2)) & 0x09249249;
    return x;
}

uint encodeMorton3_64(in uvec3 a)
{
    return part1By2_64(a.x) | (part1By2_64(a.y) << 1) | (part1By2_64(a.z) << 2);
}

uint compact1By2_64(in uint a)
{
    uint x = a & 0x09249249;
    x = (x | (x >>  2)) & 0x030c30c3;
    x = (x | (x >>  4)) & 0x0300f00f;
    x = (x | (x >>  8)) & 0xff0000ff;
    x = (x | (x >> 16)) & 0x000003ff;
    return x;
}

uvec3 decodeMorton3_64(in uint code)
{
    return uvec3(compact1By2_64(code >> 0), compact1By2_64(code >> 1), compact1By2_64(code >> 2));
}

ivec2 decodeDepthLeaf(int dl){
    return ivec2((dl >> 31) & 1, dl & 0x7FFFFFFF);
}

int encodeDepthLeaf(ivec2 dl){
    return ((dl.x & 1) << 31) | (dl.y & 0x7FFFFFFF);
}
