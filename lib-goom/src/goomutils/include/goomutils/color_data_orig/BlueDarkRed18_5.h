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
static const std::vector<vivid::srgb_t> BlueDarkRed18_5
{
  {   0.14118f,   0.00000f,   0.84706f },
  {   0.33725f,   0.69020f,   1.00000f },
  {   0.91765f,   1.00000f,   1.00000f },
  {   1.00000f,   0.47059f,   0.33725f },
  {   0.64706f,   0.00000f,   0.12941f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif