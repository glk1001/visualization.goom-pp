#include "text_draw.h"

#include "color/colorutils.h"
#include "goom/logging_control.h"
#include "goom_draw.h"
#include "goom_graphic.h"
//#undef NO_LOGGING
#include "goom/logging.h"
#include "goom/spimpl.h"

#include <codecvt>
#include <format>
#include <fstream>
#ifndef NO_FREETYPE_INSTALLED
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#endif
#include <locale>

namespace GOOM::DRAW
{

using COLOR::GetColorBlend;
using UTILS::Logging;

#ifdef NO_FREETYPE_INSTALLED
class TextDraw::TextDrawImpl
{
public:
  explicit TextDrawImpl(const IGoomDraw* draw) noexcept;
  ~TextDrawImpl() noexcept;
  TextDrawImpl(const TextDrawImpl&) noexcept = delete;
  TextDrawImpl(TextDrawImpl&&) noexcept = delete;
  auto operator=(const TextDrawImpl&) -> TextDrawImpl& = delete;
  auto operator=(TextDrawImpl&&) -> TextDrawImpl& = delete;

  void SetAlignment(TextAlignment a);
  [[nodiscard]] auto GetFontFile() const -> const std::string&;
  void SetFontFile(const std::string& filename);
  void SetFontSize(int32_t val);
  void SetOutlineWidth(float val);
  void SetCharSpacing(float val);
  void SetText(const std::string& str);

  void SetFontColorFunc(const FontColorFunc& f);
  void SetOutlineFontColorFunc(const FontColorFunc& f);

  void Prepare();
  [[nodiscard]] auto GetPreparedTextBoundingRect() const -> Rect;
  [[nodiscard]] auto GetBearingX() const -> int;
  [[nodiscard]] auto GetBearingY() const -> int;

  void Draw(int32_t xPen, int32_t yPen);
  void Draw(int32_t xPen, int32_t yPen, int32_t& xNext, int32_t& yNext);
private:
  std::string m_fontFilename{};
};

TextDraw::TextDrawImpl::TextDrawImpl(const IGoomDraw* const) noexcept
{
}

TextDraw::TextDrawImpl::~TextDrawImpl() noexcept
{
}

inline void TextDraw::TextDrawImpl::SetAlignment(const TextAlignment)
{
}

inline auto TextDraw::TextDrawImpl::GetFontFile() const -> const std::string&
{
  return m_fontFilename;
}

void TextDraw::TextDrawImpl::SetFontFile(const std::string& filename)
{
  m_fontFilename = filename;
}

void TextDraw::TextDrawImpl::SetFontSize(const int32_t)
{
}

void TextDraw::TextDrawImpl::SetOutlineWidth(const float)
{
}

void TextDraw::TextDrawImpl::SetCharSpacing(const float)
{
}

void TextDraw::TextDrawImpl::SetText(const std::string&)
{
}

void TextDraw::TextDrawImpl::SetFontColorFunc(const FontColorFunc&)
{
}

void TextDraw::TextDrawImpl::SetOutlineFontColorFunc(const FontColorFunc&)
{
}

void TextDraw::TextDrawImpl::Prepare()
{
}

inline void TextDraw::TextDrawImpl::Draw(const int32_t, const int32_t)
{
}

void TextDraw::TextDrawImpl::Draw(const int32_t,
                                  const int32_t,
                                  int32_t&,
                                  int32_t&)
{
}

inline auto TextDraw::TextDrawImpl::GetPreparedTextBoundingRect() const -> TextDraw::Rect
{
    return Rect{};
}

inline auto TextDraw::TextDrawImpl::GetBearingX() const -> int32_t
{
    return 1;
}

inline auto TextDraw::TextDrawImpl::GetBearingY() const -> int32_t
{
    return 1;
}
#endif

#ifndef NO_FREETYPE_INSTALLED
class TextDraw::TextDrawImpl
{
public:
  explicit TextDrawImpl(IGoomDraw& draw) noexcept;
  TextDrawImpl(const TextDrawImpl&) noexcept = delete;
  TextDrawImpl(TextDrawImpl&&) noexcept = delete;
  ~TextDrawImpl() noexcept;
  auto operator=(const TextDrawImpl&) -> TextDrawImpl& = delete;
  auto operator=(TextDrawImpl&&) -> TextDrawImpl& = delete;

  void SetAlignment(TextAlignment alignment);
  [[nodiscard]] auto GetFontFile() const -> const std::string&;
  void SetFontFile(const std::string& filename);
  auto GetFontSize() const -> int32_t;
  void SetFontSize(int32_t val);
  auto GetLineSpacing() const -> int32_t;
  void SetOutlineWidth(float val);
  void SetCharSpacing(float val);
  void SetText(const std::string& str);

  void SetFontColorFunc(const FontColorFunc& func);
  void SetOutlineFontColorFunc(const FontColorFunc& func);

  void Prepare();
  [[nodiscard]] auto GetPreparedTextBoundingRect() const -> Rect;
  [[nodiscard]] auto GetBearingX() const -> int;
  [[nodiscard]] auto GetBearingY() const -> int;

  void Draw(int32_t xPen, int32_t yPen);
  void Draw(int32_t xPen, int32_t yPen, int32_t& xNext, int32_t& yNext);

private:
  IGoomDraw& m_draw;
  FT_Library m_library{};
  static constexpr int32_t DEFAULT_FONT_SIZE = 100;
  int32_t m_fontSize = DEFAULT_FONT_SIZE;
  uint32_t m_horizontalResolution = 90;
  uint32_t m_verticalResolution = 90;
  float m_outlineWidth = 3.0F;
  float m_charSpacing = 0.0F;
  std::string m_fontFilename{};
  std::vector<unsigned char> m_fontBuffer{};
  std::string m_theText{};
  TextAlignment m_textAlignment{TextAlignment::LEFT};
  FT_Face m_face{};
  void SetFaceFontSize();

  FontColorFunc m_getFontColor{};
  FontColorFunc m_getOutlineFontColor{};

  static constexpr auto ToStdPixelCoord(int32_t freeTypeCoord) -> int;
  static constexpr auto ToFreeTypeCoord(int32_t stdPixelCoord) -> int;
  static constexpr auto ToFreeTypeCoord(float stdPixelCoord) -> int;
  struct Vec2;
  struct Span;
  using SpanArray = std::vector<Span>;

  struct RectImpl : Rect
  {
    RectImpl() noexcept = default;
    RectImpl(int32_t left, int32_t top, int32_t right, int32_t bottom) noexcept;
    RectImpl(const RectImpl&) noexcept = default;
    RectImpl(RectImpl&&) noexcept = default;
    ~RectImpl() noexcept = default;
    auto operator=(const RectImpl&) noexcept -> RectImpl = delete;
    auto operator=(RectImpl&&) noexcept -> RectImpl = delete;

    void Include(const Vec2& span);
  };

  struct Spans
  {
    SpanArray stdSpans{};
    SpanArray outlineSpans{};
    size_t textIndexOfChar{};
    RectImpl rect{};
    int32_t advance{};
    int32_t bearingX{};
    int32_t bearingY{};
  };

  std::vector<Spans> m_textSpans{};
  Rect m_textBoundingRect{};
  static auto GetBoundingRect(const SpanArray& stdSpans, const SpanArray& outlineSpans) -> RectImpl;
  [[nodiscard]] auto GetSpans(size_t textIndexOfChar) const -> Spans;
  [[nodiscard]] auto GetStdSpans() const -> SpanArray;
  [[nodiscard]] auto GetOutlineSpans() const -> SpanArray;
  void RenderSpans(FT_Outline* outline, SpanArray* spans) const;
  static void RasterCallback(int32_t y, int32_t count, const FT_Span* spans, void* user);

  [[nodiscard]] auto GetStartXPen(int32_t xPen) const -> int;
  [[nodiscard]] static auto GetStartYPen(int32_t yPen) -> int;
  void WriteGlyph(const Spans& spans, int32_t xPen, int32_t yPen) const;
  void WriteSpansToImage(const SpanArray& spanArray,
                         const RectImpl& rect,
                         int32_t xPen,
                         int32_t yPen,
                         size_t textIndexOfChar,
                         const FontColorFunc& getColor) const;
};
#endif

TextDraw::TextDraw(IGoomDraw& draw) noexcept
  : m_textDrawImpl{spimpl::make_unique_impl<TextDrawImpl>(draw)}
{
}

void TextDraw::SetAlignment(const TextAlignment alignment)
{
  m_textDrawImpl->SetAlignment(alignment);
}

auto TextDraw::GetFontFile() const -> const std::string&
{
  return m_textDrawImpl->GetFontFile();
}

void TextDraw::SetFontFile(const std::string& filename)
{
  m_textDrawImpl->SetFontFile(filename);
}

auto TextDraw::GetFontSize() const -> int32_t
{
  return m_textDrawImpl->GetFontSize();
}

void TextDraw::SetFontSize(const int32_t val)
{
  m_textDrawImpl->SetFontSize(val);
}

auto TextDraw::GetLineSpacing() const -> int32_t
{
  return m_textDrawImpl->GetLineSpacing();
}

void TextDraw::SetOutlineWidth(const float val)
{
  m_textDrawImpl->SetOutlineWidth(val);
}

void TextDraw::SetCharSpacing(const float val)
{
  m_textDrawImpl->SetCharSpacing(val);
}

void TextDraw::SetText(const std::string& str)
{
  m_textDrawImpl->SetText(str);
}

void TextDraw::SetFontColorFunc(const FontColorFunc& func)
{
  m_textDrawImpl->SetFontColorFunc(func);
}

void TextDraw::SetOutlineFontColorFunc(const FontColorFunc& func)
{
  m_textDrawImpl->SetOutlineFontColorFunc(func);
}

void TextDraw::Prepare()
{
  m_textDrawImpl->Prepare();
}

auto TextDraw::GetPreparedTextBoundingRect() const -> TextDraw::Rect
{
  return m_textDrawImpl->GetPreparedTextBoundingRect();
}

auto TextDraw::GetBearingX() const -> int32_t
{
  return m_textDrawImpl->GetBearingX();
}

auto TextDraw::GetBearingY() const -> int32_t
{
  return m_textDrawImpl->GetBearingY();
}

void TextDraw::Draw(const int32_t xPen, const int32_t yPen)
{
  m_textDrawImpl->Draw(xPen, yPen);
}

void TextDraw::Draw(const int32_t xPen, const int32_t yPen, int32_t& xNext, int32_t& yNext)
{
  m_textDrawImpl->Draw(xPen, yPen, xNext, yNext);
}

#ifndef NO_FREETYPE_INSTALLED
TextDraw::TextDrawImpl::TextDrawImpl(IGoomDraw& draw) noexcept : m_draw{draw}
{
  (void)FT_Init_FreeType(&m_library);
}

TextDraw::TextDrawImpl::~TextDrawImpl() noexcept
{
  (void)FT_Done_FreeType(m_library);
}

inline void TextDraw::TextDrawImpl::SetAlignment(const TextAlignment alignment)
{
  m_textAlignment = alignment;
}

inline auto TextDraw::TextDrawImpl::GetFontFile() const -> const std::string&
{
  return m_fontFilename;
}

void TextDraw::TextDrawImpl::SetFontFile(const std::string& filename)
{
  m_fontFilename = filename;
  LogInfo("Setting font file '{}'.", m_fontFilename);

  std::ifstream fontFile(m_fontFilename, std::ios::binary);
  if (!fontFile)
  {
    throw std::runtime_error(std20::format("Could not open font file \"{}\".", m_fontFilename));
  }

  (void)fontFile.seekg(0, std::ios::end);
  std::fstream::pos_type fontFileSize = fontFile.tellg();
  (void)fontFile.seekg(0);
  m_fontBuffer.resize(static_cast<size_t>(fontFileSize));
  (void)fontFile.read(reinterpret_cast<char*>(m_fontBuffer.data()), fontFileSize);

  // Create a face from a memory buffer.  Be sure not to delete the memory buffer
  // until we are done using that font as FreeType will reference it directly.
  (void)FT_New_Memory_Face(m_library, m_fontBuffer.data(),
                           static_cast<FT_Long>(m_fontBuffer.size()), 0, &m_face);

  SetFaceFontSize();
}

void TextDraw::TextDrawImpl::SetFaceFontSize()
{
  if (FT_Set_Char_Size(m_face, ToFreeTypeCoord(m_fontSize), ToFreeTypeCoord(m_fontSize),
                       m_horizontalResolution, m_verticalResolution) != 0)
  {
    throw std::logic_error(std20::format("Could not set face font size to {}.", m_fontSize));
  }
}

auto TextDraw::TextDrawImpl::GetFontSize() const -> int32_t
{
  return m_fontSize;
}

void TextDraw::TextDrawImpl::SetFontSize(const int32_t val)
{
  if (val <= 0)
  {
    throw std::logic_error(std20::format("Font size <= 0: {}.", val));
  }

  m_fontSize = val;
  LogInfo("Setting font size {}.", m_fontSize);
  if (m_face)
  {
    SetFaceFontSize();
  }
}

inline auto TextDraw::TextDrawImpl::GetLineSpacing() const -> int32_t
{
  return m_face->height / 64;
}

void TextDraw::TextDrawImpl::SetOutlineWidth(const float val)
{
  if (val <= 0.0F)
  {
    throw std::logic_error(std20::format("Outline width <= 0: {}.", val));
  }
  m_outlineWidth = val;
}

void TextDraw::TextDrawImpl::SetCharSpacing(const float val)
{
  if (val < 0.0F)
  {
    throw std::logic_error(std20::format("Char spacing < 0: {}.", val));
  }
  m_charSpacing = val;
}

void TextDraw::TextDrawImpl::SetText(const std::string& str)
{
  if (str.empty())
  {
    throw std::logic_error("Text string is empty.");
  }

  m_theText = str;
  LogInfo("Setting font text '{}'.", m_theText);
}

void TextDraw::TextDrawImpl::SetFontColorFunc(const FontColorFunc& func)
{
  m_getFontColor = func;
}

void TextDraw::TextDrawImpl::SetOutlineFontColorFunc(const FontColorFunc& func)
{
  m_getOutlineFontColor = func;
}

void TextDraw::TextDrawImpl::Prepare()
{
  if (!m_face)
  {
    throw std::logic_error("Font face has not been set.");
  }

  m_textSpans.resize(0);

  int32_t xMax = 0;
  int32_t yMin = std::numeric_limits<int32_t>::max();
  int32_t yMax = 0;

  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  const std::u32string utf32Text = conv.from_bytes(m_theText);

  for (size_t i = 0; i < utf32Text.size(); ++i)
  {
    // Load the glyph we are looking for.
    const FT_UInt gIndex = FT_Get_Char_Index(m_face, static_cast<FT_ULong>(utf32Text[i]));
    if (FT_Load_Glyph(m_face, gIndex, FT_LOAD_NO_BITMAP) != 0)
    {
      throw std::runtime_error(
          std20::format("Could not load font char '{}' and glyph index {}.", m_theText[i], gIndex));
    }

    // Need an outline for this to work.
    if (m_face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
    {
      throw std::logic_error(
          std20::format("Not a correct font format: {}.", m_face->glyph->format));
    }

    const Spans spans = GetSpans(i);
    m_textSpans.emplace_back(spans);

    xMax += spans.advance;
    yMin = std::min(yMin, spans.rect.ymin);
    yMax = std::max(yMax, spans.rect.ymax);
  }

  m_textBoundingRect.xmin = 0;
  m_textBoundingRect.xmax = xMax;
  m_textBoundingRect.ymin = yMin;
  m_textBoundingRect.ymax = yMax;
  LogInfo("Font bounding rectangle: {}, {}, {}, {}.", m_textBoundingRect.xmin,
          m_textBoundingRect.xmax, m_textBoundingRect.ymin, m_textBoundingRect.ymax);
}

auto TextDraw::TextDrawImpl::GetStartXPen(const int32_t xPen) const -> int
{
  switch (m_textAlignment)
  {
    case TextAlignment::LEFT:
      return xPen;
    case TextAlignment::CENTER:
      return xPen - ((GetPreparedTextBoundingRect().xmax - GetPreparedTextBoundingRect().xmin) / 2);
    case TextAlignment::RIGHT:
      return xPen - (GetPreparedTextBoundingRect().xmax - GetPreparedTextBoundingRect().xmin);
    default:
      throw std::logic_error("Unknown TextAlignment value.");
  }
}

inline auto TextDraw::TextDrawImpl::GetStartYPen(const int32_t yPen) -> int
{
  return yPen;
}

inline void TextDraw::TextDrawImpl::Draw(const int32_t xPen, const int32_t yPen)
{
  int32_t xNext;
  int32_t yNext;
  Draw(xPen, yPen, xNext, yNext);
}

void TextDraw::TextDrawImpl::Draw(const int32_t xPen,
                                  const int32_t yPen,
                                  int32_t& xNext,
                                  int32_t& yNext)
{
  if (m_textSpans.empty())
  {
    throw std::logic_error("textSpans is empty.");
  }

  xNext = GetStartXPen(xPen);
  yNext = GetStartYPen(yPen);

  for (auto& s : m_textSpans)
  {
    WriteGlyph(s, xNext, static_cast<int>(m_draw.GetScreenHeight()) - yNext);
    xNext += s.advance;
  }
}

// A horizontal pixel span generated by the FreeType renderer.
struct TextDraw::TextDrawImpl::Vec2
{
  Vec2(const int32_t a, const int32_t b) : x{a}, y{b} {}
  int32_t x;
  int32_t y;
};

TextDraw::TextDrawImpl::RectImpl::RectImpl(const int32_t left,
                                           const int32_t top,
                                           const int32_t right,
                                           const int32_t bottom) noexcept
{
  xmin = left;
  xmax = right;
  ymin = top;
  ymax = bottom;
}

void TextDraw::TextDrawImpl::RectImpl::Include(const Vec2& span)
{
  xmin = std::min(xmin, span.x);
  ymin = std::min(ymin, span.y);
  xmax = std::max(xmax, span.x);
  ymax = std::max(ymax, span.y);
}

struct TextDraw::TextDrawImpl::Span
{
  Span(const int32_t _x, const int32_t _y, const int32_t _width, const int32_t _coverage)
    : x{_x}, y{_y}, width{_width}, coverage{_coverage}
  {
  }

  int32_t x;
  int32_t y;
  int32_t width;
  int32_t coverage;
};

// Render the specified character as a colored glyph with a colored outline.
void TextDraw::TextDrawImpl::WriteGlyph(const Spans& spans,
                                        const int32_t xPen,
                                        const int32_t yPen) const
{
  // Loop over the outline spans and just draw them into the image.
  WriteSpansToImage(spans.outlineSpans, spans.rect, xPen, yPen, spans.textIndexOfChar,
                    m_getOutlineFontColor);

  // Then loop over the regular glyph spans and blend them into the image.
  WriteSpansToImage(spans.stdSpans, spans.rect, xPen, yPen, spans.textIndexOfChar, m_getFontColor);
}

void TextDraw::TextDrawImpl::WriteSpansToImage(const SpanArray& spanArray,
                                               const RectImpl& rect,
                                               const int32_t xPen,
                                               const int32_t yPen,
                                               const size_t textIndexOfChar,
                                               const FontColorFunc& getColor) const
{
  for (const auto& s : spanArray)
  {
    const int32_t yPos = static_cast<int>(m_draw.GetScreenHeight()) - (yPen + s.y);
    if ((yPos < 0) || (yPos >= static_cast<int>(m_draw.GetScreenHeight())))
    {
      continue;
    }

    const int32_t xPos0 = xPen + (s.x - rect.xmin);
    const int32_t xf0 = s.x - rect.xmin;
    const auto coverage = static_cast<uint8_t>(s.coverage);
    for (int32_t w = 0; w < s.width; ++w)
    {
      const int32_t xPos = xPos0 + w;
      if ((xPos < 0) || (xPos >= static_cast<int>(m_draw.GetScreenWidth())))
      {
        continue;
      }

      const Pixel color = getColor(textIndexOfChar, xf0 + w, rect.Height() - (s.y - rect.ymin),
                                   rect.Width(), rect.Height());
      const Pixel srceColor{
          {/*.r = */ color.R(), /*.g = */ color.G(), /*.b = */ color.B(), /*.a = */ coverage}};
      const Pixel destColor = m_draw.GetPixel(xPos, yPos);

      m_draw.DrawPixelsUnblended(xPos, yPos, {GetColorBlend(srceColor, destColor)});
    }
  }
}

constexpr auto TextDraw::TextDrawImpl::ToStdPixelCoord(const int32_t freeTypeCoord) -> int32_t
{
  return freeTypeCoord >> 6;
}

constexpr auto TextDraw::TextDrawImpl::ToFreeTypeCoord(const int32_t stdPixelCoord) -> int32_t
{
  return stdPixelCoord << 6;
}

constexpr auto TextDraw::TextDrawImpl::ToFreeTypeCoord(const float stdPixelCoord) -> int32_t
{
  return static_cast<int>(std::lround(stdPixelCoord * 64.0F));
}

// Each time the renderer calls us back we just push another span entry on our list.
void TextDraw::TextDrawImpl::RasterCallback(const int32_t y,
                                            const int32_t count,
                                            const FT_Span* const spans,
                                            void* const user)
{
  auto* const userSpans = static_cast<SpanArray*>(user);
  for (int32_t i = 0; i < count; ++i)
  {
    userSpans->push_back(Span{spans[i].x, y, spans[i].len, spans[i].coverage});
  }
}

// Set up the raster parameters and render the outline.
void TextDraw::TextDrawImpl::RenderSpans(FT_Outline* const outline, SpanArray* const spans) const
{
  FT_Raster_Params params;
  (void)memset(&params, 0, sizeof(params));
  params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
  params.gray_spans = RasterCallback;
  params.user = spans;

  (void)FT_Outline_Render(m_library, outline, &params);
}

auto TextDraw::TextDrawImpl::GetSpans(const size_t textIndexOfChar) const -> Spans
{
  const SpanArray stdSpans = GetStdSpans();
  const int32_t advance = ToStdPixelCoord(static_cast<int32_t>(m_face->glyph->advance.x)) +
                          static_cast<int>(m_charSpacing * static_cast<float>(m_fontSize));
  const FT_Glyph_Metrics metrics = m_face->glyph->metrics;
  if (stdSpans.empty())
  {
    return Spans{
        /*.stdSpans = */ stdSpans,
        /*.outlineSpans = */ SpanArray{},
        /*.textIndexOfChar = */ textIndexOfChar,
        /*.rect = */ RectImpl{},
        /*.advance = */ advance,
        /*.bearingX = */ ToStdPixelCoord(static_cast<int32_t>(metrics.horiBearingX)),
        /*.bearingY = */ ToStdPixelCoord(static_cast<int32_t>(metrics.horiBearingY)),
    };
  }

  const SpanArray outlineSpans = GetOutlineSpans();
  return Spans{
      /*.stdSpans = */ stdSpans,
      /*.outlineSpans = */ outlineSpans,
      /*.textIndexOfChar = */ textIndexOfChar,
      /*.rect = */ GetBoundingRect(stdSpans, outlineSpans),
      /*.advance = */ advance,
      /*.bearingX = */ ToStdPixelCoord(static_cast<int32_t>(metrics.horiBearingX)),
      /*.bearingY = */ ToStdPixelCoord(static_cast<int32_t>(metrics.horiBearingY)),
  };
}

auto TextDraw::TextDrawImpl::GetStdSpans() const -> SpanArray
{
  SpanArray spans{};

  RenderSpans(&m_face->glyph->outline, &spans);

  return spans;
}

auto TextDraw::TextDrawImpl::GetOutlineSpans() const -> SpanArray
{
  // Set up a stroker.
  FT_Stroker stroker{};
  (void)FT_Stroker_New(m_library, &stroker);
  FT_Stroker_Set(stroker, ToFreeTypeCoord(m_outlineWidth), FT_STROKER_LINECAP_ROUND,
                 FT_STROKER_LINEJOIN_ROUND, 0);

  FT_Glyph glyph{};
  if (FT_Get_Glyph(m_face->glyph, &glyph) != 0)
  {
    throw std::runtime_error("Could not get glyph for outline spans.");
  }

  // Next we need the spans for the outline.
  (void)FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1);

  // Again, this needs to be an outline to work.
  if (glyph->format != FT_GLYPH_FORMAT_OUTLINE)
  {
    throw std::runtime_error("Glyph does not have outline format.");
  }

  // Render the outline spans to the span list
  FT_Outline* const outline = &reinterpret_cast<FT_OutlineGlyph>(glyph)->outline;
  SpanArray outlineSpans{};
  RenderSpans(outline, &outlineSpans);
  if (outlineSpans.empty())
  {
    throw std::logic_error("Rendered outline spans are empty.");
  }

  // Clean up afterwards.
  FT_Stroker_Done(stroker);
  FT_Done_Glyph(glyph);

  return outlineSpans;
}

inline auto TextDraw::TextDrawImpl::GetPreparedTextBoundingRect() const -> TextDraw::Rect
{
  return m_textBoundingRect;
}

inline auto TextDraw::TextDrawImpl::GetBearingX() const -> int32_t
{
  return m_textSpans.front().bearingX;
}

inline auto TextDraw::TextDrawImpl::GetBearingY() const -> int32_t
{
  return m_textSpans.front().bearingY;
}

auto TextDraw::TextDrawImpl::GetBoundingRect(const SpanArray& stdSpans,
                                             const SpanArray& outlineSpans) -> RectImpl
{
  RectImpl rect{stdSpans.front().x, stdSpans.front().y, stdSpans.front().x, stdSpans.front().y};

  for (const auto& s : stdSpans)
  {
    rect.Include(Vec2{s.x, s.y});
    rect.Include(Vec2{s.x + (s.width - 1), s.y});
  }
  for (const auto& s : outlineSpans)
  {
    rect.Include(Vec2{s.x, s.y});
    rect.Include(Vec2{s.x + (s.width - 1), s.y});
  }

  return rect;
}
#endif

} // namespace GOOM::DRAW
