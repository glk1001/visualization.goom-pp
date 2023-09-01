#pragma once

#include "goom/point2d.h"
#include "normalized_coords.h"

#include <cstdint>
#include <memory>
#include <span> // NOLINT: Waiting to use C++20.
#include <vector>

namespace GOOM::FILTER_FX
{

template<class FilterStriper>
class ZoomFilterBuffers
{
public:
  enum class TransformBufferState
  {
    START_FRESH_TRANSFORM_BUFFER,
    RESET_TRANSFORM_BUFFER,
    TRANSFORM_BUFFER_READY_FOR_UPDATES,
  };

  explicit ZoomFilterBuffers(std::unique_ptr<FilterStriper> filterStriper) noexcept;

  auto Start() noexcept -> void;

  auto SetTransformBufferMidpoint(const Point2dInt& val) noexcept -> void;
  auto SetFilterViewport(const Viewport& val) noexcept -> void;

  auto NotifyFilterSettingsHaveChanged() noexcept -> void;

  [[nodiscard]] auto IsTransformBufferReady() const noexcept -> bool;
  [[nodiscard]] auto GetPreviousTransformBuffer() const noexcept -> const std::vector<Point2dFlt>&;
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  auto CopyTransformBuffer(std_spn::span<Point2dFlt> destBuff) noexcept -> void;
  auto RestartTransformBuffer() noexcept -> void;

  auto UpdateTransformBuffer() noexcept -> void;
  [[nodiscard]] auto GetTransformBufferState() const noexcept -> TransformBufferState;

protected:
  // For testing only.
  [[nodiscard]] auto GetTransformBufferBuffMidpoint() const noexcept -> Point2dInt;
  [[nodiscard]] auto GetTransformBufferYLineStart() const noexcept -> uint32_t;
  [[nodiscard]] auto HaveFilterSettingsChanged() const noexcept -> bool;

private:
  std::unique_ptr<FilterStriper> m_filterStriper;

  bool m_filterSettingsHaveChanged = false;
  TransformBufferState m_transformBufferState =
      TransformBufferState::TRANSFORM_BUFFER_READY_FOR_UPDATES;

  auto InitTransformBuffer() noexcept -> void;
  auto StartFreshTranBuffer() noexcept -> void;
  auto ResetTransformBuffer() noexcept -> void;
  auto UpdateNextTransformBufferStripe() noexcept -> void;
};

template<class FilterStriper>
ZoomFilterBuffers<FilterStriper>::ZoomFilterBuffers(
    std::unique_ptr<FilterStriper> filterStriper) noexcept
  : m_filterStriper{std::move(filterStriper)}
{
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::IsTransformBufferReady() const noexcept -> bool
{
  return m_filterStriper->IsTransformBufferReady();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::CopyTransformBuffer(
    // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
    std_spn::span<Point2dFlt> destBuff) noexcept -> void
{
  m_filterStriper->CopyTransformBuffer(destBuff);
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::RestartTransformBuffer() noexcept -> void
{
  m_filterStriper->StartFreshTransformBuffer();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetTransformBufferBuffMidpoint() const noexcept
    -> Point2dInt
{
  return m_filterStriper->GetTransformBufferMidpoint();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::SetTransformBufferMidpoint(
    const Point2dInt& val) noexcept -> void
{
  m_filterStriper->SetTransformBufferMidpoint(val);
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::SetFilterViewport(const Viewport& val) noexcept
    -> void
{
  m_filterStriper->SetFilterViewport(val);
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetTransformBufferState() const noexcept
    -> TransformBufferState
{
  return m_transformBufferState;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::HaveFilterSettingsChanged() const noexcept -> bool
{
  return m_filterSettingsHaveChanged;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::NotifyFilterSettingsHaveChanged() noexcept -> void
{
  m_filterSettingsHaveChanged = true;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetTransformBufferYLineStart() const noexcept
    -> uint32_t
{
  return m_filterStriper->GetTransformBufferYLineStart();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::Start() noexcept -> void
{
  InitTransformBuffer();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::InitTransformBuffer() noexcept -> void
{
  m_filterStriper->InitTransformBuffer();
  m_transformBufferState = TransformBufferState::START_FRESH_TRANSFORM_BUFFER;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::UpdateTransformBuffer() noexcept -> void
{
  if (m_transformBufferState == TransformBufferState::RESET_TRANSFORM_BUFFER)
  {
    ResetTransformBuffer();
  }
  else if (m_transformBufferState == TransformBufferState::START_FRESH_TRANSFORM_BUFFER)
  {
    StartFreshTranBuffer();
  }
  else
  {
    Expects(m_transformBufferState == TransformBufferState::TRANSFORM_BUFFER_READY_FOR_UPDATES);
    UpdateNextTransformBufferStripe();
  }
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::StartFreshTranBuffer() noexcept -> void
{
  Expects(m_transformBufferState == TransformBufferState::START_FRESH_TRANSFORM_BUFFER);

  if (not m_filterSettingsHaveChanged)
  {
    return;
  }

  m_filterSettingsHaveChanged = false;
  m_filterStriper->ResetTransformBufferToStart();
  m_filterStriper->ResetTransformBufferIsReadyFlag();
  m_transformBufferState = TransformBufferState::TRANSFORM_BUFFER_READY_FOR_UPDATES;

  Ensures(m_transformBufferState == TransformBufferState::TRANSFORM_BUFFER_READY_FOR_UPDATES);
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::ResetTransformBuffer() noexcept -> void
{
  Expects(m_transformBufferState == TransformBufferState::RESET_TRANSFORM_BUFFER);

  m_filterStriper->ResetTransformBufferToStart();
  m_transformBufferState = TransformBufferState::START_FRESH_TRANSFORM_BUFFER;

  Ensures(m_transformBufferState == TransformBufferState::START_FRESH_TRANSFORM_BUFFER);
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::UpdateNextTransformBufferStripe() noexcept -> void
{
  Expects(m_transformBufferState == TransformBufferState::TRANSFORM_BUFFER_READY_FOR_UPDATES);

  m_filterStriper->UpdateNextTransformBufferStripe();
  if (0 == m_filterStriper->GetTransformBufferYLineStart())
  {
    m_transformBufferState = TransformBufferState::RESET_TRANSFORM_BUFFER;
  }
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetPreviousTransformBuffer() const noexcept
    -> const std::vector<Point2dFlt>&
{
  Expects(IsTransformBufferReady());
  return m_filterStriper->GetPreviousTransformBuffer();
}

} // namespace GOOM::FILTER_FX
