// Assumes:
//
// uniform float u_time;

uniform uint u_gpuSrceFilterMode;
uniform uint u_gpuDestFilterMode;
uniform float u_gpuSrceDestFilterLerpFactor;
uniform float u_gpuFilterMaxTime;
uniform vec2 u_gpuFilterMidpoint;

uniform float u_amuletXAmplitude;
uniform float u_amuletYAmplitude;
uniform float u_amuletXBase;
uniform float u_amuletYBase;
uniform float u_amuletXFreq;
uniform float u_amuletYFreq;

vec2 GetAmuletVelocity(const vec2 position)
{
    vec2 p = position;

    const float sqDistFromZero = (p.x * p.x) + (p.y * p.y);

    const float sinT = sin(u_amuletXFreq * u_time);
    const float cosT = cos(u_amuletYFreq * u_time);
    const float x = p.x;
    p.x = p.x * cosT - p.y * sinT;
    p.y = p.y * cosT + x * sinT;

    const float baseX = u_amuletXBase;
    const float baseY = u_amuletYBase;

    const vec2 v = vec2(baseX + (u_amuletXAmplitude * sqDistFromZero),
                        baseY + (u_amuletYAmplitude * sqDistFromZero));

    return -p * v;
}

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

    const float sqDistFromZero = (p.x * p.x) + (p.y * p.y);
    const float reducer      = exp(-u_waveReducerCoeff * sqDistFromZero);

    const float freqFactor = 10.5;
    const float angle = pow(sqDistFromZero, u_waveSqDistPower);

    const float cosAngle = cos(u_waveXFreq * angle);
    const float sinAngle = sin(u_waveYFreq * angle);

    const vec2 v = vec2(u_waveXBase + (reducer * u_waveXAmplitude * cosAngle),
                        u_waveYBase + (reducer * u_waveYAmplitude * sinAngle));

    return -p * v;
}

uniform float u_vortexXAmplitude;
uniform float u_vortexYAmplitude;
uniform float u_vortexXBase;
uniform float u_vortexYBase;
uniform float u_vortexFreq;
uniform float u_vortexPositionFactor;
uniform float u_vortexRFactor;

vec2 GetVortexVelocity(const vec2 position)
{
    const vec2 p = position;

    const float r = length(p);
    const float theta = atan(p.y, p.x);
    const float t = sqrt(u_vortexRFactor * r) + theta + (u_vortexFreq * u_time);

    vec2 v = vec2(p.y, -p.x) / r;
    v *= sin(t);
    v.x *= u_vortexXAmplitude * length(v.x);
    v.y *= u_vortexYAmplitude * length(v.y);
    v += u_vortexPositionFactor * p;

    return v;
}

vec2 GetReflectingPoolVelocity(const vec2 position)
{
    const vec2 p = position;

    const float timeFreq = 0.01;
    const float t = 5.0 * sin(timeFreq * u_time);
    const float A = 1.5;

    const vec2 v = vec2(A * sin((t * p.y) + p.x),
                        A * cos((t * p.x) - p.y));

    return v;
}

vec2 GetBeautifulFieldVelocity(const vec2 position)
{
    const vec2 p = position;

    const float frame = 150.0 * sin(0.01 * u_time);

    const float PI = 3.14;
    const float dt = 0.01;

    const float t = frame * dt;
    const float w = 2.0 * (PI / 5.0);
    const float A = 5.0;

    const float d = sqrt((p.x * p.x) + (p.y * p.y));

    const vec2 v = vec2(A * cos((w * t) / d), A * sin((w * t) / d));

    return v;
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
        return GetGpuFilteredPosition(u_gpuDestFilterMode, deviceXY);
    }
    if (u_gpuDestFilterMode == GPU_NONE_MODE)
    {
        return GetGpuFilteredPosition(u_gpuSrceFilterMode, deviceXY);
    }
    if ((u_gpuSrceFilterMode == u_gpuDestFilterMode) || (u_gpuSrceDestFilterLerpFactor >= 1.0F))
    {
        return GetGpuFilteredPosition(u_gpuDestFilterMode, deviceXY);
    }

    const vec2 srceFilterPosition = GetGpuFilteredPosition(u_gpuSrceFilterMode, deviceXY);
    const vec2 destFilterPosition = GetGpuFilteredPosition(u_gpuDestFilterMode, deviceXY);

    return mix(srceFilterPosition, destFilterPosition, u_gpuSrceDestFilterLerpFactor);
}
