module;

#include <blend2d.h> // NOLINT(misc-include-cleaner): Blend2d insists on this.
#include <blend2d/context.h>
#include <blend2d/format.h>
#include <blend2d/image.h>
#include <cstdint>
#include <span>

export module Goom.Utils.Graphics.Blend2dToGoom;

import Goom.Draw.GoomDrawBase;
import Goom.Draw.GoomDrawToBuffer;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;

export namespace GOOM::UTILS::GRAPHICS
{

class Blend2dToGoom
{
public:
  struct Blend2DBuffer
  {
    BLImage blend2dImage;
    BLContext blend2dContext;
  };

  explicit Blend2dToGoom(const Dimensions& dimensions,
                         const DRAW::IGoomDraw::PixelBlendFunc& func = nullptr) noexcept;

  [[nodiscard]] auto GetBlend2DBuffer() const noexcept -> const Blend2DBuffer&;
  [[nodiscard]] auto GetBlend2DBuffer() noexcept -> Blend2DBuffer&;

  [[nodiscard]] static auto GetBlend2dColor(const Pixel& pixel) -> uint32_t;

  auto SetIntBuffIntensity(float buffIntensity) noexcept -> void;
  auto SetNewAlpha(PixelChannelType newAlpha) noexcept -> void;
  auto SetPixelBlendFunc(const DRAW::IGoomDraw::PixelBlendFunc& func) noexcept -> void;

  auto UpdateGoomBuffer(PixelBuffer& goomBuffer) noexcept -> void;

private:
  DRAW::IGoomDraw::PixelBlendFunc m_pixelBlendFunc;
  // TODO(glk): Buff intensity is not really used?
  static constexpr float DEFAULT_BUFF_INTENSITY = 1.0F;
  uint32_t m_intBuffIntensity = DRAW::IGoomDraw::GetIntBuffIntensity(DEFAULT_BUFF_INTENSITY);
  PixelChannelType m_newAlpha = MAX_ALPHA;

  Blend2DBuffer m_blend2DBuffer;
  static auto GetNewBlend2DBuffer(const Dimensions& dimensions) noexcept -> Blend2DBuffer;
  [[nodiscard]] static auto GetImageData(const BLImage& blImage) noexcept -> BLImageData;
  [[nodiscard]] static auto GetPixelBuffer(const BLImage& blImage) noexcept
      -> std::span<const uint32_t>;

  struct ShiftsAndMasks
  {
    uint32_t rShift = 0;
    uint32_t gShift = 0;
    uint32_t bShift = 0;
    uint32_t aShift = 0;
    uint32_t rMask  = 0;
    uint32_t gMask  = 0;
    uint32_t bMask  = 0;
    uint32_t aMask  = 0;
  };
  ShiftsAndMasks m_shiftsAndMasks;
  [[nodiscard]] static auto GetShiftsAndMasks(const BLImage& blImage) noexcept -> ShiftsAndMasks;
  [[nodiscard]] static auto GetShiftsAndMasks(const BLFormatInfo& formatInfo) noexcept
      -> ShiftsAndMasks;

  static constexpr auto MAX_BLEND2D_CHANNEL = 256U;
  static constexpr auto CHANNEL_MULTIPLIER  = 50U;
  [[nodiscard]] auto GetGoomPixel(uint32_t blend2dColor) const noexcept -> Pixel;
  [[nodiscard]] auto GetBlendedPixel(uint32_t blend2dColor, const Pixel& pixel) const noexcept
      -> Pixel;
};

struct Blend2dContexts
{
  BLContext mainBlend2dContext;
  BLContext lowBlend2dContext;
  bool blend2dBuffersWereUsed;
};

class Blend2dDoubleGoomBuffers
{
public:
  Blend2dDoubleGoomBuffers(DRAW::GoomDrawToTwoBuffers& draw,
                           const Dimensions& dimensions,
                           const DRAW::IGoomDraw::PixelBlendFunc& func = nullptr) noexcept;

  [[nodiscard]] auto GetBlend2dContexts() noexcept -> Blend2dContexts&;

  auto Blend2dClearAll() -> void;
  auto UpdateGoomBuffers() noexcept -> void;

private:
  DRAW::GoomDrawToTwoBuffers* m_draw;
  Blend2dToGoom m_mainBuffer;
  Blend2dToGoom m_lowBuffer;
  Blend2dContexts m_blend2DContexts;
};

} // namespace GOOM::UTILS::GRAPHICS

namespace GOOM::UTILS::GRAPHICS
{

inline auto Blend2dDoubleGoomBuffers::GetBlend2dContexts() noexcept -> Blend2dContexts&
{
  return m_blend2DContexts;
}

inline auto Blend2dToGoom::GetBlend2DBuffer() const noexcept -> const Blend2DBuffer&
{
  return m_blend2DBuffer;
}

inline auto Blend2dToGoom::GetBlend2DBuffer() noexcept -> Blend2DBuffer&
{
  return m_blend2DBuffer;
}

inline auto Blend2dToGoom::SetPixelBlendFunc(const DRAW::IGoomDraw::PixelBlendFunc& func) noexcept
    -> void
{
  m_pixelBlendFunc = func;
}

inline auto Blend2dToGoom::SetIntBuffIntensity(const float buffIntensity) noexcept -> void
{
  m_intBuffIntensity = DRAW::IGoomDraw::GetIntBuffIntensity(buffIntensity);
}

inline auto Blend2dToGoom::SetNewAlpha(const PixelChannelType newAlpha) noexcept -> void
{
  m_newAlpha = newAlpha;
}

} // namespace GOOM::UTILS::GRAPHICS
