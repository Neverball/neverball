#include "ease.h"
#include "vec3.h"

/*
 * https://easings.net/#easeInBack
 * Date: 2024-10-19
 * License: GPLv3
 */
float easeInBack(float x)
{
    const float c1 = 1.70158;
    const float c3 = c1 + 1;

    return c3 * x * x * x - c1 * x * x;
}

float easeOutBack(float x)
{
    const float c1 = 1.70158;
    const float c3 = c1 + 1;

    return 1 + c3 * fpowf(x - 1, 3) + c1 * fpowf(x - 1, 2);
}

/*
 * https://easings.net/#easeInOutBack
 * Date: 2024-10-18
 * License: GPLv3
 */
float easeInOutBack(float x)
{
    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;

    return (
        x < 0.5f ?
        (fpowf(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2 :
        (fpowf(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2
    );
}

float easeInElastic(float x)
{
    const float c4 = (2 * V_PI) / 3;

    return (
        x == 0
        ? 0
        : (
            x == 1
            ? 1
            : -fpowf(2, 10 * x - 10) * fsinf((x * 10 - 10.75) * c4)
        )
    );
}

float easeOutElastic(float x)
{
    const float c4 = (2 * V_PI) / 3;

    return (
        x == 0
        ? 0
        : (
            x == 1
            ? 1
            : fpowf(2, -10 * x) * fsinf((x * 10 - 0.75) * c4) + 1
        )
    );
}

/*
 * https://easings.net/#easeInOutElastic
 * Date: 2024-10-18
 * License: GPLv3
 */
float easeInOutElastic(float x)
{
    const float c5 = (2 * V_PI) / 4.5;

    return (
        x == 0.0f
        ? 0
        : (
            x == 1
            ? 1
            : (
                x < 0.5
                ? -(fpowf(2, +20 * x - 10) * fsinf((20 * x - 11.125) * c5)) / 2
                : +(fpowf(2, -20 * x + 10) * fsinf((20 * x - 11.125) * c5)) / 2 + 1
            )
        )
    );
}