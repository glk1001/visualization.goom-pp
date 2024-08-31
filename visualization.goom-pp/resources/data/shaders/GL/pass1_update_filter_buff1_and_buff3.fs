#version 430

#include "pass1_update_filter_buff1_and_buff3_consts.h"

uniform sampler2D tex_filterBuff2;    // Low colors from last frame
uniform sampler2D tex_mainColorImage; // Main colors for this frame
uniform sampler2D tex_lowColorImage;  // Low colors for this frame

// At end of this pass, 'filterBuff1' contains the newly mapped colors plus low colors.
layout(binding = FILTER_BUFF1_IMAGE_UNIT, rgba16f) uniform image2D img_filterBuff1;
// At end of this pass, 'filterBuff3' contains the newly mapped colors plus main colors.
layout(binding = FILTER_BUFF3_IMAGE_UNIT, rgba16f) uniform image2D img_filterBuff3;

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
uniform float u_filterLerpFactor;  // For lerping between gpu and srce and dest buffers.

// For base multiplier, too close to 1, gives washed
// out look, too far away and things get too dark.
uniform float u_baseColorMultiplier;        // Used to factor this frames' buff2 color.
uniform float u_mainColorMultiplier = 1.0F; // Used to factor this frames' main color.
uniform float u_lowColorMultiplier  = 0.7F; // Used to factor this frames' low color.

#include "pass1_gpu_filter_effects_consts.h"
#include "pass1_gpu_filter_effects.fs"

vec4 GetPosMappedFilterBuff2ColorValue(const vec2 uv, const ivec2 deviceXY);
float GetBaseColorMultiplier(const vec3 color);
vec2 GetTexelPos(const vec2 filterPos);

void main()
{
  const ivec2 deviceXY = ivec2(gl_FragCoord.xy);

  vec4 filterBuff2Val = GetPosMappedFilterBuff2ColorValue(texCoord, deviceXY);
  const vec4 filterBuff3Val = imageLoad(img_filterBuff3, deviceXY);

  // Mix in some of the previous frames' color from the current deviceXY.
  filterBuff2Val.rgb = mix(filterBuff2Val.rgb, filterBuff3Val.rgb, u_prevFrameTMix);

  // Boost this frames' buff2 color by the base color multiplier.
  filterBuff2Val.rgb *= GetBaseColorMultiplier(filterBuff2Val.rgb);

  // Get and store the low color added to this frames' buff2 color.
  const vec4 colorLow            = texture(tex_lowColorImage, texCoord);
  const float alpha              = colorLow.a; // Use low color alpha for main also.
  const vec3 filterBuff2ColorLow = filterBuff2Val.rgb + (u_lowColorMultiplier * colorLow.rgb);
  imageStore(img_filterBuff1, deviceXY, vec4(filterBuff2ColorLow, alpha));

  // Get and store the main color added to this frames' buff2 color.
  const vec4 colorMain            = texture(tex_mainColorImage, texCoord);
  const vec3 filterBuff2ColorMain = filterBuff2Val.rgb + (u_mainColorMultiplier * colorMain.rgb);
  imageStore(img_filterBuff3, deviceXY, vec4(filterBuff2ColorMain, alpha));

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

TexelPositions GetPosMappedFilterBuff2TexelPositions(ivec2 deviceXY);
FilterBuffColors GetFilterBuff2Colors(const TexelPositions texelPositions);
vec4 GetColorFromMixOfColor1AndColor2(const FilterBuffColors filterBuff2Colors, const float tMix);
float GetColor1Color2TMix(const vec2 fromUV, const TexelPositions toTexelPositions);

vec4 GetPosMappedFilterBuff2ColorValue(const vec2 uv, const ivec2 deviceXY)
{
//   deviceXY = ivec2(deviceXY.x, HEIGHT - 1 - deviceXY.y);
//
//   const float xRatioDeviceToNormalizedCoord = FILTER_POS_COORD_WIDTH / float(WIDTH - 1);
//   const float yRatioDeviceToNormalizedCoord = FILTER_POS_COORD_WIDTH / float(WIDTH - 1);
//   const float X_MIN_COORD = FILTER_POS_MIN_COORD;
//   const float Y_MIN_COORD = FILTER_POS_MIN_COORD;
//
//   vec2 pos = vec2(X_MIN_COORD + (xRatioDeviceToNormalizedCoord * float(deviceXY.x)),
//                   Y_MIN_COORD + (yRatioDeviceToNormalizedCoord * float(deviceXY.y))
//              );
//
//   float x = (pos.x - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH;
//   float y = (pos.y - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH;
//   uv = vec2(x, 1 - (ASPECT_RATIO * y));
//
//   uv = GetTexelPos(pos);
//   //uv = vec2(deviceXY.x/float(WIDTH-1), deviceXY.y/float(HEIGHT-1));
//
// //   return texture(tex_filterBuff2, uv);
//   TexelPositions filterBuff2TexelPositions = TexelPositions(uv, uv);

  const TexelPositions filterBuff2TexelPositions = GetPosMappedFilterBuff2TexelPositions(deviceXY);
  const FilterBuffColors filterBuff2Colors       = GetFilterBuff2Colors(filterBuff2TexelPositions);
  return GetColorFromMixOfColor1AndColor2(
      filterBuff2Colors, GetColor1Color2TMix(uv, filterBuff2TexelPositions));
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

TexelPositions GetPosMappedFilterBuff2TexelPositions(ivec2 deviceXY)
{
  deviceXY = ivec2(deviceXY.x, HEIGHT - 1 - deviceXY.y);

  LerpedNormalizedPositions lerpedNormalizedPositions = GetLerpedNormalizedPositions(deviceXY);

  if (u_resetSrceFilterPosBuffers)
  {
    ResetImageSrceFilterBuffPositions(deviceXY, lerpedNormalizedPositions);
  }

  const float deltaAmp  = 0.01F;
  const float deltaFreq = 0.05F;
  const vec2 delta      = vec2(cos(deltaFreq * u_time), sin(deltaFreq * u_time));
  lerpedNormalizedPositions.pos1 += deltaAmp * delta;
  lerpedNormalizedPositions.pos2 -= deltaAmp * delta;

  const vec2 GPUPos = GetGPUFilteredPosition(deviceXY);
  lerpedNormalizedPositions.pos1 = mix(lerpedNormalizedPositions.pos1, GPUPos, u_filterLerpFactor);
  lerpedNormalizedPositions.pos2 = mix(lerpedNormalizedPositions.pos2, GPUPos, u_filterLerpFactor);

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
  return SrceAndDestNormalizedPositions(
             imageLoad(img_filterSrcePosBuff1, deviceXY).xy,
             imageLoad(img_filterDestPosBuff1, deviceXY).xy,
             imageLoad(img_filterSrcePosBuff2, deviceXY).xy,
             imageLoad(img_filterDestPosBuff2, deviceXY).xy
  );
}

LerpedNormalizedPositions GetLerpedNormalizedPositions(const ivec2 deviceXY)
{
  const SrceAndDestNormalizedPositions normalizedPositions
        = GetSrceAndDestNormalizedPositions(deviceXY);

  return LerpedNormalizedPositions(
             mix(normalizedPositions.srcePos1, normalizedPositions.destPos1, u_srceDestLerpFactor),
             mix(normalizedPositions.srcePos2, normalizedPositions.destPos2, u_srceDestLerpFactor)
  );
}

void ResetImageSrceFilterBuffPositions(const ivec2 deviceXY,
                                       const LerpedNormalizedPositions lerpedNormalizedPositions)
{
  // Reset the filter srce pos buffers to the current lerped state, ready for
  // a new filter dest pos buffer.
  imageStore(img_filterSrcePosBuff1, deviceXY, vec4(lerpedNormalizedPositions.pos1, 0, 0));
  imageStore(img_filterSrcePosBuff2, deviceXY, vec4(lerpedNormalizedPositions.pos2, 0, 0));
}

vec2 GetTexelPos(const vec2 filterPos)
{
  const float x = (filterPos.x - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH;
  const float y = (filterPos.y - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH;

  return vec2(x, 1 - (ASPECT_RATIO * y));
}

TexelPositions GetTexelPositions(const LerpedNormalizedPositions lerpedNormalizedPositions)
{
  return TexelPositions(
             GetTexelPos(lerpedNormalizedPositions.pos1),
             GetTexelPos(lerpedNormalizedPositions.pos2)
  );
}


FilterBuffColors GetFilterBuff2Colors(const TexelPositions texelPositions)
{
  return FilterBuffColors(
             texture(tex_filterBuff2, texelPositions.uv1),
             texture(tex_filterBuff2, texelPositions.uv2)
  );
}

float GetSinTMix()
{
  // const float r = 0.25F;
  // const vec2 circleCentre1 = 0.5F + vec2(r*cos(u_pos1Pos2MixFreq * u_time), r*sin(u_pos1Pos2MixFreq * u_time));
  // const vec2 circleCentre2 = 0.4F + vec2(r*cos(1+u_pos1Pos2MixFreq * u_time), r*sin(1+u_pos1Pos2MixFreq * u_time));
  // float dist1 = distance(uv, circleCentre1);
  // float dist2 = distance(uv, circleCentre2) - 0.2F;
  // vec3 t = vec3(step(0.0F, sdf_smin(dist1, dist2)));

  // vec3 t = vec3(step(0.5F, uv.x));
  // float posDist = distance(filtBuff2Pos1, filtBuff2Pos2)/5.5;
  // float posDist = distance(vec2(0), filtBuff2Pos2)/5.5F;

  // return 0.5F;
  // return step(100, u_time % 200);
  return 0.5F * (1.0F + sin(u_pos1Pos2MixFreq * u_time));
}

float GetUVDistAdjustedTMix(const vec2 fromUV,
                            const TexelPositions toTexelPositions,
                            const float tMix)
{
  const float MAX_UV = sqrt(2.0F);
  //vec2 posUv = mix(toTexelPositions.uv1, toTexelPositions.uv2, vec2(tMix.x));
  //float distUv = min(distance(fromUV, posUv), MAX_UV) / MAX_UV;
  const float uvDist = min(distance(fromUV, toTexelPositions.uv2), MAX_UV) / MAX_UV;

  return tMix * (1.0F - uvDist);
}

float GetColor1Color2TMix(const vec2 fromUV, const TexelPositions toTexelPositions)
{
  const float SIN_T_MIX = GetSinTMix();
  // return SIN_T_MIX;
  return GetUVDistAdjustedTMix(fromUV, toTexelPositions, SIN_T_MIX);
}

vec4 GetColorFromMixOfColor1AndColor2(const FilterBuffColors filterBuff2Colors, const float tMix)
{
  const vec3 mixedColor = mix(filterBuff2Colors.color1.rgb,
                              filterBuff2Colors.color2.rgb,
                              vec3(tMix));
  const float alpha = filterBuff2Colors.color1.a;

  return vec4(mixedColor, alpha);
}
