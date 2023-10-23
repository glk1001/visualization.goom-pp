#version 430

#include "pass1_update_filter_buff1_and_buff3_consts.h"

//#include "glsl-blend/all.glsl"

uniform sampler2D tex_filterBuff2;
uniform sampler2D tex_filterSrcePositions;
uniform sampler2D tex_filterSrcePositions2;
uniform sampler2D tex_filterDestPositions;
uniform sampler2D tex_filterDestPositions2;
uniform sampler2D tex_mainImage;
uniform sampler2D tex_lowImage;

layout(binding = FILTER_BUFF1_IMAGE_UNIT, rgba16f) uniform image2D img_filterBuff1;
layout(binding = FILTER_BUFF3_IMAGE_UNIT, rgba16f) uniform image2D img_filterBuff3;

in vec3 position;
in vec2 texCoord;

uniform float u_lerpFactor;
uniform float u_buff2Buff3Mix = 0.1;
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

vec4 GetPosMappedFilterBuff2Value(vec2 uv);
float GetBaseColorMultiplier(vec3 color);

void main()
{
  vec4 filtBuff2Val = GetPosMappedFilterBuff2Value(texCoord);

  ivec2 xy          = ivec2(gl_FragCoord.xy);
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

vec4 GetPosMappedFilterBuff2Value(vec2 uv)
{
  vec2 srceNormalizedPos1 = texture(tex_filterSrcePositions, uv).xy;
  vec2 destNormalizedPos1 = texture(tex_filterDestPositions, uv).xy;

  vec2 srceNormalizedPos2 = texture(tex_filterSrcePositions2, uv).xy;
  vec2 destNormalizedPos2 = texture(tex_filterDestPositions2, uv).xy;

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

  vec2 lerpNormalizedPos1 = mix(srceNormalizedPos1, destNormalizedPos1, u_lerpFactor);
  vec2 lerpNormalizedPos2 = mix(srceNormalizedPos2, destNormalizedPos2, u_lerpFactor);

  // u_time example use 2.
  // const float FREQ_FACTOR = 0.01;
  // float amp = 0.5 * 0.5*(1.0 + sin(0.1*u_time));
  // vec2 newMidPoint = amp * vec2(0.5*(1.0 + sin(FREQ_FACTOR*u_time)), 0.5*(1.0 + cos(FREQ_FACTOR*u_time)));
  // lerpNormalizedPos -= newMidPoint;

  vec2 filtBuff2Pos1 = vec2((lerpNormalizedPos1.x - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH,
                            (lerpNormalizedPos1.y - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH);
  vec2 filtBuff2Pos2 = vec2((lerpNormalizedPos2.x - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH,
                            (lerpNormalizedPos2.y - FILTER_POS_MIN_COORD) / FILTER_POS_COORD_WIDTH);


  //  vec4 tex = texture(tex_lowImage, vec2(filtBuff2Pos.x, 1 - (ASPECT_RATIO * filtBuff2Pos.y)));
  //  return vec4(tex.x, tex.y, filtBuff2Pos.x, 1 - (ASPECT_RATIO * filtBuff2Pos.y));

  //  vec4 tex = texture(tex_filterBuff2, vec2(filtBuff2Pos.x, 1 - (ASPECT_RATIO * filtBuff2Pos.y)));
  //  return vec4(tex.x, tex.y, uv.x, uv.y);

//  return texture(tex_filterBuff2, vec2(filtBuff2Pos.x, 1 - (ASPECT_RATIO * filtBuff2Pos.y)));

  vec4 filtBuff2Color1 = texture(tex_filterBuff2, vec2(filtBuff2Pos1.x, 1 - (ASPECT_RATIO * filtBuff2Pos1.y)));
  vec4 filtBuff2Color2 = texture(tex_filterBuff2, vec2(filtBuff2Pos2.x, 1 - (ASPECT_RATIO * filtBuff2Pos2.y)));

  const float freq = 0.01;
  const float t = 0.5 * (1.0 + sin(freq * u_time));
  //const float t = 0.5;
  // const float t = step(100, u_time % 200);
  vec3 color = mix(filtBuff2Color1.rgb, filtBuff2Color2.rgb, vec3(t));
  //vec3 color = mix(filtBuff2Color1.rgb, filtBuff2Color1.rgb, vec3(t));
  //vec3 color = mix(filtBuff2Color2.rgb, filtBuff2Color2.rgb, vec3(t));

  return vec4(color, filtBuff2Color1.a);
  //return vec4(color, filtBuff2Color2.a);
}
