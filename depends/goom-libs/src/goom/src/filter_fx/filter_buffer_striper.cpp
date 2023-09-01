#include "filter_buffer_striper.h"

#include "goom/goom_config.h"
#include "goom/point2d.h"
#include "goom_plugin_info.h"
#include "normalized_coords.h"
#include "utils/parallel_utils.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace GOOM::FILTER_FX
{

using UTILS::Parallel;

ZoomFilterBufferStriper::ZoomFilterBufferStriper(
    Parallel& parallel,
    const PluginInfo& goomInfo,
    const NormalizedCoordsConverter& normalizedCoordsConverter,
    const ZoomPointFunc& getZoomPointFunc) noexcept
  : m_dimensions{goomInfo.GetDimensions()},
    m_normalizedCoordsConverter{&normalizedCoordsConverter},
    m_parallel{&parallel},
    m_getZoomPoint{getZoomPointFunc},
    m_transformBuffer(m_dimensions.GetSize()),
    m_previousTransformBuffer(m_dimensions.GetSize())
{
}

auto ZoomFilterBufferStriper::ResetTransformBufferToStart() noexcept -> void
{
  m_transformBufferYLineStart = 0;
}

auto ZoomFilterBufferStriper::ResetTransformBufferIsReadyFlag() noexcept -> void
{
  m_transformBufferUpdateStatus = TransformBufferUpdateStatus::IN_PROGRESS;
}

/*
 * Makes a stripe of a transform buffer
 *
 * The transform is (in order) :
 * Translation (-data->middleX, -data->middleY)
 * Homothetie (Center : 0,0   Coeff : 2/data->screenWidth)
 */
auto ZoomFilterBufferStriper::DoNextStripe(const uint32_t transformBufferStripeHeight) noexcept
    -> void
{
  Expects(m_transformBuffer.size() == m_dimensions.GetSize());
  Expects(m_transformBufferUpdateStatus == TransformBufferUpdateStatus::IN_PROGRESS);

  const auto screenWidth                  = m_dimensions.GetWidth();
  const auto screenSpan                   = static_cast<float>(screenWidth - 1);
  const auto sourceCoordsStepSize         = NormalizedCoords::COORD_WIDTH / screenSpan;
  const auto sourceViewportCoordsStepSize = m_filterViewport.GetViewportWidth() / screenSpan;

  const auto doStripeLine =
      [this, &screenWidth, &sourceCoordsStepSize, &sourceViewportCoordsStepSize](const size_t y)
  {
    // Y-position of the first stripe pixel to compute in screen coordinates.
    const auto yScreenCoord = static_cast<uint32_t>(y) + m_transformBufferYLineStart;
    auto tranBufferPos      = yScreenCoord * screenWidth;

    auto centredSourceCoords =
        m_normalizedCoordsConverter->OtherToNormalizedCoords(GetPoint2dInt(0U, yScreenCoord)) -
        m_normalizedMidpoint;
    auto centredSourceViewportCoords = m_filterViewport.GetViewportCoords(centredSourceCoords);

    for (auto x = 0U; x < screenWidth; ++x)
    {
      const auto zoomPoint = m_getZoomPoint(centredSourceCoords, centredSourceViewportCoords);
      const auto uncenteredZoomPoint = m_normalizedMidpoint + zoomPoint;

      m_transformBuffer[tranBufferPos] = uncenteredZoomPoint.GetFltCoords();

      centredSourceCoords.IncX(sourceCoordsStepSize);
      centredSourceViewportCoords.IncX(sourceViewportCoordsStepSize);
      ++tranBufferPos;
    }
  };

  // Where (vertically) to stop generating the buffer stripe.
  const auto tranBuffYLineEnd =
      std::min(m_dimensions.GetHeight(), m_transformBufferYLineStart + transformBufferStripeHeight);
  const auto numStripes = static_cast<size_t>(tranBuffYLineEnd - m_transformBufferYLineStart);

  m_parallel->ForLoop(numStripes, doStripeLine);

  m_transformBufferYLineStart += transformBufferStripeHeight;
  if (tranBuffYLineEnd >= m_dimensions.GetHeight())
  {
    m_transformBufferYLineStart   = 0;
    m_transformBufferUpdateStatus = TransformBufferUpdateStatus::READY_TO_COPY;
  }
}

} // namespace GOOM::FILTER_FX
