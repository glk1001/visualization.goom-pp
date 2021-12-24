#pragma once

#include "color/colormaps.h"
#include "color/random_colormaps.h"
#include "goom_graphic.h"
#include "tentacle2d.h"
#include "utils/goom_rand_base.h"

#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

struct V3dFlt
{
  float x = 0.0;
  float y = 0.0;
  float z = 0.0;
  bool ignore = false;
};

class Tentacle3D
{
public:
  Tentacle3D() noexcept = delete;
  Tentacle3D(std::unique_ptr<Tentacle2D> tentacle,
             const Pixel& headColor,
             const Pixel& headLowColor,
             const V3dFlt& head,
             size_t numHeadNodes,
             const UTILS::IGoomRand& goomRand) noexcept;
  Tentacle3D(const Tentacle3D&) noexcept = delete;
  Tentacle3D(Tentacle3D&&) noexcept = default;
  ~Tentacle3D() noexcept = default;
  auto operator=(const Tentacle3D&) -> Tentacle3D& = delete;
  auto operator=(Tentacle3D&&) -> Tentacle3D& = delete;

  auto Get2DTentacle() -> Tentacle2D& { return *m_tentacle; }
  [[nodiscard]] auto Get2DTentacle() const -> const Tentacle2D& { return *m_tentacle; }

  void SetWeightedColorMaps(const std::shared_ptr<COLOR::RandomColorMaps>& weightedMaps);
  void ColorMapsChanged();

  [[nodiscard]] auto GetMixedColors(size_t nodeNum,
                                    const Pixel& color,
                                    const Pixel& lowColor,
                                    float brightness) const -> std::pair<Pixel, Pixel>;

  void SetReverseColorMix(const bool val) { m_reverseColorMix = val; }

  [[nodiscard]] auto GetHead() const -> const V3dFlt& { return m_head; }
  void SetHead(const V3dFlt& val) { m_head = val; }

  [[nodiscard]] auto GetNumHeadNodes() const -> size_t { return m_numHeadNodes; }
  void SetNumHeadNodes(const size_t val) { m_numHeadNodes = val; }

  [[nodiscard]] auto GetVertices() const -> std::vector<V3dFlt>;

private:
  const UTILS::IGoomRand& m_goomRand;
  std::unique_ptr<Tentacle2D> m_tentacle;

  std::shared_ptr<COLOR::RandomColorMaps> m_colorMaps{};
  std::shared_ptr<const COLOR::IColorMap> m_currentColorMap{};
  std::shared_ptr<const COLOR::IColorMap> m_currentLowColorMap{};
  bool m_reverseColorMix = false;
  bool m_useIncreasedChroma = true;

  Pixel m_headColor{};
  Pixel m_headLowColor{};
  V3dFlt m_head{};
  size_t m_numHeadNodes{};

  [[nodiscard]] auto GetMixedColors(size_t nodeNum, const Pixel& color, const Pixel& lowColor) const
      -> std::pair<Pixel, Pixel>;
  [[nodiscard]] auto GetMixedHeadColors(size_t nodeNum,
                                        const Pixel& color,
                                        const Pixel& lowColor) const -> std::pair<Pixel, Pixel>;
  [[nodiscard]] auto GetFinalMixedColor(const Pixel& color,
                                        const Pixel& segmentColor,
                                        float t) const -> Pixel;
};

inline void Tentacle3D::SetWeightedColorMaps(
    const std::shared_ptr<COLOR::RandomColorMaps>& weightedMaps)
{
  m_colorMaps = weightedMaps;
  ColorMapsChanged();
}

} // namespace GOOM::VISUAL_FX::TENTACLES