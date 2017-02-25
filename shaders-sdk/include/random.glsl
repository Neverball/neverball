float counter = 0.0f;
int globalInvocationSMP = 0;

int hash( int x ) {
    x += ( x << 10 );
    x ^= ( x >>  6 );
    x += ( x <<  3 );
    x ^= ( x >> 11 );
    x += ( x << 15 );
    return x;
}

int hash( ivec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
int hash( ivec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
int hash( ivec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

float floatConstruct( int m ) {
    const int ieeeMantissa = 0x007FFFFF;
    const int ieeeOne      = 0x3F800000;
    m &= ieeeMantissa;
    m |= ieeeOne;
    float f = intBitsToFloat( m );
    return f - 1.0f;
}

float random( float x ) { return floatConstruct(hash(floatBitsToInt(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToInt(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToInt(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToInt(v))); }

float random() {
    float x = float(globalInvocationSMP);
    float y = 10.0f + counter + randomUniform.time;
    float r = random(vec2(x, y));
    counter += 1.0f;
    return r;
}

vec3 randomCosine(in vec3 normal) {
    float up = sqrt(random()); // cos(theta)
    float over = sqrt(-fma(up, up, -1.0f));
    float around = random() * TWO_PI;

    vec3 directionNotNormal;
    if (abs(normal.x) < SQRT_OF_ONE_THIRD) {
        directionNotNormal = vec3(1, 0, 0);
    } else if (abs(normal.y) < SQRT_OF_ONE_THIRD) {
        directionNotNormal = vec3(0, 1, 0);
    } else {
        directionNotNormal = vec3(0, 0, 1);
    }
    vec3 perpendicular1 = normalize( cross(normal, directionNotNormal) );
    vec3 perpendicular2 =             cross(normal, perpendicular1);
    return normalize(
        fma(     normal, vec3(up), 
            fma( perpendicular1 , vec3(cos(around)) * over, 
                 perpendicular2 * vec3(sin(around)) * over 
            )
        )
    );
}

vec3 randomDirectionInSphere() {
    float up = random() * 2.0f - 1.0f;
    float over = sqrt(1.0f - up * up);
    float around = random() * TWO_PI;
    return normalize(vec3( up, cos(around) * over, sin(around) * over ));
}
