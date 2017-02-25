#!/bin/sh

CFLAGS="--target-env=opengl -x glsl -Werror -Os -E" 
INDIR=./
OUTDIR=../shaders/
VXL2=tools/
RNDR=render/
HLBV=hlbvh/

CMPPROF=-fshader-stage=compute
FRGPROF=-fshader-stage=fragment
VRTPROF=-fshader-stage=vertex
GMTPROF=-fshader-stage=geometry

glslc ${INDIR}${VXL2}loader.comp        ${CMPPROF} ${CFLAGS} -o ${OUTDIR}${VXL2}loader.comp
glslc ${INDIR}${RNDR}render.frag        ${FRGPROF} ${CFLAGS} -o ${OUTDIR}${RNDR}render.frag
glslc ${INDIR}${RNDR}render.vert        ${VRTPROF} ${CFLAGS} -o ${OUTDIR}${RNDR}render.vert
glslc ${INDIR}${RNDR}begin.comp         ${CMPPROF} ${CFLAGS} -o ${OUTDIR}${RNDR}begin.comp
glslc ${INDIR}${RNDR}camera.comp        ${CMPPROF} ${CFLAGS} -o ${OUTDIR}${RNDR}camera.comp
glslc ${INDIR}${RNDR}close.comp         ${CMPPROF} ${CFLAGS} -o ${OUTDIR}${RNDR}close.comp
glslc ${INDIR}${RNDR}clear.comp         ${CMPPROF} ${CFLAGS} -o ${OUTDIR}${RNDR}clear.comp
glslc ${INDIR}${RNDR}reclaim.comp       ${CMPPROF} ${CFLAGS} -o ${OUTDIR}${RNDR}reclaim.comp
glslc ${INDIR}${RNDR}resort.comp        ${CMPPROF} ${CFLAGS} -o ${OUTDIR}${RNDR}resort.comp
glslc ${INDIR}${RNDR}sampler.comp       ${CMPPROF} ${CFLAGS} -o ${OUTDIR}${RNDR}sampler.comp
glslc ${INDIR}${RNDR}testmat.comp       ${CMPPROF} ${CFLAGS} -o ${OUTDIR}${RNDR}testmat.comp
glslc ${INDIR}${RNDR}filter.comp        ${CMPPROF} ${CFLAGS} -o ${OUTDIR}${RNDR}filter.comp
glslc ${INDIR}${RNDR}intersection.comp  ${CMPPROF} ${CFLAGS} -o ${OUTDIR}${RNDR}intersection.comp

glslc ${INDIR}${HLBV}aabbmaker.comp     ${FRGPROF} ${CFLAGS} -o ${OUTDIR}${HLBV}aabbmaker.comp
glslc ${INDIR}${HLBV}build.comp         ${FRGPROF} ${CFLAGS} -o ${OUTDIR}${HLBV}build.comp
glslc ${INDIR}${HLBV}flag.comp          ${FRGPROF} ${CFLAGS} -o ${OUTDIR}${HLBV}flag.comp
glslc ${INDIR}${HLBV}minmax.comp        ${FRGPROF} ${CFLAGS} -o ${OUTDIR}${HLBV}minmax.comp
glslc ${INDIR}${HLBV}refit.comp         ${FRGPROF} ${CFLAGS} -o ${OUTDIR}${HLBV}refit.comp
glslc ${INDIR}${HLBV}resort.comp        ${FRGPROF} ${CFLAGS} -o ${OUTDIR}${HLBV}resort.comp

