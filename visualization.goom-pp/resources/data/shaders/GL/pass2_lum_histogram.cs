#version 430

// Adapted from "https://bruop.github.io/exposure/"

#include "pass2_lum_histogram_consts.h"

#define GROUP_SIZE 256
#define THREADS_X 16
#define THREADS_Y 16

#define EPSILON 0.005
// Taken from RTR vol 4 pg. 278
#define RGB_TO_LUM vec3(0.2125, 0.7154, 0.0721)

uniform vec4 u_params;
// @formatter:off
// clang-format off
#define MIN_LOG_LUM           u_params.x
#define INVERSE_LOG_LUM_RANGE u_params.y
#define WIDTH                 u_params.z
#define HEIGHT                u_params.w
// clang-format on
// @formatter:on

// @formatter:off
// clang-format off
layout(binding=LOW_COLORS_BUFF_IMAGE_UNIT, rgba16f) uniform readonly image2D img_input;
layout(binding=LUM_HISTOGRAM_BUFFER_INDEX, std430)                   buffer  buff_histogram
// clang-format on
// @formatter:on
{
  uint histogram[];
};

// Shared histogram buffer used for storing intermediate sums for each work group.
shared uint histogramShared[GROUP_SIZE];

layout(local_size_x = THREADS_X, local_size_y = THREADS_Y) in;

uint colorToBin(const vec3 hdrColor, const float minLogLum, const float inverseLogLumRange);

void main()
{
  // Initialize the bin for this thread to 0.
  histogramShared[gl_LocalInvocationIndex] = 0;

  barrier();

  const uvec2 dim = uvec2(u_params.zw);
  // Ignore threads that map to areas beyond the bounds of our HDR image.
  if ((gl_GlobalInvocationID.x < dim.x) && (gl_GlobalInvocationID.y < dim.y))
  {
    const vec3 hdrColor = imageLoad(img_input, ivec2(gl_GlobalInvocationID.xy)).rgb;
    const uint binIndex = colorToBin(hdrColor, MIN_LOG_LUM, INVERSE_LOG_LUM_RANGE);
    // We use an atomic add to ensure we don't write to the same bin in our
    // histogram from two different threads at the same time.
    atomicAdd(histogramShared[binIndex], 1);
  }

  // Wait for all threads in the work group to reach this point before adding
  // our local histogram to the global one.
  barrier();

  // Technically there's no chance that two threads write to the same bin here,
  // but different work groups might! So we still need the atomic add.
  atomicAdd(histogram[gl_LocalInvocationIndex], histogramShared[gl_LocalInvocationIndex]);
}


// For a given color and luminance range, return the histogram bin index.
uint colorToBin(const vec3 hdrColor, const float minLogLum, const float inverseLogLumRange)
{
  // Convert our RGB value to Luminance, see note for RGB_TO_LUM macro above.
  const float lum = dot(hdrColor, RGB_TO_LUM);
  // Avoid taking the log of zero.
  if (lum < EPSILON)
  {
    return 0;
  }

  // Calculate the log_2 luminance and express it as a value in [0.0, 1.0]
  // where 0.0 represents the minimum luminance, and 1.0 represents the max.
  const float logLum = clamp((log2(lum) - minLogLum) * inverseLogLumRange, 0.0, 1.0);

  // Map [0, 1] to [1, 255]. The zeroth bin is handled by the epsilon check above.
  return uint((logLum * 254.0) + 1.0);
}
