module;

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

export module Goom.Control.GoomTitleDisplayer;

import Goom.Color.ColorAdjustment;
import Goom.Color.ColorMaps;
import Goom.Color.RandomColorMaps;
import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShaperDrawers.TextDrawer;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

export namespace GOOM::CONTROL
{

class GoomTitleDisplayer
{
public:
  GoomTitleDisplayer(DRAW::IGoomDraw& draw,
                     const UTILS::MATH::GoomRand& goomRand,
                     const std::string& fontDirectory);

  void SetInitialPosition(int32_t xStart, int32_t yStart);

  [[nodiscard]] auto IsInitialPhase() const -> bool;
  [[nodiscard]] auto IsMiddlePhase() const -> bool;
  [[nodiscard]] auto IsFinalPhase() const -> bool;
  [[nodiscard]] auto IsFinalMoments() const -> bool;
  [[nodiscard]] auto IsFinished() const -> bool;

  void DrawMovingText(const std::string& title);
  void DrawStaticText(const std::string& title);

private:
  const UTILS::MATH::GoomRand* m_goomRand;
  static constexpr auto MAX_TEXT_DISPLAY_TIME       = 200;
  static constexpr auto TIME_TO_START_MIDDLE_PHASE  = 100;
  static constexpr auto TIME_TO_START_FINAL_PHASE   = 50;
  static constexpr auto TIME_TO_START_FINAL_MOMENTS = -5;
  static constexpr auto LINGER_AFTER_END_TIME       = 10;
  float m_xPos                                      = 0.0F;
  float m_yPos                                      = 0.0F;
  int32_t m_timeLeftOfTitleDisplay                  = MAX_TEXT_DISPLAY_TIME;
  std::unique_ptr<DRAW::SHAPE_DRAWERS::TextDrawer> m_textDrawer;
  int32_t m_screenWidth;
  int32_t m_screenHeight;
  std::string m_fontDirectory;
  size_t m_fontInfoIndex;
  [[nodiscard]] auto GetSelectedFontPath() const -> std::string;
  [[nodiscard]] auto GetSelectedFontSize() const -> int32_t;

  static constexpr auto DEFAULT_ALPHA = MAX_ALPHA;
  COLOR::RandomColorMaps m_randomColorMaps{DEFAULT_ALPHA, *m_goomRand};
  COLOR::ColorMapPtrWrapper m_textColorMap{m_randomColorMaps.GetRandomColorMap()};
  COLOR::ColorMapPtrWrapper m_textOutlineColorMap{m_randomColorMaps.GetRandomColorMap()};
  COLOR::ColorMapPtrWrapper m_charColorMap{
      m_randomColorMaps.GetRandomColorMap(COLOR::ColorMapGroup::DIVERGING_BLACK)};
  void DrawText(const std::string& text);
  [[nodiscard]] auto GetColorT() const -> float;
  [[nodiscard]] auto GetFontCharColorMixT() const -> float;
  [[nodiscard]] auto GetTextBrightness() const -> float;
  [[nodiscard]] auto GetInteriorColor(float fontColorT,
                                      float fontCharColorMixT,
                                      const Point2dInt& point,
                                      const Dimensions& charDimensions) const -> Pixel;
  [[nodiscard]] auto GetInitialPhaseInteriorColor(float fontColorT) const -> Pixel;

  struct FontTs
  {
    float fontColorT;
    float fontCharColorMixT;
  };
  [[nodiscard]] auto GetMiddlePhaseInteriorColor(const Point2dInt& point,
                                                 const FontTs& fontTs,
                                                 const Dimensions& charDimensions) const -> Pixel;
  [[nodiscard]] auto GetFinalPhaseInteriorColor(const Point2dInt& point,
                                                float fontCharColorMixT,
                                                const Dimensions& charDimensions) const -> Pixel;
  [[nodiscard]] auto GetOutlineColor(int32_t x,
                                     const FontTs& fontTs,
                                     int32_t charWidth) const -> Pixel;

  [[nodiscard]] auto GetCharSpacing() const -> float;
  void UpdateColorMaps();
  void UpdateFontSize();
  void UpdatePositionIncrements(const std::string& title);
  void UpdateTextPosition();
  void SetFinalPhaseColorMaps();
  [[nodiscard]] auto GetFinalPhaseFontSize(int32_t timeLeftOfTitleDisplay) const -> int32_t;
  static constexpr float INITIAL_PHASE_X_INCREMENT = 0.01F;
  static constexpr float INITIAL_PHASE_Y_INCREMENT = 0.0F;
  static constexpr float MIDDLE_PHASE_X_INCREMENT  = 1.0F;
  static constexpr float MIDDLE_PHASE_Y_INCREMENT  = 0.0F;
  struct FinalPhaseIncrements
  {
    float xIncrement = 0.0F;
    float yIncrement = 0.0F;
  };
  FinalPhaseIncrements m_finalPhaseIncrements{};
  [[nodiscard]] auto GetFinalPhaseIncrements(const std::string& title) -> FinalPhaseIncrements;
  [[nodiscard]] auto GetXIncrement() const -> float;
  [[nodiscard]] auto GetYIncrement() const -> float;
  [[nodiscard]] auto GetFinalPhaseCentrePenPos(const std::string& str) -> Point2dFlt;

  static constexpr float TEXT_GAMMA = 1.0F / 2.0F;
  COLOR::ColorAdjustment m_textColorAdjust{
      {.gamma = TEXT_GAMMA, .alterChromaFactor = COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR}
  };
};

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline void GoomTitleDisplayer::DrawStaticText(const std::string& title)
{
  DrawText(title);
}

inline void GoomTitleDisplayer::SetInitialPosition(const int32_t xStart, const int32_t yStart)
{
  m_xPos = static_cast<float>(xStart);
  m_yPos = static_cast<float>(yStart);
}

inline auto GoomTitleDisplayer::IsInitialPhase() const -> bool
{
  return m_timeLeftOfTitleDisplay > TIME_TO_START_MIDDLE_PHASE;
}

inline auto GoomTitleDisplayer::IsMiddlePhase() const -> bool
{
  return (TIME_TO_START_MIDDLE_PHASE >= m_timeLeftOfTitleDisplay) &&
         (m_timeLeftOfTitleDisplay > TIME_TO_START_FINAL_PHASE);
}

inline auto GoomTitleDisplayer::IsFinalPhase() const -> bool
{
  return m_timeLeftOfTitleDisplay <= TIME_TO_START_FINAL_PHASE;
}

inline auto GoomTitleDisplayer::IsFinalMoments() const -> bool
{
  return m_timeLeftOfTitleDisplay <= TIME_TO_START_FINAL_MOMENTS;
}

inline auto GoomTitleDisplayer::IsFinished() const -> bool
{
  return m_timeLeftOfTitleDisplay <= -LINGER_AFTER_END_TIME;
}

} // namespace GOOM::CONTROL
