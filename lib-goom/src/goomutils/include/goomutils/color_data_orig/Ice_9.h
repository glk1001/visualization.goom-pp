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
static const std::vector<vivid::srgb_t> Ice_9
{
  {   0.01569f,   0.02353f,   0.07451f },
  {   0.12941f,   0.12549f,   0.25490f },
  {   0.21961f,   0.22353f,   0.45882f },
  {   0.24706f,   0.34118f,   0.63922f },
  {   0.25882f,   0.48235f,   0.71765f },
  {   0.34118f,   0.61176f,   0.76471f },
  {   0.47451f,   0.74510f,   0.81569f },
  {   0.68627f,   0.86667f,   0.88235f },
  {   0.91765f,   0.99216f,   0.99216f },
};
// clang-format on

#if __cplusplus > 201402L
} // namespace GOOM::UTILS::COLOR_DATA
#else
} // namespace COLOR_DATA
} // namespace UTILS
} // namespace GOOM
#endif