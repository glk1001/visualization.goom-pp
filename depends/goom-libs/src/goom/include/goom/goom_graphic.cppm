module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

export module Goom.Lib.GoomGraphic;

import Goom.Lib.GoomTypes;

#ifdef WORDS_BIGENDIAN
#define COLOR_ARGB
#else
#define COLOR_BGRA
#endif

export namespace GOOM
{

template<class T>
struct channel_limits // NOLINT(readability-identifier-naming)
{
  [[nodiscard]] static constexpr auto min() noexcept -> T { return T(); }
  [[nodiscard]] static constexpr auto max() noexcept -> T { return T(); }
};
template<>
struct channel_limits<uint8_t>
{
  [[nodiscard]] static constexpr auto min() noexcept -> uint8_t { return 0; }
  [[nodiscard]] static constexpr auto max() noexcept -> uint8_t
  {
    return std::numeric_limits<uint8_t>::max();
  }
};
template<>
struct channel_limits<uint16_t>
{
  [[nodiscard]] static constexpr auto min() noexcept -> uint16_t
  {
    return channel_limits<uint8_t>::min();
  }
  [[nodiscard]] static constexpr auto max() noexcept -> uint16_t
  {
    return channel_limits<uint8_t>::max();
  }
};
template<>
struct channel_limits<uint32_t>
{
  [[nodiscard]] static constexpr auto min() noexcept -> uint32_t
  {
    return channel_limits<uint8_t>::min();
  }
  [[nodiscard]] static constexpr auto max() noexcept -> uint32_t
  {
    return channel_limits<uint8_t>::max();
  }
};
template<>
struct channel_limits<int32_t>
{
  [[nodiscard]] static constexpr auto min() noexcept -> int32_t
  {
    return channel_limits<uint8_t>::min();
  }
  [[nodiscard]] static constexpr auto max() noexcept -> int32_t
  {
    return channel_limits<uint8_t>::max();
  }
};
template<>
struct channel_limits<float>
{
  [[nodiscard]] static constexpr auto min() noexcept -> float
  {
    return channel_limits<uint8_t>::min();
  }
  [[nodiscard]] static constexpr auto max() noexcept -> float
  {
    return channel_limits<uint8_t>::max();
  }
};

using PixelChannelType = uint16_t;
using PixelIntType     = uint64_t;

inline constexpr auto MAX_CHANNEL_VALUE_HDR = 30U * 1024U;
static_assert(MAX_CHANNEL_VALUE_HDR <= std::numeric_limits<PixelChannelType>::max());

inline constexpr auto MAX_COLOR_VAL = channel_limits<PixelChannelType>::max();
inline constexpr auto MAX_ALPHA     = std::numeric_limits<PixelChannelType>::max();
[[nodiscard]] constexpr auto ToPixelAlpha(const uint8_t byteAlpha) -> PixelChannelType
{
  constexpr auto MAX_ALPHA_PLUS_1 = static_cast<uint32_t>(MAX_ALPHA) + 1;
  constexpr auto MAX_BYTE_PLUS_1  = static_cast<uint32_t>(std::numeric_limits<uint8_t>::max()) + 1;
  const auto byteAlphaPlus1       = static_cast<uint32_t>(byteAlpha) + 1;
  return static_cast<PixelChannelType>(((MAX_ALPHA_PLUS_1 / MAX_BYTE_PLUS_1) * byteAlphaPlus1) - 1);
}

// TODO(glk) - maybe should be template: Pixel<uint8_t>, Pixel<uint16_t>
class Pixel
{
public:
  struct RGB
  {
    PixelChannelType red   = 0U;
    PixelChannelType green = 0U;
    PixelChannelType blue  = 0U;
    PixelChannelType alpha = MAX_ALPHA;
  };

  constexpr Pixel() noexcept = default;
  constexpr explicit Pixel(const RGB& color) noexcept;
  constexpr Pixel(PixelChannelType red,
                  PixelChannelType green,
                  PixelChannelType blue,
                  PixelChannelType alpha) noexcept;

  [[nodiscard]] constexpr auto R() const noexcept -> PixelChannelType;
  [[nodiscard]] constexpr auto G() const noexcept -> PixelChannelType;
  [[nodiscard]] constexpr auto B() const noexcept -> PixelChannelType;
  [[nodiscard]] constexpr auto A() const noexcept -> PixelChannelType;

  constexpr auto SetR(PixelChannelType val) noexcept -> void;
  constexpr auto SetG(PixelChannelType val) noexcept -> void;
  constexpr auto SetB(PixelChannelType val) noexcept -> void;
  constexpr auto SetA(PixelChannelType val) noexcept -> void;

  [[nodiscard]] constexpr auto RFlt() const noexcept -> float;
  [[nodiscard]] constexpr auto GFlt() const noexcept -> float;
  [[nodiscard]] constexpr auto BFlt() const noexcept -> float;
  [[nodiscard]] constexpr auto AFlt() const noexcept -> float;

  [[nodiscard]] constexpr auto IsBlack() const noexcept -> bool;

  friend constexpr auto operator==(const Pixel& pixel1, const Pixel& pixel2) noexcept -> bool;

private:
#ifdef COLOR_BGRA
  struct Channels
  {
    PixelChannelType r = 0;
    PixelChannelType g = 0;
    PixelChannelType b = 0;
    PixelChannelType a = 0;
  };
#else
  struct Channels
  {
    PixelChannelType a = 0;
    PixelChannelType r = 0;
    PixelChannelType g = 0;
    PixelChannelType b = 0;
  };
#endif /* COLOR_BGRA */

  Channels m_color{};
};

static_assert(sizeof(Pixel) == sizeof(PixelIntType));
[[nodiscard]] constexpr auto Rgba(const Pixel& color) noexcept -> PixelIntType;
[[nodiscard]] constexpr auto operator==(const Pixel& pixel1, const Pixel& pixel2) noexcept -> bool;

struct FXBuffSettings
{
  static constexpr float INITIAL_BUFF_INTENSITY = 0.5F;
  float buffIntensity                           = INITIAL_BUFF_INTENSITY;
};

class PixelBuffer
{
  using Buffer = std::span<Pixel>;

public:
  PixelBuffer() = default;
  PixelBuffer(const Buffer& buffer, const Dimensions& dimensions) noexcept;
  PixelBuffer(const PixelBuffer&)                    = delete;
  PixelBuffer(PixelBuffer&&)                         = delete;
  ~PixelBuffer() noexcept                            = default;
  auto operator=(const PixelBuffer&) -> PixelBuffer& = delete;
  auto operator=(PixelBuffer&&) -> PixelBuffer&      = delete;

  [[nodiscard]] auto GetPixelBuffer() const noexcept -> const Buffer&;
  [[nodiscard]] auto GetPixelBuffer() noexcept -> Buffer&;
  auto SetPixelBuffer(const Buffer& buffer, const Dimensions& dimensions) noexcept -> void;
  auto Fill(const Pixel& pixel) noexcept -> void;

  [[nodiscard]] auto GetWidth() const noexcept -> uint32_t;
  [[nodiscard]] auto GetHeight() const noexcept -> uint32_t;

  [[nodiscard]] auto operator()(size_t x, size_t y) const noexcept -> const Pixel&;
  [[nodiscard]] auto operator()(int32_t x, int32_t y) const noexcept -> const Pixel&;
  [[nodiscard]] auto operator()(size_t x, size_t y) noexcept -> Pixel&;
  [[nodiscard]] auto operator()(int32_t x, int32_t y) noexcept -> Pixel&;

  // Fastest access.
  [[nodiscard]] auto GetBuffPos(size_t x, size_t y) const noexcept -> size_t;
  [[nodiscard]] auto GetBuffPos(int32_t x, int32_t y) const noexcept -> size_t;
  [[nodiscard]] auto GetPixel(size_t buffPos) const noexcept -> const Pixel&;
  [[nodiscard]] auto GetPixel(size_t buffPos) noexcept -> Pixel&;

protected:
  explicit PixelBuffer(const Dimensions& dimensions) noexcept;
  auto SetPixelBuffer(const Buffer& buffer) noexcept -> void;

private:
  uint32_t m_width  = 0U;
  uint32_t m_height = 0U;
  Buffer m_buff;
};

class PixelBufferVector : public PixelBuffer
{
public:
  explicit PixelBufferVector(const Dimensions& dimensions) noexcept;

  [[nodiscard]] auto GetBuffer() noexcept -> std::span<Pixel>;

private:
  std::vector<Pixel> m_owningBuff;
};

} // namespace GOOM

namespace GOOM
{

inline PixelBufferVector::PixelBufferVector(const Dimensions& dimensions) noexcept
  : PixelBuffer{dimensions}, m_owningBuff(dimensions.GetSize())
{
  SetPixelBuffer(GetBuffer());
}

inline auto PixelBufferVector::GetBuffer() noexcept -> std::span<Pixel>
{
  return std::span<Pixel>{m_owningBuff};
}

constexpr Pixel::Pixel(const RGB& color) noexcept
  : m_color{.r = color.red, .g = color.green, .b = color.blue, .a = color.alpha}
{
}

constexpr Pixel::Pixel(const PixelChannelType red,
                       const PixelChannelType green,
                       const PixelChannelType blue,
                       const PixelChannelType alpha) noexcept
  : m_color{.r = red, .g = green, .b = blue, .a = alpha}
{
}

export inline constexpr auto ZERO_PIXEL  = Pixel{0U, 0U, 0U, 0U};
export inline constexpr auto BLACK_PIXEL = Pixel{0U, 0U, 0U, MAX_ALPHA};
export inline constexpr auto WHITE_PIXEL =
    Pixel{MAX_COLOR_VAL, MAX_COLOR_VAL, MAX_COLOR_VAL, MAX_ALPHA};

constexpr auto operator==(const Pixel& pixel1, const Pixel& pixel2) noexcept -> bool
{
  return (pixel1.m_color.r == pixel2.m_color.r) and (pixel1.m_color.g == pixel2.m_color.g) and
         (pixel1.m_color.b == pixel2.m_color.b) and (pixel1.m_color.a == pixel2.m_color.a);
}

constexpr auto Pixel::R() const noexcept -> PixelChannelType
{
  return m_color.r;
}

constexpr auto Pixel::SetR(const PixelChannelType val) noexcept -> void
{
  m_color.r = val;
}

constexpr auto Pixel::G() const noexcept -> PixelChannelType
{
  return m_color.g;
}

constexpr auto Pixel::SetG(const PixelChannelType val) noexcept -> void
{
  m_color.g = val;
}

constexpr auto Pixel::B() const noexcept -> PixelChannelType
{
  return m_color.b;
}

constexpr auto Pixel::SetB(const PixelChannelType val) noexcept -> void
{
  m_color.b = val;
}

constexpr auto Pixel::A() const noexcept -> PixelChannelType
{
  return m_color.a;
}

constexpr auto Pixel::SetA(const PixelChannelType val) noexcept -> void
{
  m_color.a = val;
}

constexpr auto Pixel::RFlt() const noexcept -> float
{
  return static_cast<float>(R()) / channel_limits<float>::max();
}

constexpr auto Pixel::GFlt() const noexcept -> float
{
  return static_cast<float>(G()) / channel_limits<float>::max();
}

constexpr auto Pixel::BFlt() const noexcept -> float
{
  return static_cast<float>(B()) / channel_limits<float>::max();
}

constexpr auto Pixel::AFlt() const noexcept -> float
{
  return static_cast<float>(A()) / MAX_ALPHA;
}

constexpr auto Rgba(const Pixel& color) noexcept -> PixelIntType
{
  constexpr auto CHANNEL_DEPTH = sizeof(PixelChannelType) * 8U;
  constexpr auto SHIFT_AMOUNT1 = 3U * CHANNEL_DEPTH;
  constexpr auto SHIFT_AMOUNT2 = 2U * CHANNEL_DEPTH;
  constexpr auto SHIFT_AMOUNT3 = 1U * CHANNEL_DEPTH;
  constexpr auto SHIFT_AMOUNT4 = 0U * CHANNEL_DEPTH;
#ifdef COLOR_BGRA
  return (static_cast<PixelIntType>(color.R()) << SHIFT_AMOUNT1) +
         (static_cast<PixelIntType>(color.G()) << SHIFT_AMOUNT2) +
         (static_cast<PixelIntType>(color.B()) << SHIFT_AMOUNT3) +
         (static_cast<PixelIntType>(color.A()) << SHIFT_AMOUNT4);
#else
  return (static_cast<PixelIntType>(color.A()) << SHIFT_AMOUNT1) +
         (static_cast<PixelIntType>(color.B()) << SHIFT_AMOUNT2) +
         (static_cast<PixelIntType>(color.G()) << SHIFT_AMOUNT3) +
         (static_cast<PixelIntType>(color.R()) << SHIFT_AMOUNT4);
#endif
}

constexpr auto Pixel::IsBlack() const noexcept -> bool
{
  return (0 == m_color.r) and (0 == m_color.g) and (0 == m_color.b);
}

inline auto PixelBuffer::GetWidth() const noexcept -> uint32_t
{
  return m_width;
}

inline auto PixelBuffer::GetHeight() const noexcept -> uint32_t
{
  return m_height;
}

inline auto PixelBuffer::GetPixelBuffer() const noexcept -> const Buffer&
{
  return m_buff;
}

inline auto PixelBuffer::GetPixelBuffer() noexcept -> Buffer&
{
  return m_buff;
}

inline auto PixelBuffer::Fill(const Pixel& pixel) noexcept -> void
{
  std::ranges::fill(m_buff, pixel);
}

inline auto PixelBuffer::operator()(const size_t x, const size_t y) const noexcept -> const Pixel&
{
  return GetPixel(GetBuffPos(x, y));
}

inline auto PixelBuffer::operator()(const int32_t x, const int32_t y) const noexcept -> const Pixel&
{
  return GetPixel(GetBuffPos(x, y));
}

inline auto PixelBuffer::operator()(const size_t x, const size_t y) noexcept -> Pixel&
{
  return GetPixel(GetBuffPos(x, y));
}

inline auto PixelBuffer::operator()(const int32_t x, const int32_t y) noexcept -> Pixel&
{
  return GetPixel(GetBuffPos(x, y));
}

inline auto PixelBuffer::GetBuffPos(const size_t x, const size_t y) const noexcept -> size_t
{
  return (y * m_width) + x;
}

inline auto PixelBuffer::GetBuffPos(const int32_t x, const int32_t y) const noexcept -> size_t
{
  return (static_cast<size_t>(y) * m_width) + static_cast<size_t>(x);
}

inline auto PixelBuffer::GetPixel(const size_t buffPos) const noexcept -> const Pixel&
{
  return m_buff[buffPos];
}

inline auto PixelBuffer::GetPixel(const size_t buffPos) noexcept -> Pixel&
{
  return m_buff[buffPos];
}

} // namespace GOOM
