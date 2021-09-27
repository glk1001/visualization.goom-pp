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
static const std::vector<vivid::srgb_t> Delta_7
{
  {   0.06667f,   0.12549f,   0.25098f },
  {   0.10980f,   0.40392f,   0.62745f },
  {   0.42353f,   0.70980f,   0.70196f },
  {   1.00000f,   0.99216f,   0.80392f },
  {   0.66667f,   0.67451f,   0.12549f },
  {   0.09412f,   0.45098f,   0.15686f },
  {   0.09020f,   0.13725f,   0.07451f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif