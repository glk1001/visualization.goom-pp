module;

#include <algorithm>
#include <cstdint>

export module Goom.Utils.Graphics.PixelUtils;

import Goom.Utils.Math.Misc;
import Goom.Lib.GoomGraphic;

export namespace GOOM::UTILS::GRAPHICS
{

inline constexpr auto CHANNEL_COLOR_SCALAR_DIVISOR = channel_limits<uint32_t>::max() + 1U;

[[nodiscard]] constexpr auto MakePixel(uint32_t red,
                                       uint32_t green,
                                       uint32_t blue,
                                       uint32_t alpha) noexcept -> Pixel;
[[nodiscard]] constexpr auto MakePixel(float red, float green, float blue, float alpha) noexcept
    -> Pixel;

[[nodiscard]] constexpr auto GetPixelWithNewAlpha(const Pixel& pixel, PixelChannelType newAlpha)
    -> Pixel;

[[nodiscard]] constexpr auto GetColorAdd(const Pixel& color1,
                                         const Pixel& color2,
                                         PixelChannelType newAlpha) -> Pixel;
[[nodiscard]] constexpr auto GetColorChannelAdd(PixelChannelType ch1, PixelChannelType ch2)
    -> uint32_t;

[[nodiscard]] constexpr auto GetColorMultiply(const Pixel& color1,
                                              const Pixel& color2,
                                              PixelChannelType newAlpha) -> Pixel;
[[nodiscard]] constexpr auto GetColorChannelMultiply(PixelChannelType ch1,
                                                     PixelChannelType ch2) noexcept -> uint32_t;
[[nodiscard]] constexpr auto GetChannelColorMultiplyByScalar(uint32_t scalar,
                                                             PixelChannelType channelVal) noexcept
    -> uint32_t;

[[nodiscard]] constexpr auto GetColorMin(const Pixel& color1,
                                         const Pixel& color2,
                                         PixelChannelType newAlpha) -> Pixel;
[[nodiscard]] constexpr auto GetColorMax(const Pixel& color1,
                                         const Pixel& color2,
                                         PixelChannelType newAlpha) -> Pixel;

} // namespace GOOM::UTILS::GRAPHICS

namespace GOOM::UTILS::GRAPHICS
{

constexpr auto MakePixel(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha) noexcept
    -> Pixel
{
  red   = std::min(red, MAX_CHANNEL_VALUE_HDR);
  green = std::min(green, MAX_CHANNEL_VALUE_HDR);
  blue  = std::min(blue, MAX_CHANNEL_VALUE_HDR);
  alpha = std::min<uint32_t>(alpha, MAX_ALPHA);

  return Pixel{static_cast<PixelChannelType>(red),
               static_cast<PixelChannelType>(green),
               static_cast<PixelChannelType>(blue),
               static_cast<PixelChannelType>(alpha)};
}

constexpr auto MakePixel(const float red,
                         const float green,
                         const float blue,
                         const float alpha) noexcept -> Pixel
{
  constexpr auto MAX_CHANNEL_VALUE = channel_limits<float>::max();

  return MakePixel(static_cast<uint32_t>(red * MAX_CHANNEL_VALUE),
                   static_cast<uint32_t>(green * MAX_CHANNEL_VALUE),
                   static_cast<uint32_t>(blue * MAX_CHANNEL_VALUE),
                   static_cast<uint32_t>(alpha * MAX_ALPHA));
}

constexpr auto GetPixelWithNewAlpha(const Pixel& pixel, const PixelChannelType newAlpha) -> Pixel
{
  return Pixel{pixel.R(), pixel.G(), pixel.B(), newAlpha};
}

constexpr auto GetColorAdd(const Pixel& color1,
                           const Pixel& color2,
                           const PixelChannelType newAlpha) -> Pixel
{
  const auto newR = GetColorChannelAdd(color1.R(), color2.R());
  const auto newG = GetColorChannelAdd(color1.G(), color2.G());
  const auto newB = GetColorChannelAdd(color1.B(), color2.B());

  return UTILS::GRAPHICS::MakePixel(newR, newG, newB, newAlpha);
}

constexpr auto GetColorChannelAdd(const PixelChannelType ch1, const PixelChannelType ch2)
    -> uint32_t
{
  return static_cast<uint32_t>(ch1) + static_cast<uint32_t>(ch2);
}

constexpr auto GetColorMultiply(const Pixel& color1,
                                const Pixel& color2,
                                const PixelChannelType newAlpha) -> Pixel
{
  const auto newR = static_cast<PixelChannelType>(GetColorChannelMultiply(color1.R(), color2.R()));
  const auto newG = static_cast<PixelChannelType>(GetColorChannelMultiply(color1.G(), color2.G()));
  const auto newB = static_cast<PixelChannelType>(GetColorChannelMultiply(color1.B(), color2.B()));

  return Pixel{newR, newG, newB, newAlpha};
}

constexpr auto GetColorChannelMultiply(const PixelChannelType ch1,
                                       const PixelChannelType ch2) noexcept -> uint32_t
{
  return (static_cast<uint32_t>(ch1) * static_cast<uint32_t>(ch2)) /
         channel_limits<uint32_t>::max();
}

constexpr auto GetChannelColorMultiplyByScalar(const uint32_t scalar,
                                               const PixelChannelType channelVal) noexcept
    -> uint32_t
{
  constexpr auto CHANNEL_COLOR_SCALAR_DIVISOR_EXP = UTILS::MATH::Log2(CHANNEL_COLOR_SCALAR_DIVISOR);

  return (scalar * static_cast<uint32_t>(channelVal)) >> CHANNEL_COLOR_SCALAR_DIVISOR_EXP;
}

constexpr auto GetColorMin(const Pixel& color1,
                           const Pixel& color2,
                           const PixelChannelType newAlpha) -> Pixel
{
  const auto minR = std::min(color1.R(), color2.R());
  const auto minG = std::min(color1.G(), color2.G());
  const auto minB = std::min(color1.B(), color2.B());

  return Pixel{minR, minG, minB, newAlpha};
}

constexpr auto GetColorMax(const Pixel& color1,
                           const Pixel& color2,
                           const PixelChannelType newAlpha) -> Pixel
{
  const auto maxR = std::max(color1.R(), color2.R());
  const auto maxG = std::max(color1.G(), color2.G());
  const auto maxB = std::max(color1.B(), color2.B());

  return Pixel{maxR, maxG, maxB, newAlpha};
}

} // namespace GOOM::UTILS::GRAPHICS
