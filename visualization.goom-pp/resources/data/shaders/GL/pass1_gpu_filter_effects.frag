// Assumes:
//
// uniform float u_time;

const float TWO_PI = 2.0F * 3.14159F;

uniform uint u_gpuSrceFilterMode;
uniform uint u_gpuDestFilterMode;
uniform float u_gpuSrceDestFilterLerpFactor;
uniform vec2 u_gpuFilterMidpoint;

uniform float u_amuletStartTime;
uniform float u_amuletMaxTime;
uniform float u_amuletXCycleFreq;
uniform float u_amuletYCycleFreq;
uniform float u_amuletXAmplitude;
uniform float u_amuletYAmplitude;
uniform float u_amuletXBase;
uniform float u_amuletYBase;
uniform float u_amuletXFreq;
uniform float u_amuletYFreq;
uniform float u_amuletSpinSign;

vec2 GetAmuletVelocity(const vec2 position)
{
    vec2 p = position;

    const vec2 amuletBase = vec2(u_amuletXBase, u_amuletYBase);

    const float elapsedTime  = u_time - u_amuletStartTime;
    const float timeToGoFrac = elapsedTime / u_amuletMaxTime;

    const float sinT = sin(u_amuletXCycleFreq * timeToGoFrac * TWO_PI);
    const float cosT = cos(u_amuletYCycleFreq * timeToGoFrac * TWO_PI);

    // Rotate...
    const float x = p.x;
    p.x = p.x * cosT - (u_amuletSpinSign * p.y * sinT);
    p.y = p.y * cosT + (u_amuletSpinSign * x * sinT);

    const float sqDistFromZero = (p.x * p.x) + (p.y * p.y);

    const vec2 v = amuletBase + vec2(u_amuletXAmplitude * sqDistFromZero,
                                     u_amuletYAmplitude * sqDistFromZero);

    return -p * v;
}

uniform float u_waveStartTime;
uniform float u_waveMaxTime;
uniform float u_waveXCycleFreq;
uniform float u_waveYCycleFreq;
uniform float u_waveXAmplitude;
uniform float u_waveYAmplitude;
uniform float u_waveXBase;
uniform float u_waveYBase;
uniform float u_waveXFreq;
uniform float u_waveYFreq;
uniform float u_waveReducerCoeff;
uniform float u_waveSqDistPower;

vec2 GetWaveVelocity(const vec2 position)
{
    const vec2 p = position;

    const vec2 waveBase = vec2(u_waveXBase, u_waveYBase);

    const float sqDistFromZero = (p.x * p.x) + (p.y * p.y);
    const float reducer        = exp(-u_waveReducerCoeff * sqDistFromZero);

    const float angle    = pow(sqDistFromZero, u_waveSqDistPower);
    const float cosAngle = cos(u_waveXFreq * angle);
    const float sinAngle = sin(u_waveYFreq * angle);

    const vec2 v = waveBase + vec2(reducer * u_waveXAmplitude * cosAngle,
                                   reducer * u_waveYAmplitude * sinAngle);

    return -p * v;
}

uniform float u_vortexStartTime;
uniform float u_vortexMaxTime;
uniform float u_vortexXCycleFreq;
uniform float u_vortexYCycleFreq;
uniform float u_vortexXAmplitude;
uniform float u_vortexYAmplitude;
uniform float u_vortexXBase;
uniform float u_vortexYBase;
uniform float u_vortexFreq;
uniform float u_vortexPositionFactor;
uniform float u_vortexRFactor;
uniform float u_vortexSpinSign;

vec2 GetVortexVelocity(const vec2 position)
{
    vec2 p = position;

    const vec2 vortexBase = vec2(u_vortexXBase, u_vortexYBase);

    const float elapsedTime  = u_time - u_vortexStartTime;
    const float timeToGoFrac = elapsedTime / u_vortexMaxTime;

    const float sinT = sin(u_vortexXCycleFreq * timeToGoFrac * TWO_PI);
    const float cosT = cos(u_vortexYCycleFreq * timeToGoFrac * TWO_PI);

    // Rotate...
    const float x = p.x;
    p.x = p.x * cosT - (u_vortexSpinSign * p.y * sinT);
    p.y = p.y * cosT + (u_vortexSpinSign * x * sinT);

    const float r     = length(p);
    const float theta = atan(p.y, p.x);
    const float t     = sqrt(u_vortexRFactor * r) + theta + (u_vortexFreq * u_time);

    vec2 v = vec2(p.y, -p.x) / r;

    v *= sin(t);

    const float vLength = length(v);
    v.x *= u_vortexXAmplitude * vLength;
    v.y *= u_vortexYAmplitude * vLength;

    v += u_vortexPositionFactor * p;

    return vortexBase + v;
}

uniform float u_reflectingPoolStartTime;
uniform float u_reflectingPoolMaxTime;
uniform float u_reflectingPoolXCycleFreq;
uniform float u_reflectingPoolYCycleFreq;
uniform float u_reflectingPoolXAmplitude;
uniform float u_reflectingPoolYAmplitude;
uniform float u_reflectingPoolXBase;
uniform float u_reflectingPoolYBase;
uniform float u_reflectingPoolXInnerSinFreq;
uniform float u_reflectingPoolYInnerSinFreq;
uniform float u_reflectingPoolXFreq;
uniform float u_reflectingPoolYFreq;
uniform float u_reflectingPoolInnerPosXFactor;
uniform float u_reflectingPoolInnerPosYFactor;

vec2 GetReflectingPoolVelocity(const vec2 position)
{
    const vec2 p = position;

    const vec2 reflectingPoolBase = vec2(u_reflectingPoolXBase, u_reflectingPoolYBase);

    const float elapsedTime  = u_time - u_reflectingPoolStartTime;
    const float timeToGoFrac = elapsedTime / u_reflectingPoolMaxTime;

    const float xT = u_reflectingPoolXFreq * sin(u_reflectingPoolXCycleFreq * timeToGoFrac * TWO_PI);
    const float yT = u_reflectingPoolYFreq * sin(u_reflectingPoolYCycleFreq * timeToGoFrac * TWO_PI);

    const float vX = sin((xT * p.y) + (u_reflectingPoolInnerPosXFactor * p.x));
    const float vY = cos((yT * p.x) - (u_reflectingPoolInnerPosYFactor * p.y));

    const vec2 v = vec2(u_reflectingPoolXAmplitude * vX, u_reflectingPoolYAmplitude * vY);

    return reflectingPoolBase + v;
}

uniform float u_beautifulFieldStartTime;
uniform float u_beautifulFieldMaxTime;
uniform float u_beautifulFieldXCycleFreq;
uniform float u_beautifulFieldYCycleFreq;
uniform float u_beautifulFieldXAmplitude;
uniform float u_beautifulFieldYAmplitude;
uniform float u_beautifulFieldXBase;
uniform float u_beautifulFieldYBase;
uniform float u_beautifulFieldXInnerSinFreq;
uniform float u_beautifulFieldYInnerSinFreq;
uniform float u_beautifulFieldXFreq;
uniform float u_beautifulFieldYFreq;
uniform float u_beautifulFieldDirection;

vec2 GetBeautifulFieldVelocity(const vec2 position)
{
    vec2 p = position;

    const vec2 beautifulFieldBase = vec2(u_beautifulFieldXBase, u_beautifulFieldYBase);

    const float elapsedTime       = u_time - u_beautifulFieldStartTime;
    const float timeToGoFrac      = elapsedTime / u_beautifulFieldMaxTime;
    const float timeElapsedFrac   = 1.0F - timeToGoFrac;
    const float timeElapsedFracSq = timeElapsedFrac * timeElapsedFrac;

    // Make a spiralling path for 'p'.
    const float ELLIPSE_SEMI_MAJOR = 4.5F;
    const float ELLIPSE_SEMI_MINOR = 2.5F;
    const float ELLIPSE_SPEED      = 0.025F;
    const float x = timeElapsedFracSq * (ELLIPSE_SEMI_MAJOR * cos(ELLIPSE_SPEED * u_time));
    const float y = -u_beautifulFieldDirection
                    * timeElapsedFracSq * (ELLIPSE_SEMI_MINOR * sin(ELLIPSE_SPEED * u_time));
    p  = p + 0.4 * vec2(x, y);

    const float xT = u_beautifulFieldXFreq * sin(u_beautifulFieldXCycleFreq * timeToGoFrac * TWO_PI);
    const float yT = u_beautifulFieldYFreq * sin(u_beautifulFieldYCycleFreq * timeToGoFrac * TWO_PI);

    const float w = TWO_PI / 5.0;

    const float d = length(p);
    const float MAX_D = 0.5F * (sqrt(2.0F) * FILTER_POS_COORD_WIDTH);

    const float argDenom = pow(d, 0.1 + xT) * ((0.1 + MAX_D) - d);
    const float argCos = ((w * xT) / argDenom);
    const float argSin = ((w * yT) / argDenom) * u_beautifulFieldDirection;

    const vec2 v = vec2(u_beautifulFieldXAmplitude * cos(argCos),
                        u_beautifulFieldYAmplitude * sin(argSin));

    return beautifulFieldBase + v;
}

uniform float u_upDownStartTime;
uniform float u_upDownMaxTime;
uniform float u_upDownXCycleFreq;
uniform float u_upDownYCycleFreq;
uniform float u_upDownXAmplitude;
uniform float u_upDownYAmplitude;
uniform float u_upDownXBase;
uniform float u_upDownYBase;
uniform float u_upDownXFreq;
uniform float u_upDownYFreq;
uniform float u_upDownRotateFreq;
uniform float u_upDownMixFreq;

vec2 GetUpDownVelocity(const vec2 position)
{
    vec2 p = position;

    const vec2 upDownBase = vec2(u_upDownXBase, u_upDownYBase);

    const float elapsedTime  = u_time - u_upDownStartTime;
    const float timeToGoFrac = elapsedTime / u_upDownMaxTime;

    const float xT = u_upDownXFreq * sin(u_upDownXCycleFreq * timeToGoFrac * TWO_PI);
    const float yT = u_upDownYFreq * sin(u_upDownYCycleFreq * timeToGoFrac * TWO_PI);

    const vec2 vUp = vec2(u_upDownXAmplitude * ((xT * p.x) + sin(xT * p.y)),
                          u_upDownYAmplitude * cos(yT * p.x));
    const vec2 vAcross = vec2(u_upDownXAmplitude * ((xT * p.y) + cos(xT * p.x)),
                              u_upDownYAmplitude * sin(yT * p.y));

    const vec2 rotateVec = vec2(cos(u_upDownRotateFreq * timeToGoFrac * TWO_PI),
                                sin(u_upDownRotateFreq * timeToGoFrac * TWO_PI));

    const vec2 v = rotateVec * mix(vUp, vAcross, sin(u_upDownMixFreq * timeToGoFrac * TWO_PI));

    return upDownBase + v;
}

// Following assumes HEIGHT <= WIDTH.
const float RATIO_DEV_TO_NORMALIZED_COORD = FILTER_POS_COORD_WIDTH / float(WIDTH - 1);
// const vec2 MIDPOINT = vec2(FILTER_POS_MIN_COORD + (RATIO_DEV_TO_NORMALIZED_COORD * float(WIDTH/2)),
//                            FILTER_POS_MIN_COORD + (RATIO_DEV_TO_NORMALIZED_COORD * float(HEIGHT/2))
//            );

vec2 GetGpuFilteredPosition(const uint gpuFilterMode, const ivec2 deviceXY)
{
    const vec2 pos = vec2(FILTER_POS_MIN_COORD + (RATIO_DEV_TO_NORMALIZED_COORD * float(deviceXY.x)),
                          FILTER_POS_MIN_COORD + (RATIO_DEV_TO_NORMALIZED_COORD * float(deviceXY.y)));

    const vec2 centredPos = pos - u_gpuFilterMidpoint;
    //const vec2 centredPos = pos - MIDPOINT;

    vec2 velocity = vec2(0.0);

    switch (gpuFilterMode)
    {
        case GPU_AMULET_MODE:
            velocity = GetAmuletVelocity(centredPos);
            break;
        case GPU_WAVE_MODE:
            velocity = GetWaveVelocity(centredPos);
            break;
        case GPU_VORTEX_MODE:
            velocity = GetVortexVelocity(centredPos);
            break;
        case GPU_REFLECTING_POOL_MODE:
            velocity = GetReflectingPoolVelocity(centredPos);
            break;
        case GPU_BEAUTIFUL_FIELD_MODE:
            velocity = GetBeautifulFieldVelocity(centredPos);
            break;
        case GPU_UP_DOWN_MODE:
            velocity = GetUpDownVelocity(centredPos);
            break;
        default:
            break;
    }

    //return (centredPos + velocity) + MIDPOINT;
    return (centredPos + velocity) + u_gpuFilterMidpoint;
}

bool AllGpuFilterModesAreNone()
{
    return (u_gpuSrceFilterMode == GPU_NONE_MODE) && (u_gpuDestFilterMode == GPU_NONE_MODE);
}

vec2 GetFinalGpuFilteredPosition(const ivec2 deviceXY)
{
    if (u_gpuSrceFilterMode == GPU_NONE_MODE)
    {
        return u_gpuSrceDestFilterLerpFactor <= 0.001F
            ? vec2(0.0F)
            : u_gpuSrceDestFilterLerpFactor * GetGpuFilteredPosition(u_gpuDestFilterMode, deviceXY);
    }
    if (u_gpuDestFilterMode == GPU_NONE_MODE)
    {
        const float srceFactor = 1.0F - u_gpuSrceDestFilterLerpFactor;
        return srceFactor < 0.001F
            ? vec2(0.0F)
            : srceFactor * GetGpuFilteredPosition(u_gpuSrceFilterMode, deviceXY);
    }
    if ((u_gpuSrceFilterMode == u_gpuDestFilterMode) || (u_gpuSrceDestFilterLerpFactor >= 1.0F))
    {
        return GetGpuFilteredPosition(u_gpuDestFilterMode, deviceXY);
    }

    const vec2 srceFilterPosition = GetGpuFilteredPosition(u_gpuSrceFilterMode, deviceXY);
    const vec2 destFilterPosition = GetGpuFilteredPosition(u_gpuDestFilterMode, deviceXY);

    return mix(srceFilterPosition, destFilterPosition, u_gpuSrceDestFilterLerpFactor);
}
