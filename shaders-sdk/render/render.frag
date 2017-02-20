#version 450

#extension GL_ARB_separate_shader_objects : require
#extension GL_ARB_shading_language_420pack : require

#include "../include/structs.glsl"
#include "../include/constants.glsl"
#include "../include/uniforms.glsl"
#include "../include/fast_math.glsl"
#include "../include/rays.glsl"
#include "../include/random.glsl"

layout ( location = 0 ) out vec4 outFragColor;
layout ( location = 0 ) uniform sampler2D samples;

uniform vec2 viewport;
in vec2 texcoord;


#define NEIGHBOURS 8
#define AXES       (NEIGHBOURS/2)
#define POW2(a) ((a)*(a))
#define GEN_METRIC(before, center, after) POW2((center) * vec4(2.0f) - (before) - (after))
#define BAIL_CONDITION(new,original) (lessThan(new, original))
#define SYMMETRY(a)  (NEIGHBOURS - (a) - 1)
#define O(u,v) (ivec2(u, v))

const ivec2 offsets[NEIGHBOURS] = {
    O(-1, -1), O( 0, -1), O( 1, -1),
    O(-1,  0),            O( 1,  0),
    O(-1,  1), O( 0,  1), O( 1,  1)
};

vec4 filtered(in vec2 tx){
    ivec2 center_pix = ivec2(tx * textureSize(samples, 0));
    vec4 center_pix_cache = texelFetch(samples, center_pix, 0);
    
    vec4 metric_reference[AXES];
    for (int axis = 0; axis < AXES; axis++) {
        vec4 before_pix = texelFetch(samples, center_pix + offsets[axis], 0);
        vec4 after_pix  = texelFetch(samples, center_pix + offsets[SYMMETRY(axis)], 0);
        metric_reference[axis] = GEN_METRIC (before_pix, center_pix_cache, after_pix);
    }

    vec4 sum = center_pix_cache;
    vec4 cur = center_pix_cache;
    ivec4 count = ivec4(1);
    
    for (int direction = 0; direction < NEIGHBOURS; direction++) {
        vec4 pix   = texelFetch(samples, center_pix + offsets[direction], 0);
        vec4 value = (pix + cur) * (0.5f);
        ivec4 mask = {1, 1, 1, 0};
        for (int axis = 0; axis < AXES; axis++) {
            vec4 before_pix = texelFetch(samples, center_pix + offsets[axis], 0);
            vec4 after_pix  = texelFetch(samples, center_pix + offsets[SYMMETRY(axis)], 0);
            vec4 metric_new = GEN_METRIC (before_pix, value, after_pix);
            mask = ivec4(BAIL_CONDITION(metric_new, metric_reference[axis])) & mask;
        }
        sum   += mix(vec4(0.0f), value , bvec4(mask));
        count += mix(ivec4(0), ivec4(1), bvec4(mask));
    }
    
    return (sum/vec4(count));
}


void main() {
    vec2 screensizep = textureSize(samples, 0);
    vec2 samplesizep = ivec2(samplerUniform.sceneRes.xy);

    ivec2 screencoord = ivec2(texcoord * textureSize(samples, 0));
    vec2 rates = screensizep / samplesizep;
    ivec2 samplecount = ivec2(ceil(rates));

    float empty = 0.0f;
    vec4 color = filtered(texcoord);//texture(samples, texcoord);
    outFragColor = vec4(pow(clamp(color.xyz, vec3(0.0f), vec3(1.0f)), vec3(1.0f / GAMMA)), 1.0f);
    //outFragColor = vec4(0.5f, 0.0f, 0.0f, 1.0f);
}