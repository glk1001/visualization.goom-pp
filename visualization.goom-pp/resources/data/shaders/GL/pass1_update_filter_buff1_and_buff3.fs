#version 430

#include "pass1_update_filter_buff1_and_buff3_consts.h"

//#include "glsl-blend/all.glsl"

uniform sampler2D tex_filterBuff2;
uniform sampler2D tex_filterDestPositions;
uniform sampler2D tex_filterDestPositions2;
uniform sampler2D tex_mainImage;
uniform sampler2D tex_lowImage;

layout(binding = FILTER_BUFF1_IMAGE_UNIT, rgba16f) uniform image2D img_filterBuff1;
layout(binding = FILTER_BUFF3_IMAGE_UNIT, rgba16f) uniform image2D img_filterBuff3;

layout(binding = FILTER_SRCE_POS_IMAGE_UNIT1, rg32f) uniform image2D  img_filterSrcePosBuff1;
layout(binding = FILTER_SRCE_POS_IMAGE_UNIT2, rg32f) uniform image2D  img_filterSrcePosBuff2;
layout(binding = FILTER_DEST_POS_IMAGE_UNIT1, rg32f) uniform readonly image2D img_filterDestPosBuff1;
layout(binding = FILTER_DEST_POS_IMAGE_UNIT2, rg32f) uniform readonly image2D img_filterDestPosBuff2;

in vec3 position;
in vec2 texCoord;

uniform float u_lerpFactor;
uniform float u_buff2Buff3Mix = 0.1;
uniform bool u_resetSrceFilterPosBuffers;
uniform float u_pos1Pos2MixFreq;
uniform uint u_time;

// For base multiplier, too close to 1, gives washed
// out look, too far away things get too dark.
uniform float u_baseColorMultiplier;
uniform float u_mainColorMultiplier = 1.0;
uniform float u_lowColorMultiplier  = 0.7;

/**
vec3 blend(vec3 base, vec3 blend, float opacity)
{
//  return blendAdd(base, blend, opacity);
  return blendMode(1, base, blend, opacity);
//  return blendLighten(base, blend, opacity);
}
**/

vec4 GetPosMappedFilterBuff2Value(vec2 uv, ivec2 xy);
float GetBaseColorMultiplier(vec3 color);

void main()
{
  ivec2 xy = ivec2(gl_FragCoord.xy);

  vec4 filtBuff2Val = GetPosMappedFilterBuff2Value(texCoord, xy);
  vec4 filtBuff3Val = imageLoad(img_filterBuff3, xy);

  vec4 colorMain = texture(tex_mainImage, texCoord);
  vec4 colorLow  = texture(tex_lowImage, texCoord);

  //  vec4 filtBuff2ColorMain = vec4(blend(filtBuff2Val.rgb, 90*colorMain.rgb, 0.5*colorMain.a), 1.0);
  //  vec4 filtBuff2ColorMain = vec4((1-colorMain.a)*filtBuff2Val.rgb + colorMain.a*colorMain.rgb, 1.0);
  //  float alpha = 1 - 0.5 * colorMain.a;
  //  vec4 filtBuff2ColorMain = (1-alpha)*filtBuff2Val + alpha*50*colorMain;
  //  vec4 filtBuff2ColorMain = vec4(blend(100*colorMain.rgb, 0.5*filtBuff2Val.rgb, colorMain.a), 1.0);
  //  vec4 filtBuff2ColorMain = vec4(filtBuff2Val.rgb, 1.0);

  filtBuff2Val.rgb = mix(filtBuff2Val.rgb, filtBuff3Val.rgb, u_buff2Buff3Mix);

  float baseMultiplier = GetBaseColorMultiplier(filtBuff2Val.rgb);
  //float baseMultiplier = u_baseColorMultiplier;
  //filtBuff2Val.rgb *= 1.00;
  filtBuff2Val.rgb *= baseMultiplier;

  vec3 filtBuff2ColorMain = filtBuff2Val.rgb + (u_mainColorMultiplier * colorMain.rgb);
  vec3 filtBuff2ColorLow  = filtBuff2Val.rgb + (u_lowColorMultiplier * colorLow.rgb);

  imageStore(img_filterBuff1, xy, vec4(filtBuff2ColorLow, colorLow.a));
  imageStore(img_filterBuff3, xy, vec4(filtBuff2ColorMain, colorLow.a));

  discard;
}

const float BLACK_CUTOFF = 0.03;

bool NotCloseToBlack(vec3 color)
{
  return (color.r > BLACK_CUTOFF) || (color.r != color.g) || (color.r != color.b);
}

// Try to get purer blacks by using a lower baseColorMultiplier for small grey values.
float GetBaseColorMultiplier(vec3 color)
{
  const float LOW_BASE_COLOR_MULTIPLIER = 0.25;

  return NotCloseToBlack(color) ? u_baseColorMultiplier
                                : mix(LOW_BASE_COLOR_MULTIPLIER,
                                      u_baseColorMultiplier,
                                      pow(color.r / BLACK_CUTOFF, 3.0));
}

float sdf_smin(float a, float b)
{
    const float k = 32;
    float res = exp(-k*a) + exp(-k*b);
    return -log(max(0.0001,res)) / k;
}

struct SrceAndDestPositions
{
  vec2 srcePos1;
  vec2 destPos1;
  vec2 srcePos2;
  vec2 destPos2;
};
struct LerpedPositions
{
  vec2 pos1;
  vec2 pos2;
};
struct TexelPositions
{
  vec2 uv1;
  vec2 uv2;
};

SrceAndDestPositions GetSrceAndDestPositions(ivec2 xy)
{
  return SrceAndDestPositions(
    imageLoad(img_filterSrcePosBuff1, xy).xy,
    imageLoad(img_filterDestPosBuff1, xy).xy,
    imageLoad(img_filterSrcePosBuff2, xy).xy,
    imageLoad(img_filterDestPosBuff2, xy).xy
  );
}

LerpedPositions GetLerpedPositions(ivec2 xy)
{
  SrceAndDestPositions normalizedPositions = GetSrceAndDestPositions(xy);

  return LerpedPositions(
    mix(normalizedPositions.srcePos1, normalizedPositions.destPos1, u_lerpFactor),
    mix(normalizedPositions.srcePos2, normalizedPositions.destPos2, u_lerpFactor)
  );
}

vec2 GetTexelPos(vec2 filterPos)
{
  float x = (filterPos.x - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH;
  float y = (filterPos.y - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH;

  return vec2(x, 1 - (ASPECT_RATIO * y));
}

TexelPositions GetTexelPositions(LerpedPositions lerpedPositions)
{
  return TexelPositions(
    GetTexelPos(lerpedPositions.pos1),
    GetTexelPos(lerpedPositions.pos2)
  );
}

vec4 GetPosMappedFilterBuff2Value(vec2 uv, ivec2 xy)
{
  xy = ivec2(xy.x, HEIGHT - 1 - xy.y);

  // u_time example use 1.
  // vec2 focusPoint = 0.5 * (1.0 + vec2(sin(0.01*u_time), cos(0.01*u_time)));
  // float d = length(destNormalizedPos - focusPoint);
  // destNormalizedPos *= 1.0 - 0.5*d;
  // float FREQ_FACTOR = (1.0 + (0.5*(1.0 + sin(0.01 * u_time))))*1.1;
  // destNormalizedPos = vec2(fract(FREQ_FACTOR * destNormalizedPos.x),
  //                          fract(FREQ_FACTOR * destNormalizedPos.y));
  // const float FREQ_FACTOR = 20.1;
  // destNormalizedPos = vec2((1.0 - 0.5*0.5*(1.0 + sin(FREQ_FACTOR*u_time))) * destNormalizedPos.x,
  //                          (1.0 - 0.5*0.5*(1.0 + cos(FREQ_FACTOR*u_time))) * destNormalizedPos.y);

  LerpedPositions lerpedPositions = GetLerpedPositions(xy);

  if (u_resetSrceFilterPosBuffers)
  {
    // Reset the filter srce pos buffers to the current lerped state, ready
    // for a new filter dest pos buffer.
    imageStore(img_filterSrcePosBuff1, xy, vec4(lerpedPositions.pos1, 0, 0));
    imageStore(img_filterSrcePosBuff2, xy, vec4(lerpedPositions.pos2, 0, 0));
  }

  // u_time example use 2.
  // const float FREQ_FACTOR = 0.01;
  // float amp = 0.5 * 0.5*(1.0 + sin(0.1*u_time));
  // vec2 newMidPoint = amp * vec2(0.5*(1.0 + sin(FREQ_FACTOR*u_time)), 0.5*(1.0 + cos(FREQ_FACTOR*u_time)));
  // lerpNormalizedPos -= newMidPoint;

  TexelPositions texelPositions = GetTexelPositions(lerpedPositions);

  /**
  const vec2 t1 = vec2(0.5 * (1.0 + sin(u_pos1Pos2MixFreq * u_time)));
  vec2 filtBuff2Pos = mix(filtBuff2Pos1, filtBuff2Pos2, t1);
  return texture(tex_filterBuff2, vec2(filtBuff2Pos.x, 1 - (ASPECT_RATIO * filtBuff2Pos.y)));
  **/

MAKE A FUNC
  vec4 filtBuff2Color1 = texture(tex_filterBuff2, texelPositions.uv1);
  vec4 filtBuff2Color2 = texture(tex_filterBuff2, texelPositions.uv2);

  const float r = 0.25;
  const vec2 circleCentre1 = 0.5 + vec2(r*cos(u_pos1Pos2MixFreq * u_time), r*sin(u_pos1Pos2MixFreq * u_time));
  const vec2 circleCentre2 = 0.4 + vec2(r*cos(1+u_pos1Pos2MixFreq * u_time), r*sin(1+u_pos1Pos2MixFreq * u_time));
  float dist1 = distance(uv, circleCentre1);
  float dist2 = distance(uv, circleCentre2) - 0.2;
  //vec3 t = vec3(step(0.0, sdf_smin(dist1, dist2)));
  //vec3 t = vec3(step(0.5, uv.x));
  //float posDist = distance(filtBuff2Pos1, filtBuff2Pos2)/5.5;
  //float posDist = distance(vec2(0), filtBuff2Pos2)/5.5;

  const vec3 t = vec3(0.5 * (1.0 + sin(u_pos1Pos2MixFreq * u_time)));
  //const float t = 0.5;
  // const float t = step(100, u_time % 200);

  vec2 posUv = mix(texelPositions.uv1, texelPositions.uv2, vec2(t.x));
  //float distUv = distance(uv, posUv) / sqrt(2);
  float distUv = distance(uv, texelPositions.uv2) / sqrt(2);
  float brightness = mix(1.0, 1.01, distUv);
  //brightness = 1.0;

  //vec3 color = pow(mix(filtBuff2Color1.rgb, filtBuff2Color2.rgb, t), vec3(brightness));
  //vec3 color = mix(filtBuff2Color1.rgb, filtBuff2Color2.rgb, t*(1-distUv));
  vec3 color = mix(filtBuff2Color1.rgb, filtBuff2Color2.rgb, t);
  //vec3 color = mix(filtBuff2Color1.rgb, filtBuff2Color1.rgb, t);
  //vec3 color = mix(filtBuff2Color2.rgb, filtBuff2Color2.rgb, t);

  return vec4(color, filtBuff2Color1.a);
  //return vec4(color, filtBuff2Color2.a);
}
