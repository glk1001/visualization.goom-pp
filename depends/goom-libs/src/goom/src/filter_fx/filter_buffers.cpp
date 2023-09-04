#include "filter_buffers.h"

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

ZoomFilterBuffers::ZoomFilterBuffers(Parallel& parallel,
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

auto ZoomFilterBuffers::Start() noexcept -> void
{
  Expects(m_transformBuffer.size() == m_dimensions.GetSize());

  // Make sure the previous transform buffer is filled and valid and
  // the current buffer is ready to be updated.

  ResetTransformBufferToStart();
  StartTransformBufferUpdates();

  Expects(UpdateStatus::IN_PROGRESS == m_updateStatus);
  DoNextStripe(m_dimensions.GetHeight());
  Expects(UpdateStatus::AT_END == m_updateStatus);

  std::swap(m_previousTransformBuffer, m_transformBuffer);

  ResetTransformBufferToStart();
  StartTransformBufferUpdates();
  Ensures(UpdateStatus::IN_PROGRESS == m_updateStatus);
}

auto ZoomFilterBuffers::Finish() noexcept -> void
{
  ResetTransformBufferToStart();
}

auto ZoomFilterBuffers::ResetTransformBufferToStart() noexcept -> void
{
  m_transformBufferYLineStart = 0;
  m_updateStatus              = UpdateStatus::AT_START;
}

auto ZoomFilterBuffers::StartTransformBufferUpdates() noexcept -> void
{
  Expects(UpdateStatus::AT_START == m_updateStatus);

  m_updateStatus = UpdateStatus::IN_PROGRESS;
}

auto ZoomFilterBuffers::UpdateTransformBuffer() noexcept -> void
{
  if (UpdateStatus::HAS_BEEN_COPIED == m_updateStatus)
  {
    return;
  }

  Expects(UpdateStatus::IN_PROGRESS == m_updateStatus);

  DoNextStripe(m_transformBufferStripeHeight);

  Ensures((UpdateStatus::IN_PROGRESS == m_updateStatus) or
          (UpdateStatus::AT_END == m_updateStatus));
}

/*
 * Makes a stripe of a transform buffer
 *
 * The transform is (in order) :
 * Translation (-data->middleX, -data->middleY)
 * Homothetie (Center : 0,0   Coeff : 2/data->screenWidth)
 */
auto ZoomFilterBuffers::DoNextStripe(const uint32_t transformBufferStripeHeight) noexcept -> void
{
  Expects(UpdateStatus::IN_PROGRESS == m_updateStatus);

  const auto screenWidth                  = m_dimensions.GetWidth();
  const auto screenSpan                   = static_cast<float>(screenWidth - 1);
  const auto sourceCoordsStepSize         = NormalizedCoords::COORD_WIDTH / screenSpan;
  const auto sourceViewportCoordsStepSize = m_filterViewport.GetViewportWidth() / screenSpan;

  const auto doTransformBufferRow =
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

  m_parallel->ForLoop(numStripes, doTransformBufferRow);

  m_transformBufferYLineStart += transformBufferStripeHeight;
  if (tranBuffYLineEnd >= m_dimensions.GetHeight())
  {
    m_updateStatus = UpdateStatus::AT_END;
  }
}

} // namespace GOOM::FILTER_FX
