module;

#undef NO_LOGGING

#include "goom/goom_logger.h"

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <string>

export module Goom.GoomVisualization:SlotProducerConsumer;

import Goom.Lib.AssertUtils;

export namespace GOOM
{

template<typename TResource>
class SlotProducerConsumer
{
public:
  SlotProducerConsumer(GoomLogger& goomLogger,
                       size_t maxInUseSlots,
                       const std::string& name) noexcept;
  SlotProducerConsumer(GoomLogger& goomLogger,
                       size_t maxInUseSlots,
                       const std::string& name,
                       size_t maxResourceItems) noexcept;

  auto Start() noexcept -> void;
  auto Stop() noexcept -> void;

  [[nodiscard]] auto HasFinished() const noexcept -> bool;

  [[nodiscard]] auto AddResource(const TResource& resource) noexcept -> bool;
  [[nodiscard]] auto ProduceWithoutRelease() noexcept -> bool;
  auto ReleaseAfterProduce(size_t slot) noexcept -> void;
  auto Produce() noexcept -> void;

  [[nodiscard]] auto ConsumeWithoutRelease(uint32_t waitMs) noexcept -> bool;
  auto ReleaseAfterConsume(size_t slot) noexcept -> void;
  auto Consume(uint32_t waitMs) noexcept -> void;

  [[nodiscard]] auto GetConsumeRequests() const noexcept -> uint64_t;
  [[nodiscard]] auto GetNumTimesConsumerGaveUpWaiting() const noexcept -> uint64_t;

  using ProduceItemFunc = std::function<void(size_t slot, const TResource& resource)>;
  auto SetProduceItemFunc(const ProduceItemFunc& produceItemFunc) noexcept -> void;

  using ProduceItemWithoutResourceFunc = std::function<void(size_t slot)>;
  auto SetProduceItemWithoutResourceFunc(
      const ProduceItemWithoutResourceFunc& produceItemWithoutResourceFunc) noexcept -> void;

  using ConsumeItemFunc = std::function<void(size_t slot)>;
  auto SetConsumeItemFunc(const ConsumeItemFunc& consumeItemFunc) noexcept -> void;

private:
  GoomLogger* m_goomLogger;
  std::string m_name;
  bool m_finished = false;
  std::mutex m_mutex;
  std::condition_variable m_producer_cv;
  std::condition_variable m_consumer_cv;
  std::condition_variable m_resourcer_cv;

  size_t m_maxInUseSlots;
  size_t m_maxResourceItems = 0U;
  std::queue<size_t> m_inUseSlotsQueue;
  std::queue<size_t> m_freeSlotsQueue;
  std::queue<TResource> m_resourceQueue;

  ProduceItemFunc m_produceItem;
  ProduceItemWithoutResourceFunc m_produceItemWithoutResource;
  ConsumeItemFunc m_consumeItem;
  uint64_t m_numConsumeRequests            = 0U;
  uint64_t m_numTimesConsumerGaveUpWaiting = 0U;
};

using SlotProducerConsumerWithoutResources = SlotProducerConsumer<std::nullptr_t>;

template<typename TResource>
class SlotProducerIsDriving
{
public:
  explicit SlotProducerIsDriving(SlotProducerConsumer<TResource>& slotProducerConsumer) noexcept;

  auto ProducerThread() noexcept -> void;

private:
  SlotProducerConsumer<TResource>* m_slotProducerConsumer;
};

using SlotProducerIsDrivingWithoutResources = SlotProducerIsDriving<std::nullptr_t>;

template<typename TResource>
class SlotConsumerIsDriving
{
public:
  explicit SlotConsumerIsDriving(SlotProducerConsumer<TResource>& slotProducerConsumer) noexcept;

  auto ConsumerThread() noexcept -> void;

private:
  SlotProducerConsumer<TResource>* m_slotProducerConsumer;
};

using SlotConsumerIsDrivingWithoutResources = SlotConsumerIsDriving<std::nullptr_t>;

} // namespace GOOM

namespace GOOM
{

template<typename TResource>
SlotProducerConsumer<TResource>::SlotProducerConsumer(GoomLogger& goomLogger,
                                                      const size_t maxInUseSlots,
                                                      const std::string& name) noexcept
  : m_goomLogger{&goomLogger}, m_name{name}, m_maxInUseSlots{maxInUseSlots}
{
  static_assert(std::is_same_v<TResource, std::nullptr_t>);
  Expects(maxInUseSlots > 0);
}

template<typename TResource>
SlotProducerConsumer<TResource>::SlotProducerConsumer(GoomLogger& goomLogger,
                                                      const size_t maxInUseSlots,
                                                      const std::string& name,
                                                      const size_t maxResourceItems) noexcept
  : m_goomLogger{&goomLogger},
    m_name{name},
    m_maxInUseSlots{maxInUseSlots},
    m_maxResourceItems{maxResourceItems}
{
  static_assert(not std::is_same_v<TResource, std::nullptr_t>);
  Expects(maxInUseSlots > 0);
  Expects(maxResourceItems > 0U);
}

template<typename TResource>
auto SlotProducerConsumer<TResource>::Start() noexcept -> void
{
  m_finished = false;

  m_inUseSlotsQueue = std::queue<size_t>{};
  m_freeSlotsQueue  = std::queue<size_t>{};
  if constexpr (not std::is_same_v<TResource, std::nullptr_t>)
  {
    m_resourceQueue = std::queue<TResource>{};
  }

  for (auto slot = 0U; slot < m_maxInUseSlots; ++slot)
  {
    m_freeSlotsQueue.push(slot);
  }

  m_numTimesConsumerGaveUpWaiting = 0U;

  Ensures((m_inUseSlotsQueue.size() + m_freeSlotsQueue.size()) == m_maxInUseSlots);
}

template<typename TResource>
auto SlotProducerConsumer<TResource>::Stop() noexcept -> void
{
  m_finished = true;
  m_producer_cv.notify_all();
  m_consumer_cv.notify_all();
  if constexpr (not std::is_same_v<TResource, std::nullptr_t>)
  {
    m_resourcer_cv.notify_all();
  }
}

template<typename TResource>
inline auto SlotProducerConsumer<TResource>::HasFinished() const noexcept -> bool
{
  return m_finished;
}

template<typename TResource>
inline auto SlotProducerConsumer<TResource>::SetProduceItemFunc(
    const ProduceItemFunc& produceItemFunc) noexcept -> void
{
  static_assert(not std::is_same_v<TResource, std::nullptr_t>);

  Expects(produceItemFunc != nullptr);
  m_produceItem = produceItemFunc;
}

template<typename TResource>
inline auto SlotProducerConsumer<TResource>::SetProduceItemWithoutResourceFunc(
    const ProduceItemWithoutResourceFunc& produceItemWithoutResourceFunc) noexcept -> void
{
  static_assert(std::is_same_v<TResource, std::nullptr_t>);

  Expects(produceItemWithoutResourceFunc != nullptr);
  m_produceItemWithoutResource = produceItemWithoutResourceFunc;
}

template<typename TResource>
inline auto SlotProducerConsumer<TResource>::SetConsumeItemFunc(
    const ConsumeItemFunc& consumeItemFunc) noexcept -> void
{
  Expects(consumeItemFunc != nullptr);
  m_consumeItem = consumeItemFunc;
}

// TODO(glk) - USE MOVE???
template<typename TResource>
auto SlotProducerConsumer<TResource>::AddResource(const TResource& resource) noexcept -> bool
{
  static_assert(not std::is_same_v<TResource, std::nullptr_t>);

  const auto lock = std::lock_guard<std::mutex>{m_mutex};

  if (m_resourceQueue.size() >= m_maxResourceItems)
  {
    return false;
  }

  Expects(m_resourceQueue.size() < m_maxResourceItems);
  m_resourceQueue.push(resource);
  m_producer_cv.notify_all();

  return true;
}

template<typename TResource>
auto SlotProducerConsumer<TResource>::Consume(const uint32_t waitMs) noexcept -> void
{
  if (not ConsumeWithoutRelease(waitMs))
  {
    return;
  }
  ReleaseAfterConsume(m_inUseSlotsQueue.front());
}

template<typename TResource>
inline auto SlotProducerConsumer<TResource>::GetConsumeRequests() const noexcept -> uint64_t
{
  return m_numConsumeRequests;
}

template<typename TResource>
inline auto SlotProducerConsumer<TResource>::GetNumTimesConsumerGaveUpWaiting() const noexcept
    -> uint64_t
{
  return m_numTimesConsumerGaveUpWaiting;
}

template<typename TResource>
auto SlotProducerConsumer<TResource>::ConsumeWithoutRelease(const uint32_t waitMs) noexcept -> bool
{
  auto lock = std::unique_lock<std::mutex>{m_mutex};

  ++m_numConsumeRequests;
#ifdef DEBUG_LOGGING
  LogInfo(*m_goomLogger,
          "### Consumer '{}' consuming item. Consume request {}.",
          m_name,
          m_numConsumeRequests);
#endif

  if (m_inUseSlotsQueue.empty())
  {
#ifdef DEBUG_LOGGING
    LogInfo(*m_goomLogger,
            "*** Consumer '{}' is waiting {}ms for non-empty in-use queue.",
            m_name,
            waitMs);
#endif
    if (not m_consumer_cv.wait_for(lock,
                                   std::chrono::milliseconds{waitMs},
                                   [this]
                                   { return m_finished or (not m_inUseSlotsQueue.empty()); }))
    {
      ++m_numTimesConsumerGaveUpWaiting;
#ifdef DEBUG_LOGGING
      LogInfo(
          *m_goomLogger, "*** Consumer '{}' gave up waiting for non-empty in-use queue.", m_name);
#endif
      return false;
    }
  }
  if (m_finished)
  {
    return false;
  }

  Expects((m_inUseSlotsQueue.size() + m_freeSlotsQueue.size()) == m_maxInUseSlots);
  const auto slot = m_inUseSlotsQueue.front();

  lock.unlock();
  Expects(m_consumeItem != nullptr);
  m_consumeItem(slot);

  return true;
}

template<typename TResource>
auto SlotProducerConsumer<TResource>::ReleaseAfterConsume(const size_t slot) noexcept -> void
{
  const auto lock = std::lock_guard<std::mutex>{m_mutex};

  Expects(m_inUseSlotsQueue.front() == slot);

  m_freeSlotsQueue.push(slot);
  m_inUseSlotsQueue.pop();

  Ensures((m_inUseSlotsQueue.size() + m_freeSlotsQueue.size()) == m_maxInUseSlots);

  m_producer_cv.notify_all();
}

template<typename TResource>
auto SlotProducerConsumer<TResource>::Produce() noexcept -> void
{
  if (not ProduceWithoutRelease())
  {
    return;
  }

  Expects(not m_freeSlotsQueue.empty());
  ReleaseAfterProduce(m_freeSlotsQueue.front());
}

template<typename TResource>
auto SlotProducerConsumer<TResource>::ProduceWithoutRelease() noexcept -> bool
{
  auto lock = std::unique_lock<std::mutex>{m_mutex};

#ifdef DEBUG_LOGGING
  LogInfo(*m_goomLogger, "### Producer '{}' producing item.", m_name);
#endif

  if constexpr (not std::is_same_v<TResource, std::nullptr_t>)
  {
    if (m_resourceQueue.empty())
    {
#ifdef DEBUG_LOGGING
      LogInfo(*m_goomLogger, "### Producer '{}' is waiting for non-empty resource queue.", m_name);
#endif
      m_producer_cv.wait(lock, [this] { return m_finished or (not m_resourceQueue.empty()); });
    }
    if (m_finished)
    {
      return false;
    }
  }
  if (m_inUseSlotsQueue.size() >= m_maxInUseSlots)
  {
#ifdef DEBUG_LOGGING
    LogInfo(*m_goomLogger, "### Producer '{}' is waiting for in-use queue to decrease.", m_name);
#endif
    m_producer_cv.wait(
        lock, [this] { return m_finished or (m_inUseSlotsQueue.size() < m_maxInUseSlots); });
  }
  if (m_finished)
  {
    return false;
  }

  Expects((m_inUseSlotsQueue.size() + m_freeSlotsQueue.size()) == m_maxInUseSlots);
  if constexpr (not std::is_same_v<TResource, std::nullptr_t>)
  {
    Expects(not m_resourceQueue.empty());
  }
  Expects(not m_freeSlotsQueue.empty());
  const auto nextSlot = m_freeSlotsQueue.front();

  lock.unlock();
  if constexpr (not std::is_same_v<TResource, std::nullptr_t>)
  {
    Expects(m_produceItem != nullptr);
    m_produceItem(nextSlot, m_resourceQueue.front());
  }
  else
  {
    Expects(m_produceItemWithoutResource != nullptr);
    m_produceItemWithoutResource(nextSlot);
  }
  Ensures(not m_freeSlotsQueue.empty());

  return true;
}

template<typename TResource>
auto SlotProducerConsumer<TResource>::ReleaseAfterProduce(const size_t slot) noexcept -> void
{
  const auto lock = std::unique_lock<std::mutex>{m_mutex};

#ifdef DEBUG_LOGGING
  LogInfo(*m_goomLogger, "### Producer '{}' releasing slot {}.", m_name, slot);
#endif

  Expects(m_freeSlotsQueue.front() == slot);

  if constexpr (not std::is_same_v<TResource, std::nullptr_t>)
  {
    m_resourceQueue.pop();
#ifdef DEBUG_LOGGING
    LogInfo(*m_goomLogger, "### '{}' resource queue length = {}.", m_name, m_resourceQueue.size());
#endif
  }
  m_inUseSlotsQueue.push(slot);
  m_freeSlotsQueue.pop();

  m_consumer_cv.notify_all();
  if constexpr (not std::is_same_v<TResource, std::nullptr_t>)
  {
    m_resourcer_cv.notify_all();
  }
}

template<typename TResource>
SlotProducerIsDriving<TResource>::SlotProducerIsDriving(
    SlotProducerConsumer<TResource>& slotProducerConsumer) noexcept
  : m_slotProducerConsumer{&slotProducerConsumer}
{
}

template<typename TResource>
auto SlotProducerIsDriving<TResource>::ProducerThread() noexcept -> void
{
  while (not m_slotProducerConsumer->HasFinished())
  {
    m_slotProducerConsumer->Produce();
  }
}

template<typename TResource>
SlotConsumerIsDriving<TResource>::SlotConsumerIsDriving(
    SlotProducerConsumer<TResource>& slotProducerConsumer) noexcept
  : m_slotProducerConsumer{&slotProducerConsumer}
{
}

template<typename TResource>
auto SlotConsumerIsDriving<TResource>::ConsumerThread() noexcept -> void
{
  static constexpr auto CONSUME_WAIT_FOR_MS = 1U;

  while (not m_slotProducerConsumer->HasFinished())
  {
    m_slotProducerConsumer->Consume(CONSUME_WAIT_FOR_MS);
  }
}

} // namespace GOOM
