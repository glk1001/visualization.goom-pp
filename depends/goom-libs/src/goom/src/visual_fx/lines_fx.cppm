module;

#include <cstdint>
#include <string>
#include <vector>

export module Goom.VisualFx.LinesFx;

import Goom.Color.RandomColorMaps;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.VisualFxBase;
import Goom.Lib.SoundInfo;
import Goom.Lib.SPimpl;

export namespace GOOM::VISUAL_FX
{

class LinesFx : public IVisualFx
{
public:
  static constexpr uint32_t NUM_LINES = 2;

  // construit un effet de line (une ligne horitontale pour commencer)
  LinesFx() noexcept = delete;
  LinesFx(FxHelper& fxHelper, const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void override;
  auto SetSoundData(const AudioSamples& soundData) noexcept -> void override;
  auto ResetLineModes() noexcept -> void;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void override;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string> override;

  auto ApplyToImageBuffers() noexcept -> void override;

private:
  class LinesImpl;
  spimpl::unique_impl_ptr<LinesImpl> m_pimpl;
};

} // namespace GOOM::VISUAL_FX
