#version 430

#include "pass1_update_filter_buff1_and_buff3_consts.h_glsl"

uniform sampler2D tex_mainColorImage;        // Main colors for this frame
uniform sampler2D tex_lowColorImage;         // Low colors for this frame
uniform sampler2D tex_persistentColorsImage; // Colors from last frame

// At the start of this pass, 'img_lowColorsBuff' contains the previous frames'
// mapped and low colors.
// At end of this pass, 'img_lowColorsBuff' contains the newly mapped colors plus low colors.
layout(binding = LOW_COLORS_BUFF_IMAGE_UNIT, rgba16f) uniform image2D img_lowColorsBuff;
// At the start of this pass, 'img_mainColorsBuff' contains the previous frames'
// mapped and main colors.
// At end of this pass, 'img_mainColorsBuff' contains the newly mapped colors plus main colors.
layout(binding = MAIN_COLORS_BUFF_IMAGE_UNIT, rgba16f) uniform image2D img_mainColorsBuff;

// All the buffers used for position mapping.
layout(binding = FILTER_SRCE_POS_IMAGE_UNIT1, rg32f) uniform image2D  img_filterSrcePosBuff1;
layout(binding = FILTER_SRCE_POS_IMAGE_UNIT2, rg32f) uniform image2D  img_filterSrcePosBuff2;
layout(binding = FILTER_DEST_POS_IMAGE_UNIT1, rg32f) uniform readonly image2D img_filterDestPosBuff1;
layout(binding = FILTER_DEST_POS_IMAGE_UNIT2, rg32f) uniform readonly image2D img_filterDestPosBuff2;

in vec3 position;
in vec2 texCoord;

uniform float u_srceDestLerpFactor;  // For lerping between srce and dest buffers.
uniform float u_prevFrameTMix;
uniform bool u_resetSrceFilterPosBuffers;
uniform float u_pos1Pos2MixFreq;
uniform float u_time;

uniform float u_gpuFilterLerpFactor;  // For lerping between gpu and srce and dest buffers.
uniform float u_maxGpuFilterLerpFactor = 1.0F;
uniform float u_maxGpuColorMixFactor   = 0.75F;
uniform bool u_useGpuFilterPositionsToGetColor = false;

// For base multiplier, too close to 1, gives a washed out look,
// too far away and things look too dark.
uniform float u_baseColorMultiplier;        // Used to factor this frames' persistent color.
uniform float u_mainColorMultiplier = 1.0F; // Used to factor this frames' main color.
uniform float u_lowColorMultiplier  = 1.0F; // Used to factor this frames' low color.

#include "pass1_gpu_filter_effects_consts.h_glsl"
#include "pass1_gpu_filter_effects.frag"

vec4 GetPosMappedPersistentColorValue(const vec2 uv, const ivec2 deviceXY);
float GetBaseColorMultiplier(const vec3 color);
vec2 GetTexelPos(const vec2 filterPos);

void main()
{
    const ivec2 deviceXY = ivec2(gl_FragCoord.xy);

    vec4 mappedPersistentColorVal = GetPosMappedPersistentColorValue(texCoord, deviceXY);
    const vec4 mainColorVal = imageLoad(img_mainColorsBuff, deviceXY);

    // Mix in some of the previous frames' color from the current deviceXY pixel.
    mappedPersistentColorVal.rgb = mix(mappedPersistentColorVal.rgb,
                                       mainColorVal.rgb,
                                       u_prevFrameTMix);

    // Boost this frames' mapped color by the base color multiplier.
    mappedPersistentColorVal.rgb *= GetBaseColorMultiplier(mappedPersistentColorVal.rgb);

    // Get and store the low color added to this frames' mapped color.
    const vec4 imageLowColor  = texture(tex_lowColorImage, texCoord);
    const float imageLowAlpha = imageLowColor.a; // Use low color alpha for main also.
    const vec3 newLowColor    = mappedPersistentColorVal.rgb
                                + (u_lowColorMultiplier * imageLowColor.rgb);
    imageStore(img_lowColorsBuff, deviceXY, vec4(newLowColor, imageLowAlpha));

    // Get and store the main color added to this frames' mapped color.
    const vec4 imageMainColor = texture(tex_mainColorImage, texCoord);
    const vec3 newMainColor   = mappedPersistentColorVal.rgb
                                + (u_mainColorMultiplier * imageMainColor.rgb);

#if (DEBUG_GPU_FILTERS == 0)
    imageStore(img_mainColorsBuff, deviceXY, vec4(newMainColor, imageLowAlpha));
#else
    const vec3 debugMainColor = GetDebugColor(deviceXY, newMainColor);
    imageStore(img_mainColorsBuff, deviceXY, vec4(debugMainColor, imageLowAlpha));
#endif

    discard;
}


const float BLACK_CUTOFF = 0.03F;

bool NotCloseToBlack(const vec3 color)
{
    return (color.r > BLACK_CUTOFF) || (color.r != color.g) || (color.r != color.b);
}

// Try to get purer blacks by using a lower baseColorMultiplier for small grey values.
float GetBaseColorMultiplier(const vec3 color)
{
    const float LOW_BASE_COLOR_MULTIPLIER = 0.25F;

    return NotCloseToBlack(color) ? u_baseColorMultiplier
                                  : mix(LOW_BASE_COLOR_MULTIPLIER,
                                        u_baseColorMultiplier,
                                        pow(color.r / BLACK_CUTOFF, 3.0));
}


struct TexelPositions
{
    vec2 uv1;
    vec2 uv2;
};
struct FilterBuffColors
{
    vec4 color1;
    vec4 color2;
};

TexelPositions GetFilterBuffPosMappedTexelPositions(ivec2 deviceXY);
FilterBuffColors GetPersistentColors(const TexelPositions texelPositions);
vec4 GetColorFromMixOfColor1AndColor2(const FilterBuffColors filterBuffColors, const float tMix);
float GetColor1Color2TMix(const vec2 fromUV, const TexelPositions texelPositions);

vec4 GetPosMappedPersistentColorValue(const vec2 uv, const ivec2 deviceXY)
{
    const TexelPositions mappedTexelPositions = GetFilterBuffPosMappedTexelPositions(deviceXY);
    const FilterBuffColors persistentColors   = GetPersistentColors(mappedTexelPositions);

    const vec4 color1Color2Mix = GetColorFromMixOfColor1AndColor2(
        persistentColors, GetColor1Color2TMix(uv, mappedTexelPositions));

    if (!u_useGpuFilterPositionsToGetColor || AllGpuFilterModesAreNone())
    {
        return color1Color2Mix;
    }

    // Use Gpu filter position to directly get color.
    bool isZeroPos;
    const float gpuLerpFactor = u_maxGpuColorMixFactor * u_gpuFilterLerpFactor;
    const vec2 gpuPos           = GetFinalGpuFilteredPosition(deviceXY, isZeroPos);
    const vec2 gpuTexelPos      = GetTexelPos(gpuPos);
    const vec4 gpuColor         = texture(tex_persistentColorsImage, gpuTexelPos);
    // const vec4 colorGpuColorMix = mix(color1Color2Mix, gpuColor, gpuLerpFactor);
    const vec4 colorGpuColorMix = color1Color2Mix + (0.1 * gpuLerpFactor * gpuColor);

    return colorGpuColorMix;
}


struct LerpedNormalizedPositions
{
    vec2 pos1;
    vec2 pos2;
};
LerpedNormalizedPositions GetLerpedNormalizedPositions(const ivec2 deviceXY);
TexelPositions GetTexelPositions(const LerpedNormalizedPositions lerpedNormalizedPositions);
void ResetImageSrceFilterBuffPositions(const ivec2 deviceXY,
                                       const LerpedNormalizedPositions lerpedNormalizedPositions);

TexelPositions GetFilterBuffPosMappedTexelPositions(ivec2 deviceXY)
{
    deviceXY = ivec2(deviceXY.x, HEIGHT - 1 - deviceXY.y);

    LerpedNormalizedPositions lerpedNormalizedPositions = GetLerpedNormalizedPositions(deviceXY);

    if (u_resetSrceFilterPosBuffers)
    {
        ResetImageSrceFilterBuffPositions(deviceXY, lerpedNormalizedPositions);
    }

    const float deltaAmp = 0.01F;
    const float deltaFreq = 0.05F;
    const vec2 delta = vec2(cos(deltaFreq * u_time), sin(deltaFreq * u_time));
    lerpedNormalizedPositions.pos1 += deltaAmp * delta;
    lerpedNormalizedPositions.pos2 -= deltaAmp * delta;

    if (!u_useGpuFilterPositionsToGetColor && !AllGpuFilterModesAreNone())
    {
        bool isZeroPos;
        const vec2 gpuPos = GetFinalGpuFilteredPosition(deviceXY, isZeroPos);
        if (!isZeroPos)
        {
            const float gpuLerpFactor      = u_maxGpuFilterLerpFactor * u_gpuFilterLerpFactor;
            lerpedNormalizedPositions.pos1 = mix(lerpedNormalizedPositions.pos1, gpuPos, gpuLerpFactor);
            lerpedNormalizedPositions.pos2 = mix(lerpedNormalizedPositions.pos2, gpuPos, gpuLerpFactor);
        }
    }

    return GetTexelPositions(lerpedNormalizedPositions);
}

struct SrceAndDestNormalizedPositions
{
    vec2 srcePos1;
    vec2 destPos1;
    vec2 srcePos2;
    vec2 destPos2;
};

SrceAndDestNormalizedPositions GetSrceAndDestNormalizedPositions(const ivec2 deviceXY)
{
    return SrceAndDestNormalizedPositions(imageLoad(img_filterSrcePosBuff1, deviceXY).xy,
                                          imageLoad(img_filterDestPosBuff1, deviceXY).xy,
                                          imageLoad(img_filterSrcePosBuff2, deviceXY).xy,
                                          imageLoad(img_filterDestPosBuff2, deviceXY).xy);
}

LerpedNormalizedPositions GetLerpedNormalizedPositions(const ivec2 deviceXY)
{
    const SrceAndDestNormalizedPositions
              normalizedPositions = GetSrceAndDestNormalizedPositions(deviceXY);

    return LerpedNormalizedPositions(
             mix(normalizedPositions.srcePos1, normalizedPositions.destPos1, u_srceDestLerpFactor),
             mix(normalizedPositions.srcePos2, normalizedPositions.destPos2, u_srceDestLerpFactor));
}

vec2 GetTexelPos(const vec2 normalizedPosition)
{
    const float x = (normalizedPosition.x - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH;
    const float y = (normalizedPosition.y - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH;

    return vec2(x, 1 - (ASPECT_RATIO * y));
}

TexelPositions GetTexelPositions(const LerpedNormalizedPositions lerpedNormalizedPositions)
{
    return TexelPositions(GetTexelPos(lerpedNormalizedPositions.pos1),
                          GetTexelPos(lerpedNormalizedPositions.pos2));
}

void ResetImageSrceFilterBuffPositions(const ivec2 deviceXY,
const LerpedNormalizedPositions lerpedNormalizedPositions)
{
    // Reset the filter srce pos buffers to the current lerped state, ready for
    // a new filter dest pos buffer.
    imageStore(img_filterSrcePosBuff1, deviceXY, vec4(lerpedNormalizedPositions.pos1, 0, 0));
    imageStore(img_filterSrcePosBuff2, deviceXY, vec4(lerpedNormalizedPositions.pos2, 0, 0));
}


FilterBuffColors GetPersistentColors(const TexelPositions texelPositions)
{
    return FilterBuffColors(texture(tex_persistentColorsImage, texelPositions.uv1),
                            texture(tex_persistentColorsImage, texelPositions.uv2));
}

float GetSinTMix()
{
    return 0.5F * (1.0F + sin(u_pos1Pos2MixFreq * u_time));
}

float GetUVDistAdjustedTMix(const vec2 fromUV,
                            const TexelPositions texelPositions,
                            const float tMix)
{
    const float MAX_UV = sqrt(2.0F);
    const float uvDist = min(distance(fromUV, texelPositions.uv2), MAX_UV) / MAX_UV;

    return tMix * (1.0F - uvDist);
}

float GetColor1Color2TMix(const vec2 fromUV, const TexelPositions texelPositions)
{
    const float SIN_T_MIX = GetSinTMix();
    return GetUVDistAdjustedTMix(fromUV, texelPositions, SIN_T_MIX);
}

vec4 GetColorFromMixOfColor1AndColor2(const FilterBuffColors filterBuffColors, const float tMix)
{
    const vec3 mixedColor = mix(filterBuffColors.color1.rgb,
                                filterBuffColors.color2.rgb,
                                vec3(tMix));
    const float alpha = filterBuffColors.color1.a;

    return vec4(mixedColor, alpha);
}
