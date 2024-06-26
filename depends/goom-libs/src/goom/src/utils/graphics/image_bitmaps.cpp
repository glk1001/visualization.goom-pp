module;

#include <cstddef>
#include <cstdint>
#include <exception>
#include <format>
#include <stdexcept>
#include <string>
#include <tuple>

#define STB_IMAGE_IMPLEMENTATION
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4068) // unknown pragma 'GCC'
#pragma warning(disable : 4296) // '>=': expression is always true
#endif
#include "stb_image.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

module Goom.Utils.Graphics.ImageBitmaps;

import Goom.Lib.GoomTypes;

namespace GOOM::UTILS::GRAPHICS
{

auto ImageBitmap::Resize(const Dimensions& dimensions) noexcept -> void
{
  m_width  = dimensions.GetWidth();
  m_height = dimensions.GetHeight();
  m_owningBuff.resize(static_cast<size_t>(m_width) * static_cast<size_t>(m_height));
}

inline auto ImageBitmap::SetPixel(const size_t x, const size_t y, const RGB& pixel) noexcept -> void
{
  m_owningBuff.at((y * m_width) + x) = pixel;
}

auto ImageBitmap::Load(const std::string& imageFilename) -> void
{
  m_filename                                = imageFilename;
  const auto [rgbImage, width, height, bpp] = GetRGBImage();

  const auto* rgbPtr = rgbImage;
  Resize({static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
  for (auto y = 0U; y < GetHeight(); ++y)
  {
    for (auto x = 0U; x < GetWidth(); ++x)
    {
      //NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic): stb_image requirement
      const auto blue = *rgbPtr;
      ++rgbPtr;
      const auto green = *rgbPtr;
      ++rgbPtr;
      const auto red = *rgbPtr;
      ++rgbPtr;
      const auto alpha = *rgbPtr;
      ++rgbPtr;
      //NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic): stb_image requirement

      if (0 == alpha)
      {
        SetPixel(x, y, RGB{});
      }
      else
      {
        SetPixel(x, y, RGB{.red = red, .green = green, .blue = blue, .alpha = alpha});
      }
    }
  }

  ::stbi_image_free(rgbImage);
}

auto ImageBitmap::GetRGBImage() const -> std::tuple<uint8_t*, int32_t, int32_t, int32_t>
{
  try
  {
    static constexpr auto DESIRED_CHANNELS = 4;

    auto width     = 0;
    auto height    = 0;
    auto bpp       = 0;
    auto* rgbImage = ::stbi_load(m_filename.c_str(), &width, &height, &bpp, DESIRED_CHANNELS);
    if (!rgbImage)
    {
      throw std::runtime_error(std::format(R"(Could not load image file "{}".)", m_filename));
    }
    if ((0 == width) or (0 == height) or (0 == bpp))
    {
      throw std::runtime_error(
          std::format("Error loading image \"{}\". width = {}, height = {}, bpp = {}.",
                      m_filename,
                      width,
                      height,
                      bpp));
    }

    return {rgbImage, width, height, bpp};
  }
  catch (const std::exception& e)
  {
    throw std::runtime_error(
        std::format(R"(Could not load image file "{}". Exception: "{}".)", m_filename, e.what()));
  }
}

} // namespace GOOM::UTILS::GRAPHICS
