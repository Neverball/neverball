/*
 * Copyright (C) 2013 Robert Kooima
 *
 * NEVERBALL is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

#include <OVR.h>

extern "C"
{
    #include "glext.h"
    #include "glsl.h"
    #include "fbo.h"
}

/*---------------------------------------------------------------------------*/

static OVR::Ptr<OVR::DeviceManager> pManager;
static OVR::Ptr<OVR::HMDDevice>     pHMD;
static OVR::Ptr<OVR::SensorDevice>  pSensor;

static OVR::HMDInfo      Info;
static OVR::SensorFusion Fusion;

static OVR::Util::Render::StereoConfig Stereo;

static int eye = 0;

/*---------------------------------------------------------------------------*/

static fbo L_fbo;
static fbo R_fbo;

static glsl L_glsl;
static glsl R_glsl;

static GLuint L_vbo = 0;
static GLuint R_vbo = 0;

struct point
{
    GLfloat x;
    GLfloat y;
    GLfloat u;
    GLfloat v;
};

static const struct point L_rect[4] = {
    { -1, -1,  0,  0 }, {  0, -1,  1,  0 },
    { -1,  1,  0,  1 }, {  0,  1,  1,  1 }
};
static const struct point R_rect[4] = {
    {  0, -1,  0,  0 }, {  1, -1,  1,  0 },
    {  0,  1,  0,  1 }, {  1,  1,  1,  1 }
};

static const char *hmd_vert[] = {
    "void main()\n",
    "{\n",
        "gl_TexCoord[0] = gl_MultiTexCoord0;\n",
        "gl_Position    = ftransform();\n",
    "}\n",
};

static const char *hmd_frag[] = {
    "#version 120\n",

    "uniform sampler2D warpTexture;\n",
    "uniform vec2      LensCenter;\n",
    "uniform vec4      DistortionK;\n",
    "uniform vec2      ScaleIn;\n",
    "uniform vec2      ScaleOut;\n",

    "vec2 HMDWarp(vec2 c)\n",
    "{\n",
        "vec2 theta = (c - LensCenter) * ScaleIn;\n",
        "float rSq = theta.x * theta.x + theta.y * theta.y;\n",
        "vec2 rvector = theta * (DistortionK.x + "
                                "DistortionK.y * rSq + "
                                "DistortionK.z * rSq * rSq + "
                                "DistortionK.w * rSq * rSq * rSq);\n",
        "return LensCenter + ScaleOut * rvector;\n",
    "}\n",

    "void main()\n",
    "{\n",
         "vec2 c = HMDWarp(gl_TexCoord[0].xy);\n",
         "vec2 b = step(vec2(0.0), c) * step(c, vec2(1.0));\n",
         "gl_FragColor = b.x * b.y * texture2D(warpTexture, c);\n",
    "}\n",
};

static void hmd_gl_init()
{
    /* Create the off-screen frame buffers. */

    fbo_create(&L_fbo, Info.HResolution / 2, Info.VResolution);
    fbo_create(&R_fbo, Info.HResolution / 2, Info.VResolution);

    /* Create and initialize the distortion shader. */

    float d = 1 - (2 * Info.LensSeparationDistance) / Info.HScreenSize;
    float s = 1 / Stereo.GetDistortionScale();
    float a = Info.HResolution / 2.0f / Info.VResolution;

    glsl_create(&L_glsl, sizeof (hmd_vert) / sizeof (char *), hmd_vert,
                         sizeof (hmd_frag) / sizeof (char *), hmd_frag);
    glsl_create(&R_glsl, sizeof (hmd_vert) / sizeof (char *), hmd_vert,
                         sizeof (hmd_frag) / sizeof (char *), hmd_frag);

    glUseProgram_  (L_glsl.program);
    glsl_uniform2f(&L_glsl, "LensCenter", 0.5 + 0.5f * d, 0.5);
    glsl_uniform4f(&L_glsl, "DistortionK", Info.DistortionK[0],
                                           Info.DistortionK[1],
                                           Info.DistortionK[2],
                                           Info.DistortionK[3]);
    glsl_uniform2f(&L_glsl, "ScaleOut", 0.5f * s, 0.5f * s * a);
    glsl_uniform2f(&L_glsl, "ScaleIn",  2.0f,     2.0f     / a);


    glUseProgram_  (R_glsl.program);
    glsl_uniform2f(&R_glsl, "LensCenter", 0.5 - 0.5f * d, 0.5);
    glsl_uniform4f(&R_glsl, "DistortionK", Info.DistortionK[0],
                                           Info.DistortionK[1],
                                           Info.DistortionK[2],
                                           Info.DistortionK[3]);
    glsl_uniform2f(&L_glsl, "ScaleOut", 0.5f * s, 0.5f * s * a);
    glsl_uniform2f(&L_glsl, "ScaleIn",  2.0f,     2.0f     / a);

    glUseProgram_(0);

    /* Initialize VBOs for the on-screen rectangles. */

    glGenBuffers_(1, &L_vbo);
    glBindBuffer_(GL_ARRAY_BUFFER, L_vbo);
    glBufferData_(GL_ARRAY_BUFFER, sizeof (L_rect), L_rect, GL_STATIC_DRAW);
    glGenBuffers_(1, &R_vbo);
    glBindBuffer_(GL_ARRAY_BUFFER, R_vbo);
    glBufferData_(GL_ARRAY_BUFFER, sizeof (R_rect), R_rect, GL_STATIC_DRAW);
    glBindBuffer_(GL_ARRAY_BUFFER, 0);
}

static void hmd_gl_draw(GLuint program, GLuint texture, GLuint buffer)
{
    glUseProgram_(program);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindBuffer_(GL_ARRAY_BUFFER, buffer);
    glVertexPointer  (2, GL_FLOAT, sizeof (struct point), (GLvoid *) 0);
    glTexCoordPointer(2, GL_FLOAT, sizeof (struct point), (GLvoid *) 8);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void hmd_gl_free()
{
    fbo_delete(&L_fbo);
    fbo_delete(&R_fbo);

    glsl_delete(&L_glsl);
    glsl_delete(&R_glsl);

    if (L_vbo) glDeleteBuffers_(1, &L_vbo);
    if (R_vbo) glDeleteBuffers_(1, &R_vbo);
}

static void hmd_gl_matrix(OVR::Matrix4f& M)
{
    GLdouble T[16];

    T[ 0] = (GLdouble) M.M[0][0];
    T[ 1] = (GLdouble) M.M[1][0];
    T[ 2] = (GLdouble) M.M[2][0];
    T[ 3] = (GLdouble) M.M[3][0];
    T[ 4] = (GLdouble) M.M[0][1];
    T[ 5] = (GLdouble) M.M[1][1];
    T[ 6] = (GLdouble) M.M[2][1];
    T[ 7] = (GLdouble) M.M[3][1];
    T[ 8] = (GLdouble) M.M[0][2];
    T[ 9] = (GLdouble) M.M[1][2];
    T[10] = (GLdouble) M.M[2][2];
    T[11] = (GLdouble) M.M[3][2];
    T[12] = (GLdouble) M.M[0][3];
    T[13] = (GLdouble) M.M[1][3];
    T[14] = (GLdouble) M.M[2][3];
    T[15] = (GLdouble) M.M[3][3];

    glLoadMatrixd(T);
}

/*---------------------------------------------------------------------------*/

extern "C" int hmd_stat()
{
    return OVR::System::IsInitialized() ? 1 : 0;
}

extern "C" void hmd_init()
{
    OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));

    if (OVR::System::IsInitialized())
    {
        if ((pManager = *OVR::DeviceManager::Create()))
        {
            if ((pHMD = *pManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice()))
            {
                pHMD->GetDeviceInfo(&Info);

                if ((pSensor = *pHMD->GetSensor()))
                {
                    Fusion.AttachToSensor(pSensor);

                    Stereo.SetFullViewport(OVR::Util::Render::Viewport(0, 0, Info.HResolution, Info.VResolution));
                    Stereo.SetStereoMode(OVR::Util::Render::Stereo_LeftRight_Multipass);
                    Stereo.SetHMDInfo(Info);
                    Stereo.SetDistortionFitPointVP(-1.0f, 0.0f);

                    hmd_gl_init();
                }
            }
        }
    }
}

extern "C" void hmd_free()
{
    hmd_gl_free();
    // OVR::System::Destroy();
}

extern "C" void hmd_step()
{
}

extern "C" void hmd_swap()
{
    if (hmd_stat())
    {
        /* Prepare to draw a screen-filling pair of rectangles. */

        glBindFramebuffer_(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, Info.HResolution, Info.VResolution);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glDisable(GL_BLEND);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        /* Draw the left and right eyes, distorted, to the screen. */

        hmd_gl_draw(L_glsl.program, L_fbo.color_texture, L_vbo);
        hmd_gl_draw(R_glsl.program, R_fbo.color_texture, R_vbo);

        /* Revert the state. */

        glBindBuffer_(GL_ARRAY_BUFFER, 0);
        glUseProgram_(0);
        glEnable(GL_BLEND);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
}

extern "C" void hmd_prep_left()
{
    glBindFramebuffer_(GL_FRAMEBUFFER, L_fbo.framebuffer);
    glViewport(0, 0, Info.HResolution / 2, Info.VResolution);
    eye = -1;
}

extern "C" void hmd_prep_right()
{
    glBindFramebuffer_(GL_FRAMEBUFFER, R_fbo.framebuffer);
    glViewport(0, 0, Info.HResolution / 2, Info.VResolution);
    eye = +1;
}

extern "C" void hmd_persp(float n, float f)
{
    /* Projection and view matrices. */

    OVR::Util::Render::StereoEyeParams params;

    if (eye < 0)
        params = Stereo.GetEyeRenderParams(OVR::Util::Render::StereoEye_Left);
    else
        params = Stereo.GetEyeRenderParams(OVR::Util::Render::StereoEye_Right);

    glMatrixMode(GL_PROJECTION);
    hmd_gl_matrix(params.Projection);
    glMatrixMode(GL_MODELVIEW);
    hmd_gl_matrix(params.ViewAdjust);

    /* Head orientation. */

    OVR::Vector3f v;
    float         a;

    Fusion.GetOrientation().GetAxisAngle(&v, &a);
    glRotatef(OVR::RadToDegree(-a), v.x, v.y, v.z);
}

extern "C" void hmd_ortho()
{
    hmd_persp(0.5f, 2.0f);

    glScalef    ( 1.25f / Info.VResolution,  1.25f / Info.VResolution,  1.0f);
    glTranslatef(-0.50f * Info.HResolution, -0.50f * Info.VResolution, -1.0f);
}

/*---------------------------------------------------------------------------*/
