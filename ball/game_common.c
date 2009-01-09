#include "game_common.h"
#include "vec3.h"

const char *status_to_str(int s)
{
    switch (s)
    {
    case GAME_NONE:    return _("Aborted");
    case GAME_TIME:    return _("Time-out");
    case GAME_GOAL:    return _("Success");
    case GAME_FALL:    return _("Fall-out");
    default:           return _("Unknown");
    }
}

void game_comp_grav(float h[3], const float g[3],
                    float view_a,
                    float game_rx,
                    float game_rz)
{
    float x[3];
    float y[3] = { 0.0f, 1.0f, 0.0f };
    float z[3];
    float X[16];
    float Z[16];
    float M[16];

    /* Compute the gravity vector from the given world rotations. */

    z[0] = fsinf(V_RAD(view_a));
    z[1] = 0.0f;
    z[2] = fcosf(V_RAD(view_a));

    v_crs(x, y, z);
    v_crs(z, x, y);
    v_nrm(x, x);
    v_nrm(z, z);

    m_rot (Z, z, V_RAD(game_rz));
    m_rot (X, x, V_RAD(game_rx));
    m_mult(M, Z, X);
    m_vxfm(h, M, g);
}
