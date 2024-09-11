#version 430

#include "pass4_reset_filter_buff2_and_output_buff3_consts.h"
#include "tone-maps.glsl"

layout(location = 0) out vec4 fragColor;

// 'img_lowColorsBuff' has this frames' mapped color and current low color,
// which is persisted to 'img_persistentColorsBuff'.
layout(binding=LOW_COLORS_BUFF_IMAGE_UNIT, rgba16f) uniform readonly image2D img_lowColorsBuff;
layout(binding=PERSISTENT_COLORS_BUFF_IMAGE_UNIT, rgba16f) uniform image2D img_persistentColorsBuff;

// 'img_mainColorsBuff' has this frames' mapped color and current main color. Not persisted.
layout(binding=MAIN_COLORS_BUFF_IMAGE_UNIT, rgba16f) uniform readonly image2D img_mainColorsBuff;

// Luminance values for exposure correction.
layout(binding=LUM_AVG_IMAGE_UNIT, r16f) uniform readonly image2D img_lumAvg;

uniform float u_brightness;
uniform float u_brightnessAdjust;
uniform float u_hueShift;
uniform float u_chromaFactor;

vec3 GetHueShift(const vec3 color);
float GetChromaticIncrease(const float chroma);
vec3 RGBtoHCY(const vec3 RGB);
vec3 HCYtoRGB(vec3 HCY);
float GetFinalExposure(const float brightness, const float averageLuminance);

void main()
{
    const ivec2 deviceXY = ivec2(gl_FragCoord.xy);

    // Get the hdr color to work with.
    const vec4 mainColorVal = imageLoad(img_mainColorsBuff, deviceXY);
    const vec4 hdrColor       = mainColorVal;

    // Copy the low colors buffer to the persistent colors buffer, ready for the next frame.
    const vec4 lowColorVal = imageLoad(img_lowColorsBuff, deviceXY);
    imageStore(img_persistentColorsBuff, deviceXY, lowColorVal);

    // Convert to HCY.
    vec3 hcy = RGBtoHCY(hdrColor.rgb);

    // Apply the chromatic increase.
    hcy.y = GetChromaticIncrease(hcy.y);

    // Apply the exposure correction.
    const float averageLuminance = imageLoad(img_lumAvg, ivec2(0, 0)).x;
    hcy.z *= GetFinalExposure(u_brightnessAdjust * u_brightness, averageLuminance);

    // Convert back to RGB.
    vec3 finalColor = HCYtoRGB(hcy);

    // Apply the hue shift.
    finalColor = GetHueShift(finalColor);

    // Finish with the tone mapping and baked in gamma.
    finalColor = GetToneMappedColor(finalColor);

    fragColor = vec4(finalColor, 1.0F);
}

float GetFinalExposure(const float brightness, const float averageLuminance)
{
    return brightness / ((9.6 * averageLuminance) + 0.0001);
}

vec3 GetHueShift(const vec3 color)
{
    //  const vec3 k   = vec3(0.57735, 0.57735, 0.57735);
    //  float cosAngle = cos(u_hueShift);
    //  float sinAngle = sin(u_hueShift);
    //  return vec3((color * cosAngle) + (sinAngle * cross(k, color)) +
    //              (dot(k, color) * k * (1.0 - cosAngle)));

    // http://beesbuzz.biz/code/16-hsv-color-transforms
    //  float cosAngle = cos(u_hueShift);
    //  float sinAngle = sin(u_hueShift);
    //
    //  vec3 hueShiftedColor;
    //
    //  hueShiftedColor.r = (0.299 + 0.701 * cosAngle + 0.168 * sinAngle) * color.r +
    //                      (0.587 - 0.587 * cosAngle + 0.330 * sinAngle) * color.g +
    //                      (0.114 - 0.114 * cosAngle - 0.497 * sinAngle) * color.b;
    //
    //  hueShiftedColor.g = (0.299 - 0.299 * cosAngle - 0.328 * sinAngle) * color.r +
    //                      (0.587 + 0.413 * cosAngle + 0.035 * sinAngle) * color.g +
    //                      (0.114 - 0.114 * cosAngle + 0.292 * sinAngle) * color.b;
    //
    //  hueShiftedColor.b = (0.299 - 0.300 * cosAngle + 1.250 * sinAngle) * color.r +
    //                      (0.587 - 0.588 * cosAngle - 1.050 * sinAngle) * color.g +
    //                      (0.114 + 0.886 * cosAngle - 0.203 * sinAngle) * color.b;
    //
    //  return hueShiftedColor;

    // https://gist.github.com/mairod/a75e7b44f68110e1576d77419d608786 - vmedea  comment
    const float cosAngle = cos(u_hueShift);
    const float sinAngle = sin(u_hueShift);
    const mat3 T         = mat3(vec3(+0.167444, +0.329213, -0.496657),
                              vec3(-0.327948, +0.035669, +0.292279),
                              vec3(+1.250268, -1.047561, -0.202707));

    return (color * cosAngle) + ((color * sinAngle) * T) +
           (dot(vec3(0.299, 0.587, 0.114), color) * (1.0 - cosAngle));
}

float GetChromaticIncrease(const float chroma)
{
    // 'Chromatic Increase' - https://github.com/gurki/vivid
    return min(u_chromaFactor * chroma, 140.0);
}

// ========================================================
// RGB to HCY - from https://www.chilliant.com/rgb2hsv.html

#ifndef saturate
#define saturate(v) clamp(v, 0, 1)
#endif

vec3 HUEtoRGB(const float H)
{
    const float R = abs(H * 6 - 3) - 1;
    const float G = 2 - abs(H * 6 - 2);
    const float B = 2 - abs(H * 6 - 4);
    return saturate(vec3(R, G, B));
}

const float Epsilon = 1e-10;

vec3 RGBtoHCV(const vec3 RGB)
{
    // Based on work by Sam Hocevar and Emil Persson
    const vec4 P  = (RGB.g < RGB.b) ? vec4(RGB.bg, -1.0, 2.0 / 3.0) : vec4(RGB.gb, 0.0, -1.0 / 3.0);
    const vec4 Q  = (RGB.r < P.x) ? vec4(P.xyw, RGB.r) : vec4(RGB.r, P.yzx);
    const float C = Q.x - min(Q.w, Q.y);
    const float H = abs((Q.w - Q.y) / (6 * C + Epsilon) + Q.z);
    return vec3(H, C, Q.x);
}

// The weights of RGB contributions to luminance.
// Should sum to unity.
const vec3 HCYwts = vec3(0.299, 0.587, 0.114);

vec3 RGBtoHCY(const vec3 RGB)
{
    // Corrected by David Schaeffer
    vec3 HCV       = RGBtoHCV(RGB);
    const float Y  = dot(RGB, HCYwts);
    const float Z  = dot(HUEtoRGB(HCV.x), HCYwts);
    if (Y < Z)
    {
        HCV.y *= Z / (Epsilon + Y);
    }
    else
    {
        HCV.y *= (1 - Z) / (Epsilon + 1 - Y);
    }
    return vec3(HCV.x, HCV.y, Y);
}

vec3 HCYtoRGB(vec3 HCY)
{
    const vec3 RGB = HUEtoRGB(HCY.x);
    const float Z  = dot(RGB, HCYwts);
    if (HCY.z < Z)
    {
        HCY.y *= HCY.z / Z;
    }
    else if (Z < 1)
    {
        HCY.y *= (1 - HCY.z) / (1 - Z);
    }
    return (RGB - Z) * HCY.y + HCY.z;
}
