module;

#include <algorithm>
#include <cstddef>
#include <span>
#include <vector>

export module Goom.Lib.CircularBuffer;

import Goom.Lib.AssertUtils;

export namespace GOOM
{

template<typename T>
class CircularBuffer
{
public:
  explicit CircularBuffer(size_t size) noexcept;

  auto Clear() noexcept -> void;

  [[nodiscard]] auto BufferLength() const noexcept -> size_t;
  [[nodiscard]] auto DataAvailable() const noexcept -> size_t;
  [[nodiscard]] auto FreeSpace() const noexcept -> size_t;

  auto Write(std::span<const T> srce) noexcept -> void;
  auto Read(std::vector<T>& dest) noexcept -> void;

private:
  size_t m_size;
  std::vector<T> m_buffer;
  size_t m_readPtr  = 0;
  size_t m_writePtr = 0;
  size_t m_used     = 0;
};

} // namespace GOOM

namespace GOOM
{

template<typename T>
CircularBuffer<T>::CircularBuffer(const size_t size) noexcept : m_size{size}, m_buffer(size)
{
}

template<typename T>
auto CircularBuffer<T>::Clear() noexcept -> void
{
  m_readPtr  = 0;
  m_writePtr = 0;
  m_used     = 0;
}

template<typename T>
auto CircularBuffer<T>::BufferLength() const noexcept -> size_t
{
  return m_size;
}

template<typename T>
auto CircularBuffer<T>::DataAvailable() const noexcept -> size_t
{
  return m_used;
}

template<typename T>
auto CircularBuffer<T>::FreeSpace() const noexcept -> size_t
{
  Expects(m_used <= m_size);
  return m_size - m_used;
}

template<typename T>
auto CircularBuffer<T>::Write(const std::span<const T> srce) noexcept -> void
{
  Expects(srce.size() <= FreeSpace());

  size_t count     = srce.size();
  const T* srcePtr = srce.data();

  while (count)
  {
    size_t delta = m_size - m_writePtr;
    delta        = std::min(delta, count);
    std::copy(srcePtr, srcePtr + delta, begin(m_buffer) + static_cast<std::ptrdiff_t>(m_writePtr));

    m_used += delta;
    m_writePtr = (m_writePtr + delta) % m_size;

    srcePtr += delta;
    count -= delta;
  }
}

template<typename T>
auto CircularBuffer<T>::Read(std::vector<T>& dest) noexcept -> void
{
  Expects(dest.size() <= DataAvailable());

  size_t count = dest.size();
  T* destPtr   = dest.data();

  for (;;)
  {
    size_t delta = m_size - m_readPtr;
    delta        = std::min(delta, m_used);
    delta        = std::min(delta, count);

    if (0 == delta)
    {
      break;
    }

    std::copy(cbegin(m_buffer) + static_cast<std::ptrdiff_t>(m_readPtr),
              cbegin(m_buffer) + static_cast<std::ptrdiff_t>(m_readPtr + delta),
              destPtr);

    destPtr += delta;
    m_readPtr = (m_readPtr + delta) % m_size;
    count -= delta;
    m_used -= delta;
  }
}

// TODO(glk) Make this a unit test
// NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if)
#if 0 // Visual Studio not happy with this
static constexpr int SILENCE_THRESHOLD = 8;

  [[nodiscard]] auto TestSilence() const -> bool
  {
    auto* begin = static_cast<T*>(&m_buffer[0]);
    T first = *begin;
    *begin = SILENCE_THRESHOLD * 2;
    T* ptr = begin + m_size;
    while (static_cast<u_int32_t>((*--ptr + SILENCE_THRESHOLD) <=
                                  (static_cast<u_int32_t>(SILENCE_THRESHOLD) * 2)))
    {
    }
    *begin = first;
    return (ptr == begin) && (static_cast<u_int32_t>(first + SILENCE_THRESHOLD) <=
                              (static_cast<u_int32_t>(SILENCE_THRESHOLD) * 2));
  }
#endif

} // namespace GOOM
