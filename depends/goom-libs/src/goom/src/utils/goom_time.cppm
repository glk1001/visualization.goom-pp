module;

#include <cstdint>
#include <limits>

export module Goom.Utils.GoomTime;

import Goom.Lib.AssertUtils;

export namespace GOOM::UTILS
{

class GoomTime
{
public:
  auto ResetTime() noexcept -> void;
  auto UpdateTime() noexcept -> void;

  [[nodiscard]] auto GetCurrentTime() const noexcept -> uint64_t;
  [[nodiscard]] auto GetElapsedTimeSince(uint64_t time0) const noexcept -> uint64_t;

private:
  uint64_t m_numUpdates          = 0U;
  static constexpr auto MAX_TIME = std::numeric_limits<uint64_t>::max();
};

} // namespace GOOM::UTILS

namespace GOOM::UTILS
{

inline auto GoomTime::ResetTime() noexcept -> void
{
  m_numUpdates = 0U;
}

inline auto GoomTime::UpdateTime() noexcept -> void
{
  Expects(m_numUpdates < MAX_TIME);
  ++m_numUpdates;
}

inline auto GoomTime::GetCurrentTime() const noexcept -> uint64_t
{
  return m_numUpdates;
}

inline auto GoomTime::GetElapsedTimeSince(const uint64_t time0) const noexcept -> uint64_t
{
  Expects(m_numUpdates >= time0);

  return m_numUpdates - time0;
}

} // namespace GOOM::UTILS
