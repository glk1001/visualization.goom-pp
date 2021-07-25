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
static const std::vector<vivid::srgb_t> Spectral_3
{
  {   0.98824f,   0.55294f,   0.34902f },
  {   1.00000f,   1.00000f,   0.74902f },
  {   0.60000f,   0.83529f,   0.58039f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif