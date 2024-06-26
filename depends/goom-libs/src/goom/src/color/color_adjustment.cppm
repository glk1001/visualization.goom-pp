module;

export module Goom.Color.ColorAdjustment;

import Goom.Utils.Math.Misc;
import Goom.Lib.GoomGraphic;

export namespace GOOM::COLOR
{

class ColorAdjustment
{
public:
  struct AdjustmentProperties
  {
    float gamma             = 1.0F;
    float alterChromaFactor = 1.0F;
  };

  explicit ColorAdjustment(const AdjustmentProperties& adjustmentProperties);

  [[nodiscard]] auto GetIgnoreThreshold() const -> float;
  auto SetIgnoreThreshold(float val) -> void;

  [[nodiscard]] auto GetGamma() const -> float;
  auto SetGamma(float val) -> void;

  [[nodiscard]] auto GetChromaFactor() const -> float;
  auto SetChromaFactor(float val) -> void;

  [[nodiscard]] auto GetAdjustment(float brightness, const Pixel& color) const -> Pixel;

  static constexpr float INCREASED_CHROMA_FACTOR = 2.0F;
  static constexpr float DECREASED_CHROMA_FACTOR = 0.5F;
  [[nodiscard]] static auto GetAlteredChromaColor(float lchYFactor, const Pixel& color) -> Pixel;
  [[nodiscard]] static auto GetIncreasedChromaColor(const Pixel& color) -> Pixel;
  [[nodiscard]] static auto GetDecreasedChromaColor(const Pixel& color) -> Pixel;

private:
  float m_gamma;
  float m_chromaFactor;
  bool m_doAlterGamma  = not UTILS::MATH::FloatsEqual(1.0F, m_gamma);
  bool m_doAlterChroma = not UTILS::MATH::FloatsEqual(1.0F, m_chromaFactor);
  static constexpr float DEFAULT_GAMMA_BRIGHTNESS_THRESHOLD = 0.01F;
  float m_ignoreThreshold                                   = DEFAULT_GAMMA_BRIGHTNESS_THRESHOLD;
};

} // namespace GOOM::COLOR

namespace GOOM::COLOR
{

inline ColorAdjustment::ColorAdjustment(const AdjustmentProperties& adjustmentProperties)
  : m_gamma{adjustmentProperties.gamma}, m_chromaFactor{adjustmentProperties.alterChromaFactor}
{
}

inline auto ColorAdjustment::GetIgnoreThreshold() const -> float
{
  return m_ignoreThreshold;
}

inline auto ColorAdjustment::SetIgnoreThreshold(const float val) -> void
{
  m_ignoreThreshold = val;
}

inline auto ColorAdjustment::GetGamma() const -> float
{
  return m_gamma;
}

inline auto ColorAdjustment::SetGamma(const float val) -> void
{
  m_gamma        = val;
  m_doAlterGamma = not UTILS::MATH::FloatsEqual(1.0F, m_gamma);
}

inline auto ColorAdjustment::GetChromaFactor() const -> float
{
  return m_chromaFactor;
}

inline auto ColorAdjustment::SetChromaFactor(const float val) -> void
{
  m_chromaFactor  = val;
  m_doAlterChroma = not UTILS::MATH::FloatsEqual(1.0F, m_chromaFactor);
}

inline auto ColorAdjustment::GetIncreasedChromaColor(const Pixel& color) -> Pixel
{
  return GetAlteredChromaColor(INCREASED_CHROMA_FACTOR, color);
}

inline auto ColorAdjustment::GetDecreasedChromaColor(const Pixel& color) -> Pixel
{
  return GetAlteredChromaColor(DECREASED_CHROMA_FACTOR, color);
}

} // namespace GOOM::COLOR
