#pragma once

#include "draw_methods.h"
#include "goom_graphic.h"
#include "utils/parallel_utils.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace GOOM
{

namespace UTILS
{
class ImageBitmap;
} // namespace UTILS

namespace DRAW
{

class IGoomDraw
{
public:
  IGoomDraw() noexcept = delete;
  IGoomDraw(uint32_t screenWidth, uint32_t screenHeight, const DrawPixelFunc& drawPixelFunc);
  IGoomDraw(const IGoomDraw&) noexcept = delete;
  IGoomDraw(IGoomDraw&&) noexcept = delete;
  virtual ~IGoomDraw() noexcept = default;
  auto operator=(const IGoomDraw&) -> IGoomDraw& = delete;
  auto operator=(IGoomDraw&&) -> IGoomDraw& = delete;

  [[nodiscard]] auto GetScreenWidth() const -> uint32_t;
  [[nodiscard]] auto GetScreenHeight() const -> uint32_t;

  [[nodiscard]] auto GetAllowOverexposed() const -> bool;
  void SetAllowOverexposed(bool val);

  [[nodiscard]] auto GetBuffIntensity() const -> float;
  void SetBuffIntensity(float val);

  void Circle(int x0, int y0, int radius, const Pixel& color);
  void Circle(int x0, int y0, int radius, const std::vector<Pixel>& colors);

  void Line(int x1, int y1, int x2, int y2, const Pixel& color, uint8_t thickness);
  void Line(int x1, int y1, int x2, int y2, const std::vector<Pixel>& colors, uint8_t thickness);

  using GetBitmapColorFunc = std::function<Pixel(size_t x, size_t y, const Pixel& imageColor)>;
  void Bitmap(int xCentre,
              int yCentre,
              const UTILS::ImageBitmap& bitmap,
              const GetBitmapColorFunc& getColor);
  void Bitmap(int xCentre,
              int yCentre,
              const UTILS::ImageBitmap& bitmap,
              const GetBitmapColorFunc& getColor,
              bool allowOverexposed);
  void Bitmap(int xCentre,
              int yCentre,
              const UTILS::ImageBitmap& bitmap,
              const std::vector<GetBitmapColorFunc>& getColors);
  void Bitmap(int xCentre,
              int yCentre,
              const UTILS::ImageBitmap& bitmap,
              const std::vector<GetBitmapColorFunc>& getColors,
              bool allowOverexposed);

  [[nodiscard]] virtual auto GetPixel(int32_t x, int32_t y) const -> Pixel = 0;
  void DrawPixels(int32_t x, int32_t y, const std::vector<Pixel>& colors);
  void DrawPixels(int32_t x, int32_t y, const std::vector<Pixel>& colors, bool allowOverexposed);
  virtual void DrawPixelsUnblended(int32_t x, int32_t y, const std::vector<Pixel>& colors) = 0;

protected:
  [[nodiscard]] auto GetIntBuffIntensity() const -> uint32_t;
  [[nodiscard]] auto GetParallel() const -> GOOM::UTILS::Parallel&;

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;
  DrawMethods m_drawMethods;
  bool m_allowOverexposed = true;
  float m_buffIntensity = 0.5F;
  uint32_t m_intBuffIntensity;
  mutable GOOM::UTILS::Parallel m_parallel;
};

inline auto IGoomDraw::GetScreenWidth() const -> uint32_t
{
  return m_screenWidth;
}

inline auto IGoomDraw::GetScreenHeight() const -> uint32_t
{
  return m_screenHeight;
}

inline auto IGoomDraw::GetAllowOverexposed() const -> bool
{
  return m_allowOverexposed;
}

inline void IGoomDraw::SetAllowOverexposed(const bool val)
{
  m_allowOverexposed = val;
  m_drawMethods.SetAllowOverexposed(val);
}

inline auto IGoomDraw::GetBuffIntensity() const -> float
{
  return m_buffIntensity;
}

inline void IGoomDraw::SetBuffIntensity(const float val)
{
  m_buffIntensity = val;
  m_intBuffIntensity =
      static_cast<uint32_t>(std::round(channel_limits<float>::max() * m_buffIntensity));
}

inline auto IGoomDraw::GetIntBuffIntensity() const -> uint32_t
{
  return m_intBuffIntensity;
}

inline auto IGoomDraw::GetParallel() const -> GOOM::UTILS::Parallel&
{
  return m_parallel;
}

inline void IGoomDraw::Circle(const int x0, const int y0, const int radius, const Pixel& color)
{
  Circle(x0, y0, radius, std::vector<Pixel>{color});
}

inline void IGoomDraw::Circle(const int x0,
                              const int y0,
                              const int radius,
                              const std::vector<Pixel>& colors)
{
  m_drawMethods.DrawCircle(x0, y0, radius, colors);
}

inline void IGoomDraw::Line(const int x1,
                            const int y1,
                            const int x2,
                            const int y2,
                            const Pixel& color,
                            const uint8_t thickness)
{
  Line(x1, y1, x2, y2, std::vector<Pixel>{color}, thickness);
}

inline void IGoomDraw::Line(const int x1,
                            const int y1,
                            const int x2,
                            const int y2,
                            const std::vector<Pixel>& colors,
                            const uint8_t thickness)
{
  m_drawMethods.DrawLine(x1, y1, x2, y2, colors, thickness);
}

inline void IGoomDraw::Bitmap(const int xCentre,
                              const int yCentre,
                              const UTILS::ImageBitmap& bitmap,
                              const GetBitmapColorFunc& getColor)
{
  Bitmap(xCentre, yCentre, bitmap, getColor, GetAllowOverexposed());
}

inline void IGoomDraw::Bitmap(const int xCentre,
                              const int yCentre,
                              const UTILS::ImageBitmap& bitmap,
                              const GetBitmapColorFunc& getColor,
                              const bool allowOverexposed)
{
  Bitmap(xCentre, yCentre, bitmap, std::vector<GetBitmapColorFunc>{getColor}, allowOverexposed);
}

inline void IGoomDraw::Bitmap(const int xCentre,
                              const int yCentre,
                              const UTILS::ImageBitmap& bitmap,
                              const std::vector<GetBitmapColorFunc>& getColors)
{
  Bitmap(xCentre, yCentre, bitmap, getColors, GetAllowOverexposed());
}

inline void IGoomDraw::DrawPixels(const int32_t x,
                                  const int32_t y,
                                  const std::vector<Pixel>& colors)
{
  DrawPixels(x, y, colors, GetAllowOverexposed());
}

inline void IGoomDraw::DrawPixels(const int32_t x,
                                  const int32_t y,
                                  const std::vector<Pixel>& colors,
                                  const bool allowOverexposed)
{
  m_drawMethods.DrawPixels(x, y, colors, allowOverexposed);
}

} // namespace DRAW
} // namespace GOOM