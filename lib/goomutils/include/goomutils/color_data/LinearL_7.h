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
static const std::vector<vivid::srgb_t> LinearL_7
{
  {   0.01569f,   0.01569f,   0.01569f },
  {   0.17255f,   0.11373f,   0.39216f },
  {   0.03922f,   0.33333f,   0.47843f },
  {   0.00000f,   0.55686f,   0.24706f },
  {   0.30980f,   0.73725f,   0.00000f },
  {   0.88627f,   0.81176f,   0.54118f },
  {   1.00000f,   1.00000f,   1.00000f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif