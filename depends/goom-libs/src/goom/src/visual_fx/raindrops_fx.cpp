module;

#include <memory>
#include <string>
#include <vector>

module Goom.VisualFx.RaindropsFx;

import Goom.VisualFx.FxHelper;
import Goom.VisualFx.FxUtils;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import Goom.Lib.SPimpl;
import :Raindrops;

namespace GOOM::VISUAL_FX
{

using FX_UTILS::RandomPixelBlender;
using RAINDROPS::Raindrops;

class RaindropsFx::RaindropsFxImpl
{
public:
  explicit RaindropsFxImpl(FxHelper& fxHelper) noexcept;

  auto Start() noexcept -> void;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;
  [[nodiscard]] static auto GetCurrentColorMapsNames() noexcept -> std::vector<std::string>;

  auto ApplyToImageBuffers() noexcept -> void;

private:
  FxHelper* m_fxHelper;
  Point2dInt m_zoomMidpoint = m_fxHelper->GetDimensions().GetCentrePoint();

  RandomPixelBlender m_pixelBlender;
  auto UpdatePixelBlender() noexcept -> void;

  WeightedColorMaps m_weightedColorMaps;
  std::unique_ptr<Raindrops> m_raindrops;
  auto SetNumRaindrops() noexcept -> void;
  auto SetRaindropsWeightedColorMaps() noexcept -> void;
};

RaindropsFx::RaindropsFx(FxHelper& fxHelper) noexcept
  : m_pimpl{spimpl::make_unique_impl<RaindropsFxImpl>(fxHelper)}
{
}

auto RaindropsFx::GetFxName() const noexcept -> std::string
{
  return "raindrops";
}

auto RaindropsFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto RaindropsFx::Finish() noexcept -> void
{
  // nothing to do
}

auto RaindropsFx::ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pimpl->ChangePixelBlender(pixelBlenderParams);
}

auto RaindropsFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_pimpl->SetZoomMidpoint(zoomMidpoint);
}

auto RaindropsFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto RaindropsFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return RaindropsFxImpl::GetCurrentColorMapsNames();
}

auto RaindropsFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

RaindropsFx::RaindropsFxImpl::RaindropsFxImpl(FxHelper& fxHelper) noexcept
  : m_fxHelper{&fxHelper}, m_pixelBlender{fxHelper.GetGoomRand()}
{
}

auto RaindropsFx::RaindropsFxImpl::Start() noexcept -> void
{
  // clang-format off
  const auto raindropRectangle = Rectangle2dInt{
      .topLeft=Point2dInt{.x=0, .y=0},
      .bottomRight=Point2dInt{.x=m_fxHelper->GetDimensions().GetIntWidth() - 1,
                 .y=m_fxHelper->GetDimensions().GetIntHeight() - 1}
  };
  // clang-format on

  m_raindrops = std::make_unique<Raindrops>(*m_fxHelper,
                                            Raindrops::NUM_START_RAINDROPS,
                                            m_weightedColorMaps.mainColorMaps,
                                            m_weightedColorMaps.lowColorMaps,
                                            raindropRectangle,
                                            m_zoomMidpoint);

  SetRaindropsWeightedColorMaps();
}

auto RaindropsFx::RaindropsFxImpl::ChangePixelBlender(
    const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  m_pixelBlender.SetPixelBlendType(pixelBlenderParams);
}

inline auto RaindropsFx::RaindropsFxImpl::UpdatePixelBlender() noexcept -> void
{
  m_fxHelper->GetDraw().SetPixelBlendFunc(m_pixelBlender.GetCurrentPixelBlendFunc());
  m_pixelBlender.Update();
}

auto RaindropsFx::RaindropsFxImpl::SetZoomMidpoint(
    [[maybe_unused]] const Point2dInt& zoomMidpoint) noexcept -> void
{
  if (nullptr == m_raindrops)
  {
    return;
  }

  m_raindrops->SetRectangleWeightPoint(zoomMidpoint);
}

auto RaindropsFx::RaindropsFxImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_weightedColorMaps = weightedColorMaps;

  SetNumRaindrops();
  SetRaindropsWeightedColorMaps();
}

auto RaindropsFx::RaindropsFxImpl::SetNumRaindrops() noexcept -> void
{
  if (nullptr == m_raindrops)
  {
    return;
  }

  m_raindrops->SetNumRaindrops(
      m_fxHelper->GetGoomRand().GetRandInRange<Raindrops::NUM_RAINDROPS_RANGE>());
}

auto RaindropsFx::RaindropsFxImpl::SetRaindropsWeightedColorMaps() noexcept -> void
{
  if (nullptr == m_raindrops)
  {
    return;
  }

  m_raindrops->SetWeightedColorMaps(m_weightedColorMaps.mainColorMaps,
                                    m_weightedColorMaps.lowColorMaps);
}

auto RaindropsFx::RaindropsFxImpl::GetCurrentColorMapsNames() noexcept -> std::vector<std::string>
{
  return {};
}

auto RaindropsFx::RaindropsFxImpl::ApplyToImageBuffers() noexcept -> void
{
  m_fxHelper->GetBlend2dContexts().blend2dBuffersWereUsed = true;

  m_raindrops->DrawRaindrops();
  m_raindrops->UpdateRaindrops();
}


} // namespace GOOM::VISUAL_FX
