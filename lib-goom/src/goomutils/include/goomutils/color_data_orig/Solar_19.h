#pragma once

#include "vivid/types.h"
#include <vector>

#if __cplusplus > 201402L
namespace GOOM::UTILS::COLOR_DATA
{
#else
namespace GOOM
{
namespace UTILS
{
namespace COLOR_DATA
{
#endif

// clang-format off
static const std::vector<vivid::srgb_t> Solar_19
{
  {   0.20000f,   0.07843f,   0.09412f },
  {   0.27059f,   0.10196f,   0.11765f },
  {   0.33725f,   0.12157f,   0.13725f },
  {   0.40784f,   0.13725f,   0.14510f },
  {   0.47843f,   0.16078f,   0.14118f },
  {   0.54118f,   0.19216f,   0.12549f },
  {   0.59216f,   0.23137f,   0.10980f },
  {   0.63922f,   0.28235f,   0.09412f },
  {   0.67843f,   0.33333f,   0.08235f },
  {   0.71373f,   0.39216f,   0.07451f },
  {   0.74510f,   0.45098f,   0.07451f },
  {   0.77255f,   0.50980f,   0.08235f },
  {   0.79608f,   0.57255f,   0.10196f },
  {   0.81961f,   0.63529f,   0.12549f },
  {   0.83922f,   0.70196f,   0.15294f },
  {   0.85490f,   0.76863f,   0.18431f },
  {   0.86667f,   0.84314f,   0.21961f },
  {   0.87843f,   0.91765f,   0.25490f },
  {   0.88235f,   0.99216f,   0.29412f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif