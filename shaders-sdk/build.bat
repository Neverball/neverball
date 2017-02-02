
cd %~dp0
set CFLAGS=--target-env=opengl -x glsl -Werror -Os -E 
set INDIR=./
set OUTDIR=../shaders/
set VXL2=tools/
set RNDR=render/
set HLBV=hlbvh/

set CMPPROF=-fshader-stage=compute
set FRGPROF=-fshader-stage=fragment
set VRTPROF=-fshader-stage=vertex
set GMTPROF=-fshader-stage=geometry

call glslc %INDIR%%VXL2%loader.comp        %CMPPROF% %CFLAGS% -o %OUTDIR%%VXL2%loader.comp
call glslc %INDIR%%RNDR%render.frag        %FRGPROF% %CFLAGS% -o %OUTDIR%%RNDR%render.frag
call glslc %INDIR%%RNDR%render.vert        %VRTPROF% %CFLAGS% -o %OUTDIR%%RNDR%render.vert
call glslc %INDIR%%RNDR%begin.comp         %CMPPROF% %CFLAGS% -o %OUTDIR%%RNDR%begin.comp
call glslc %INDIR%%RNDR%camera.comp        %CMPPROF% %CFLAGS% -o %OUTDIR%%RNDR%camera.comp
call glslc %INDIR%%RNDR%close.comp         %CMPPROF% %CFLAGS% -o %OUTDIR%%RNDR%close.comp
call glslc %INDIR%%RNDR%clear.comp         %CMPPROF% %CFLAGS% -o %OUTDIR%%RNDR%clear.comp
call glslc %INDIR%%RNDR%reclaim.comp       %CMPPROF% %CFLAGS% -o %OUTDIR%%RNDR%reclaim.comp
call glslc %INDIR%%RNDR%resort.comp        %CMPPROF% %CFLAGS% -o %OUTDIR%%RNDR%resort.comp
call glslc %INDIR%%RNDR%sampler.comp       %CMPPROF% %CFLAGS% -o %OUTDIR%%RNDR%sampler.comp
call glslc %INDIR%%RNDR%testmat.comp       %CMPPROF% %CFLAGS% -o %OUTDIR%%RNDR%testmat.comp
call glslc %INDIR%%RNDR%filter.comp        %CMPPROF% %CFLAGS% -o %OUTDIR%%RNDR%filter.comp
call glslc %INDIR%%RNDR%intersection.comp  %CMPPROF% %CFLAGS% -o %OUTDIR%%RNDR%intersection.comp

call glslc %INDIR%%HLBV%aabbmaker.comp     %FRGPROF% %CFLAGS% -o %OUTDIR%%HLBV%aabbmaker.comp
call glslc %INDIR%%HLBV%build.comp         %FRGPROF% %CFLAGS% -o %OUTDIR%%HLBV%build.comp
call glslc %INDIR%%HLBV%flag.comp          %FRGPROF% %CFLAGS% -o %OUTDIR%%HLBV%flag.comp
call glslc %INDIR%%HLBV%minmax.comp        %FRGPROF% %CFLAGS% -o %OUTDIR%%HLBV%minmax.comp
call glslc %INDIR%%HLBV%refit.comp         %FRGPROF% %CFLAGS% -o %OUTDIR%%HLBV%refit.comp
call glslc %INDIR%%HLBV%resort.comp        %FRGPROF% %CFLAGS% -o %OUTDIR%%HLBV%resort.comp

pause