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
static const std::vector<vivid::srgb_t> Acton_10
{
  {   0.18039f,   0.12941f,   0.30196f },
  {   0.29412f,   0.23137f,   0.40000f },
  {   0.43137f,   0.32941f,   0.50196f },
  {   0.57255f,   0.38824f,   0.56471f },
  {   0.69804f,   0.40392f,   0.58431f },
  {   0.81961f,   0.48235f,   0.64706f },
  {   0.83137f,   0.58431f,   0.72157f },
  {   0.83137f,   0.67843f,   0.78824f },
  {   0.85882f,   0.78824f,   0.86275f },
  {   0.90196f,   0.90196f,   0.94118f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif