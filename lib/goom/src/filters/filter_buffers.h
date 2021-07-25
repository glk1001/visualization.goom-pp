#ifndef VISUALIZATION_GOOM_FILTER_BUFFERS_H
#define VISUALIZATION_GOOM_FILTER_BUFFERS_H

#include "goom_graphic.h"
#include "v2d.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{

class PluginInfo;
class PixelBuffer;

namespace UTILS
{
class Parallel;
} // namespace UTILS

namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

class NormalizedCoords;

class ZoomFilterBuffers
{
public:
  static constexpr int32_t DIM_FILTER_COEFFS = 16;
  static constexpr size_t NUM_NEIGHBOR_COEFFS = 4;
  using NeighborhoodCoeffArray = union
  {
    std::array<uint8_t, NUM_NEIGHBOR_COEFFS> c;
    uint32_t intVal = 0;
  };
  using FilterCoeff2dArray =
      std::array<std::array<NeighborhoodCoeffArray, DIM_FILTER_COEFFS>, DIM_FILTER_COEFFS>;
  using NeighborhoodPixelArray = std::array<Pixel, NUM_NEIGHBOR_COEFFS>;

  enum class TranBufferState
  {
    _NULL = -1,
    RESTART_TRAN_BUFFER,
    RESET_TRAN_BUFFER,
    TRAN_BUFFER_READY,
  };

  ZoomFilterBuffers(UTILS::Parallel& p, const std::shared_ptr<const PluginInfo>& goomInfo);
  ZoomFilterBuffers(const ZoomFilterBuffers&) noexcept = delete;
  ZoomFilterBuffers(ZoomFilterBuffers&&) noexcept = delete;
  ~ZoomFilterBuffers() noexcept;
  auto operator=(const ZoomFilterBuffers&) -> ZoomFilterBuffers& = delete;
  auto operator=(ZoomFilterBuffers&&) -> ZoomFilterBuffers& = delete;

  using ZoomPointFunc = std::function<NormalizedCoords(const NormalizedCoords& normalizedCoords)>;
  auto GetZoomPointFunc() const -> ZoomPointFunc;
  void SetZoomPointFunc(const ZoomPointFunc& f);

  auto GetTranLerpFactor() const -> int32_t;
  void SetTranLerpFactor(int32_t val);
  static auto GetMaxTranLerpFactor() -> int32_t;

  auto GetTranBuffYLineStart() const -> uint32_t;

  auto GetBuffMidPoint() const -> V2dInt;
  void SetBuffMidPoint(const V2dInt& val);

  void Start();

  void SettingsChanged();
  void UpdateTranBuffer();
  auto GetTranBufferState() const -> TranBufferState;
  auto GetZoomBufferSrceDestLerp(size_t buffPos) const -> V2dInt;
  auto IsTranPointClipped(const V2dInt& tranPoint) const -> bool;
  auto GetSourceInfo(const V2dInt& tranPoint) const -> std::tuple<V2dInt, NeighborhoodCoeffArray>;

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;
  const uint32_t m_bufferSize;

  class FilterCoefficients;
  const std::unique_ptr<const FilterCoefficients> m_precalculatedCoeffs;

  auto GetMaxTranX() const -> uint32_t;
  auto GetMaxTranY() const -> uint32_t;

  static auto NormalizedToTranPoint(const NormalizedCoords& normalizedPoint) -> V2dInt;

  UTILS::Parallel* const m_parallel;

  ZoomPointFunc m_getZoomPoint{};

  V2dInt m_buffMidPoint{};
  bool m_settingsChanged = false;
  // modification by jeko : fixedpoint : tranDiffFactor = (16:16) (0 <= tranDiffFactor <= 2^16)
  int32_t m_tranLerpFactor; // in [0, BUFF_POINT_MASK]

  std::vector<int32_t> m_tranXSrce{};
  std::vector<int32_t> m_tranYSrce{};
  std::vector<int32_t> m_tranXDest{};
  std::vector<int32_t> m_tranYDest{};
  std::vector<int32_t> m_tranXTemp{};
  std::vector<int32_t> m_tranYTemp{};
  const V2dInt m_maxTranPoint;
  const uint32_t m_tranBuffStripeHeight;
  uint32_t m_tranBuffYLineStart;
  TranBufferState m_tranBufferState;

  std::vector<int32_t> m_firedec{};

  void InitTranBuffer();
  void RestartTranBuffer();
  void ResetTranBuffer();
  void DoNextTranBufferStripe(uint32_t tranBuffStripeHeight);
  void GenerateWaterFxHorizontalBuffer();
  auto GetTranPoint(const NormalizedCoords& normalized) const -> V2dInt;
};

inline auto ZoomFilterBuffers::GetBuffMidPoint() const -> V2dInt
{
  return m_buffMidPoint;
}

inline void ZoomFilterBuffers::SetBuffMidPoint(const V2dInt& val)
{
  m_buffMidPoint = val;
}

inline auto ZoomFilterBuffers::GetTranBufferState() const -> TranBufferState
{
  return m_tranBufferState;
}

inline auto ZoomFilterBuffers::GetTranLerpFactor() const -> int32_t
{
  return m_tranLerpFactor;
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

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::DRAW
#endif

#endif