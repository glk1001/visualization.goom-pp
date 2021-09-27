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
static const std::vector<vivid::srgb_t> Gray_9
{
  {   0.00000f,   0.00000f,   0.00000f },
  {   0.10196f,   0.09804f,   0.09804f },
  {   0.21569f,   0.21569f,   0.21569f },
  {   0.32941f,   0.32941f,   0.32549f },
  {   0.44706f,   0.44314f,   0.44314f },
  {   0.56471f,   0.56078f,   0.56078f },
  {   0.69412f,   0.69412f,   0.69020f },
  {   0.83922f,   0.83922f,   0.83137f },
  {   1.00000f,   1.00000f,   0.99216f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif