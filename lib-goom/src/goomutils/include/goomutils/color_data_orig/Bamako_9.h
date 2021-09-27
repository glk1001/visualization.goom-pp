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
static const std::vector<vivid::srgb_t> Bamako_9
{
  {   0.00000f,   0.25098f,   0.29804f },
  {   0.08235f,   0.29804f,   0.25490f },
  {   0.16863f,   0.35294f,   0.20392f },
  {   0.26275f,   0.41569f,   0.14510f },
  {   0.38039f,   0.49412f,   0.07843f },
  {   0.52941f,   0.55686f,   0.01176f },
  {   0.72549f,   0.64706f,   0.14510f },
  {   0.89020f,   0.78824f,   0.38039f },
  {   1.00000f,   0.89804f,   0.60000f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif