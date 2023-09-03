#pragma once

#include "goom/point2d.h"
#include "normalized_coords.h"

#include <memory>
#include <span> // NOLINT: Waiting to use C++20.
#include <vector>

namespace GOOM::FILTER_FX
{

template<class FilterStriper>
class ZoomFilterBuffers
{
public:
  explicit ZoomFilterBuffers(std::unique_ptr<FilterStriper> filterStriper) noexcept;

  auto Start() noexcept -> void;

  auto SetTransformBufferMidpoint(const Point2dInt& val) noexcept -> void;
  auto SetFilterViewport(const Viewport& val) noexcept -> void;

  [[nodiscard]] auto IsTransformBufferInProgress() const noexcept -> bool;
  [[nodiscard]] auto IsTransformBufferReadyToCopy() const noexcept -> bool;
  [[nodiscard]] auto HasTransformBufferBeenCopied() const noexcept -> bool;
  [[nodiscard]] auto GetPreviousTransformBuffer() const noexcept -> const std::vector<Point2dFlt>&;
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  auto CopyTransformBuffer(std_spn::span<Point2dFlt> destBuff) noexcept -> void;
  auto StartFreshTranBuffer() noexcept -> void;

  auto UpdateTransformBuffer() noexcept -> void;

protected:
  // For testing only.
  [[nodiscard]] auto GetTransformBufferBuffMidpoint() const noexcept -> Point2dInt;

private:
  std::unique_ptr<FilterStriper> m_filterStriper;

  auto UpdateNextTransformBufferStripe() noexcept -> void;
};

template<class FilterStriper>
ZoomFilterBuffers<FilterStriper>::ZoomFilterBuffers(
    std::unique_ptr<FilterStriper> filterStriper) noexcept
  : m_filterStriper{std::move(filterStriper)}
{
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::IsTransformBufferInProgress() const noexcept -> bool
{
  return m_filterStriper->GetTransformBufferUpdateStatus() ==
         ZoomFilterBufferStriper::TransformBufferUpdateStatus::IN_PROGRESS;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::IsTransformBufferReadyToCopy() const noexcept -> bool
{
  return m_filterStriper->GetTransformBufferUpdateStatus() ==
         ZoomFilterBufferStriper::TransformBufferUpdateStatus::READY_TO_COPY;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::HasTransformBufferBeenCopied() const noexcept -> bool
{
  return m_filterStriper->GetTransformBufferUpdateStatus() ==
         ZoomFilterBufferStriper::TransformBufferUpdateStatus::HAS_BEEN_COPIED;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::CopyTransformBuffer(
    // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
    std_spn::span<Point2dFlt> destBuff) noexcept -> void
{
  m_filterStriper->CopyTransformBuffer(destBuff);
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
inline auto ZoomFilterBuffers<FilterStriper>::Start() noexcept -> void
{
  m_filterStriper->Start();
  Ensures(m_filterStriper->GetTransformBufferUpdateStatus() ==
          ZoomFilterBufferStriper::TransformBufferUpdateStatus::IN_PROGRESS);
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::UpdateTransformBuffer() noexcept -> void
{
  if (m_filterStriper->GetTransformBufferUpdateStatus() ==
      ZoomFilterBufferStriper::TransformBufferUpdateStatus::HAS_BEEN_COPIED)
  {
    return;
  }

  Expects(m_filterStriper->GetTransformBufferUpdateStatus() ==
          ZoomFilterBufferStriper::TransformBufferUpdateStatus::IN_PROGRESS);

  UpdateNextTransformBufferStripe();

  Ensures((m_filterStriper->GetTransformBufferUpdateStatus() ==
           ZoomFilterBufferStriper::TransformBufferUpdateStatus::IN_PROGRESS) or
          (m_filterStriper->GetTransformBufferUpdateStatus() ==
           ZoomFilterBufferStriper::TransformBufferUpdateStatus::READY_TO_COPY));
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::StartFreshTranBuffer() noexcept -> void
{
  Expects(m_filterStriper->GetTransformBufferUpdateStatus() ==
          ZoomFilterBufferStriper::TransformBufferUpdateStatus::HAS_BEEN_COPIED);

  m_filterStriper->ResetTransformBufferToStart();

  Ensures(m_filterStriper->GetTransformBufferUpdateStatus() ==
          ZoomFilterBufferStriper::TransformBufferUpdateStatus::IN_PROGRESS);
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::UpdateNextTransformBufferStripe() noexcept -> void
{
  m_filterStriper->UpdateNextTransformBufferStripe();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetPreviousTransformBuffer() const noexcept
    -> const std::vector<Point2dFlt>&
{
  Expects(IsTransformBufferReadyToCopy());
  return m_filterStriper->GetPreviousTransformBuffer();
}

} // namespace GOOM::FILTER_FX
