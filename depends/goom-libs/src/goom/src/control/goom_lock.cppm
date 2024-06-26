module;

#include <algorithm>
#include <cstdint>

export module Goom.Control.GoomMusicSettingsReactor:GoomLock;

export namespace GOOM::CONTROL
{

/* note pour ceux qui n'ont pas suivis : le GoomLock permet d'empecher un */
/* changement d'etat du plugin juste apres un autre changement d'etat. oki */
// -- Note for those who have not followed: the GoomLock prevents a change
// of state of the plugin just after another change of state.
class GoomLock
{
public:
  GoomLock() noexcept = default;

  [[nodiscard]] auto IsLocked() const -> bool;

  void Update();

  [[nodiscard]] auto GetLockTime() const -> uint32_t;
  void SetLockTime(uint32_t val);
  void IncreaseLockTime(uint32_t byAmount);

private:
  int32_t m_lockTime = 0;
};

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline auto GoomLock::IsLocked() const -> bool
{
  return m_lockTime > 0;
}

inline void GoomLock::Update()
{
  --m_lockTime;
  m_lockTime = std::max(m_lockTime, 0);
}

inline auto GoomLock::GetLockTime() const -> uint32_t
{
  return static_cast<uint32_t>(m_lockTime);
}

inline void GoomLock::SetLockTime(const uint32_t val)
{
  m_lockTime = static_cast<int32_t>(val);
}

inline void GoomLock::IncreaseLockTime(const uint32_t byAmount)
{
  m_lockTime += static_cast<int32_t>(byAmount);
}

} // namespace GOOM::CONTROL
