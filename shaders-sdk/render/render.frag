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

void main() {
    vec2 screensizep = textureSize(samples, 0);
    vec2 samplesizep = ivec2(samplerUniform.sceneRes.xy);

    ivec2 screencoord = ivec2(texcoord * textureSize(samples, 0));
    vec2 rates = screensizep / samplesizep;
    ivec2 samplecount = ivec2(ceil(rates));

    float empty = 0.0f;
    vec4 color = texture(samples, texcoord);
    outFragColor = vec4(pow(clamp(color.xyz, vec3(0.0f), vec3(1.0f)), vec3(1.0f / GAMMA)), color.w);
    //outFragColor = vec4(0.5f, 0.0f, 0.0f, 1.0f);
}