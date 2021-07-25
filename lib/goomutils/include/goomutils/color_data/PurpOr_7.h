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
static const std::vector<vivid::srgb_t> PurpOr_7
{
  {   0.97647f,   0.86667f,   0.85490f },
  {   0.94902f,   0.72549f,   0.76863f },
  {   0.89804f,   0.59216f,   0.72549f },
  {   0.80784f,   0.47059f,   0.70196f },
  {   0.67843f,   0.37255f,   0.67843f },
  {   0.51373f,   0.29412f,   0.62745f },
  {   0.34118f,   0.23137f,   0.53333f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif