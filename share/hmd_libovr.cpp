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
    #include "hmd_common.h"
    #include "config.h"
    #include "glext.h"
}

/*---------------------------------------------------------------------------*/

static OVR::Ptr<OVR::DeviceManager> pManager;
static OVR::Ptr<OVR::HMDDevice>     pHMD;
static OVR::Ptr<OVR::SensorDevice>  pSensor;

static OVR::HMDInfo      Info;
static OVR::SensorFusion Fusion;

static OVR::Util::Render::StereoConfig    Stereo;
static OVR::Util::Render::StereoEyeParams Params;

/*---------------------------------------------------------------------------*/

static void hmd_matrix(OVR::Matrix4f& M)
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
    return config_get_d(CONFIG_HMD);
}

extern "C" void hmd_init()
{
    // Set default HMD info for a 7" OVR DK1 in case OVR fails for any reason.

    Info.DesktopX               = 0;
    Info.DesktopY               = 0;
    Info.HResolution            = config_get_d(CONFIG_WIDTH);
    Info.VResolution            = config_get_d(CONFIG_HEIGHT);

    Info.HScreenSize            =  0.14976f;
    Info.VScreenSize            =  0.09350f;
    Info.InterpupillaryDistance =  0.0640f;
    Info.LensSeparationDistance =  0.0635f;
    Info.EyeToScreenDistance    =  0.0410f;
    Info.VScreenCenter          =  Info.VScreenSize * 0.5f;

    Info.DistortionK[0]         =  1.00f;
    Info.DistortionK[1]         =  0.22f;
    Info.DistortionK[2]         =  0.24f;

    Info.ChromaAbCorrection[0]  =  0.996f;
    Info.ChromaAbCorrection[1]  = -0.004f;
    Info.ChromaAbCorrection[2]  =  1.014f;
    Info.ChromaAbCorrection[3]  =  0.000f;

    // Initialize OVR, the device, the sensor, and the sensor fusion.

    OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));

    if (OVR::System::IsInitialized())
    {
        if ((pManager = *OVR::DeviceManager::Create()))
        {
            if ((pHMD = *pManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice()))
            {
                if ((pSensor = *pHMD->GetSensor()))
                {
                    Fusion.AttachToSensor(pSensor);

                    pHMD->GetDeviceInfo(&Info);
                }
            }
        }
    }

    // Set up the stereo config using the HMD info.

    using namespace OVR::Util::Render;

    Stereo.SetHMDInfo(Info);
    Stereo.SetDistortionFitPointVP(-1.0f, 0.0f);
    Stereo.SetStereoMode(Stereo_LeftRight_Multipass);
    Stereo.SetFullViewport(Viewport(0, 0, Info.HResolution,
                                          Info.VResolution));

    hmd_common_init(Info.HResolution, Info.VResolution);
}

extern "C" void hmd_free()
{
    hmd_common_free();

    pSensor  = 0;
    pHMD     = 0;
    pManager = 0;

    OVR::System::Destroy();
}

extern "C" void hmd_step()
{
    /* LibOVR is threaded. No synchronous update is necessary. */
}

extern "C" void hmd_swap()
{
    float center = 1 - (2 * Info.LensSeparationDistance) / Info.HScreenSize;
    float scale  = Stereo.GetDistortionScale();

    hmd_common_swap(center, scale, Info.DistortionK, Info.ChromaAbCorrection);
}

extern "C" void hmd_prep_left()
{
    hmd_common_left();
    Params = Stereo.GetEyeRenderParams(OVR::Util::Render::StereoEye_Left);
}

extern "C" void hmd_prep_right()
{
    hmd_common_right();
    Params = Stereo.GetEyeRenderParams(OVR::Util::Render::StereoEye_Right);
}

extern "C" void hmd_persp(float n, float f)
{
    /* Projection and view matrices. */

    glMatrixMode(GL_PROJECTION);
    hmd_matrix(Params.Projection);
    glMatrixMode(GL_MODELVIEW);
    hmd_matrix(Params.ViewAdjust);

    /* Head orientation. */

    if (pSensor)
    {
        OVR::Vector3f v;
        float         a;

        Fusion.GetOrientation().GetAxisAngle(&v, &a);
        glTranslatef(0.0f, -0.1f, 0.f);
        glRotatef(OVR::RadToDegree(-a), v.x, v.y, v.z);
        glTranslatef(0.0f, +0.1f, 0.f);
    }
}

extern "C" void hmd_ortho()
{
    hmd_persp(0.5f, 2.0f);

    glScalef    ( 1.25f / Info.VResolution,  1.25f / Info.VResolution,  1.0f);
    glTranslatef(-0.50f * Info.HResolution, -0.50f * Info.VResolution, -1.0f);
}

/*---------------------------------------------------------------------------*/
