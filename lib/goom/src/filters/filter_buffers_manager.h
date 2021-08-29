#ifndef VISUALIZATION_GOOM_FILTER_BUFFERS_H
#define VISUALIZATION_GOOM_FILTER_BUFFERS_H

#include "filter_normalized_coords.h"
#include "goom_graphic.h"
#include "v2d.h"

#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace GOOM
{

class FilterStats;
class PluginInfo;
class PixelBuffer;

namespace UTILS
{
class Parallel;
} // namespace UTILS

namespace FILTERS
{

class ZoomFilterBuffers
{
public:
  static constexpr int32_t DIM_FILTER_COEFFS = 16;
  static constexpr size_t NUM_NEIGHBOR_COEFFS = 4;
  using NeighborhoodPixelArray = std::array<Pixel, NUM_NEIGHBOR_COEFFS>;
  class CoordTransforms;

  enum class TranBuffersState
  {
    _NULL = -1,
    START_FRESH_TRAN_BUFFERS,
    RESET_TRAN_BUFFERS,
    TRAN_BUFFERS_READY,
  };

  using ZoomPointFunc = std::function<NormalizedCoords(const NormalizedCoords& normalizedCoords)>;

  ZoomFilterBuffers(UTILS::Parallel& p,
                    const std::shared_ptr<const PluginInfo>& goomInfo,
                    const ZoomPointFunc& zoomPointFunc,
                    FilterStats& stats);

  [[nodiscard]] auto GetTranLerpFactor() const -> int32_t;
  void SetTranLerpFactor(int32_t val);
  [[nodiscard]] static auto GetMaxTranLerpFactor() -> int32_t;

  [[nodiscard]] auto GetTranBuffYLineStart() const -> uint32_t;

  [[nodiscard]] auto GetBuffMidPoint() const -> V2dInt;
  void SetBuffMidPoint(const V2dInt& val);

  void Start();

  void NotifyFilterSettingsHaveChanged();
  auto HaveFilterSettingsChanged() const -> bool;

  void UpdateTranBuffers();
  [[nodiscard]] auto GetTranBuffersState() const -> TranBuffersState;
  [[nodiscard]] auto GetZoomBufferTranPoint(size_t buffPos) const -> V2dInt;
  [[nodiscard]] auto IsTranPointClipped(const V2dInt& tranPoint) const -> bool;

  using NeighborhoodCoeffArray = struct
  {
    std::array<uint32_t, NUM_NEIGHBOR_COEFFS> val;
    bool isZero;
  };
  [[nodiscard]] auto GetSourcePointInfo(const V2dInt& tranPoint) const
      -> std::pair<V2dInt, NeighborhoodCoeffArray>;

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;
  FilterStats& m_stats;

  class FilterCoefficients;
  const std::unique_ptr<const FilterCoefficients> m_precalculatedCoeffs;

  [[nodiscard]] auto GetMaxTranX() const -> uint32_t;
  [[nodiscard]] auto GetMaxTranY() const -> uint32_t;

  UTILS::Parallel& m_parallel;
  const ZoomPointFunc m_getZoomPoint;
  class TransformBuffers;
  std::unique_ptr<TransformBuffers> m_transformBuffers;

  V2dInt m_buffMidPoint{};
  bool m_filterSettingsHaveChanged = false;

  const V2dInt m_maxTranPoint;
  const uint32_t m_tranBuffStripeHeight;
  uint32_t m_tranBuffYLineStart = 0;
  TranBuffersState m_tranBuffersState = TranBuffersState::TRAN_BUFFERS_READY;

  std::vector<int32_t> m_firedec{};

  static constexpr float MIN_SCREEN_COORD_ABS_VAL = 1.0F / static_cast<float>(DIM_FILTER_COEFFS);

  void InitAllTranBuffers();
  void StartFreshTranBuffers();
  void ResetTranBuffers();
  void FillTempTranBuffers();
  void DoNextTempTranBuffersStripe(uint32_t tranBuffStripeHeight);
  void GenerateWaterFxHorizontalBuffer();
  [[nodiscard]] static auto GetTranPoint(const NormalizedCoords& normalized) -> V2dInt;
};

class ZoomFilterBuffers::CoordTransforms
{
public:
  // Use these consts for optimising multiplication, division, and mod, by DIM_FILTER_COEFFS.
  static constexpr int32_t MAX_TRAN_LERP_VALUE = 0xFFFF;
  static constexpr int32_t DIM_FILTER_COEFFS_DIV_SHIFT = 4;
  static constexpr int32_t DIM_FILTER_COEFFS_MOD_MASK = 0xF;

  [[nodiscard]] static auto TranCoordToCoeffIndex(uint32_t tranCoord) -> uint32_t;
  [[nodiscard]] static auto TranToScreenPoint(const V2dInt& tranPoint) -> V2dInt;
  [[nodiscard]] static auto ScreenToTranPoint(const V2dInt& screenPoint) -> V2dInt;
  [[nodiscard]] static auto ScreenToTranCoord(float screenCoord) -> uint32_t;

  [[nodiscard]] static auto NormalizedToTranPoint(const NormalizedCoords& normalizedPoint)
      -> V2dInt;
};

class ZoomFilterBuffers::FilterCoefficients
{
public:
  FilterCoefficients() noexcept;

  using FilterCoeff2dArray =
      std::array<std::array<NeighborhoodCoeffArray, DIM_FILTER_COEFFS>, DIM_FILTER_COEFFS>;
  [[nodiscard]] auto GetCoeffs() const -> const FilterCoeff2dArray&;

private:
  // modif d'optim by Jeko : precalcul des 4 coeffs resultant des 2 pos
  const FilterCoeff2dArray m_precalculatedCoeffs{GetPrecalculatedCoefficients()};
  [[nodiscard]] static auto GetPrecalculatedCoefficients() -> FilterCoeff2dArray;
};

class ZoomFilterBuffers::TransformBuffers
{
public:
  TransformBuffers(uint32_t screenWidth, uint32_t screenHeight) noexcept;

  void SetSrceTranToIdentity();
  void CopyTempTranToDestTran();
  void CopyDestTranToSrceTran();
  void SetUpNextDestTran();

  void SetTempBuffersTransformPoint(uint32_t pos, const V2dInt& transformPoint);

  [[nodiscard]] auto GetTranLerpFactor() const -> int32_t;
  void SetTranLerpFactor(int32_t val);

  [[nodiscard]] auto GetSrceDestLerpBufferPoint(const size_t buffPos) const -> V2dInt;

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;
  const uint32_t m_bufferSize;
  std::vector<int32_t> m_tranXSrce{};
  std::vector<int32_t> m_tranYSrce{};
  std::vector<int32_t> m_tranXDest{};
  std::vector<int32_t> m_tranYDest{};
  std::vector<int32_t> m_tranXTemp{};
  std::vector<int32_t> m_tranYTemp{};
  int32_t m_tranLerpFactor = 0;

  void CopyAllDestTranToSrceTran();
  void CopyUnlerpedDestTranToSrceTran();
  [[nodiscard]] static auto GetTranBuffLerpVal(int32_t srceBuffVal, int32_t destBuffVal, int32_t t)
      -> int32_t;
};

inline auto ZoomFilterBuffers::CoordTransforms::TranCoordToCoeffIndex(const uint32_t tranCoord)
    -> uint32_t
{
  return tranCoord & DIM_FILTER_COEFFS_MOD_MASK;
}

inline auto ZoomFilterBuffers::CoordTransforms::TranToScreenPoint(const V2dInt& tranPoint) -> V2dInt
{
  return {tranPoint.x >> DIM_FILTER_COEFFS_DIV_SHIFT, tranPoint.y >> DIM_FILTER_COEFFS_DIV_SHIFT};
}

inline auto ZoomFilterBuffers::CoordTransforms::ScreenToTranPoint(const V2dInt& screenPoint)
    -> V2dInt
{
  return {screenPoint.x << DIM_FILTER_COEFFS_DIV_SHIFT,
          screenPoint.y << DIM_FILTER_COEFFS_DIV_SHIFT};
}

inline auto ZoomFilterBuffers::CoordTransforms::ScreenToTranCoord(const float screenCoord)
    -> uint32_t
{
  // IMPORTANT: Without 'lround' a faint cross artifact appears in the centre of the screen.
  return static_cast<uint32_t>(std::lround(screenCoord * static_cast<float>(DIM_FILTER_COEFFS)));
}

inline auto ZoomFilterBuffers::CoordTransforms::NormalizedToTranPoint(
    const NormalizedCoords& normalizedPoint) -> V2dInt
{
  const V2dFlt screenCoords = normalizedPoint.GetScreenCoordsFlt();

  // IMPORTANT: Without 'lround' a faint cross artifact appears in the centre of the screen.
  return {static_cast<int32_t>(std::lround(ScreenToTranCoord(screenCoords.x))),
          static_cast<int32_t>(std::lround(ScreenToTranCoord(screenCoords.y)))};
}

inline auto ZoomFilterBuffers::GetBuffMidPoint() const -> V2dInt
{
  return m_buffMidPoint;
}

inline void ZoomFilterBuffers::SetBuffMidPoint(const V2dInt& val)
{
  m_buffMidPoint = val;
}

inline auto ZoomFilterBuffers::GetTranBuffersState() const -> TranBuffersState
{
  return m_tranBuffersState;
}

inline auto ZoomFilterBuffers::GetTranLerpFactor() const -> int32_t
{
  return m_transformBuffers->GetTranLerpFactor();
}

inline auto ZoomFilterBuffers::GetMaxTranLerpFactor() -> int32_t
{
  return CoordTransforms::MAX_TRAN_LERP_VALUE;
}

inline void ZoomFilterBuffers::SetTranLerpFactor(const int32_t val)
{
  m_transformBuffers->SetTranLerpFactor(val);
}

inline auto ZoomFilterBuffers::GetMaxTranX() const -> uint32_t
{
  return static_cast<uint32_t>(m_maxTranPoint.x);
}

inline auto ZoomFilterBuffers::GetMaxTranY() const -> uint32_t
{
  return static_cast<uint32_t>(m_maxTranPoint.y);
}

inline auto ZoomFilterBuffers::GetTranBuffYLineStart() const -> uint32_t
{
  return m_tranBuffYLineStart;
}

inline auto ZoomFilterBuffers::IsTranPointClipped(const V2dInt& tranPoint) const -> bool
{
  return (tranPoint.x < 0) || (tranPoint.y < 0) ||
         (static_cast<uint32_t>(tranPoint.x) >= GetMaxTranX()) ||
         (static_cast<uint32_t>(tranPoint.y) >= GetMaxTranY());
}

inline auto ZoomFilterBuffers::GetZoomBufferTranPoint(const size_t buffPos) const -> V2dInt
{
  return m_transformBuffers->GetSrceDestLerpBufferPoint(buffPos);
}

inline auto ZoomFilterBuffers::TransformBuffers::GetTranLerpFactor() const -> int32_t
{
  return m_tranLerpFactor;
}

inline void ZoomFilterBuffers::TransformBuffers::SetTranLerpFactor(const int32_t val)
{
  m_tranLerpFactor = val;
}

inline auto ZoomFilterBuffers::TransformBuffers::GetSrceDestLerpBufferPoint(
    const size_t buffPos) const -> V2dInt
{
  return {GetTranBuffLerpVal(m_tranXSrce[buffPos], m_tranXDest[buffPos], m_tranLerpFactor),
          GetTranBuffLerpVal(m_tranYSrce[buffPos], m_tranYDest[buffPos], m_tranLerpFactor)};
}

inline auto ZoomFilterBuffers::TransformBuffers::GetTranBuffLerpVal(const int32_t srceBuffVal,
                                                                    const int32_t destBuffVal,
                                                                    const int32_t t) -> int32_t
{
  // IMPORTANT: Looking at this mathematically I can't see that the '-1' should be there.
  //            But without it, slight static artifacts appear in the centre of the image.
  //            Originally, Goom used '>> DIM_FILTER_COEFFS' instead of the division,
  //            which was OK, but the '-1' is better.
  return srceBuffVal + ((t * (destBuffVal - srceBuffVal))  >> DIM_FILTER_COEFFS);
}

} // namespace FILTERS
} // namespace GOOM

#endif
