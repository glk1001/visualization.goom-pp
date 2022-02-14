#pragma once

#include "thread_pool.h"

#include <algorithm>
#include <cstdint>
#include <future>
#include <thread>
#include <vector>

namespace GOOM::UTILS
{

class Parallel
{
public:
  // numPoolThreads > 0:  use this number of threads in the pool
  // numPoolThreads <= 0: use (max cores - numPoolThreads) in the pool
  explicit Parallel(int32_t numPoolThreads = 0) noexcept;

  auto GetNumThreadsUsed() const -> size_t;
  auto GetThreadIds() const -> std::vector<std::thread::id>;

  template<typename Callable>
  void ForLoop(size_t numIters, Callable loopFunc);

private:
  ThreadPool m_threadPool;
  bool m_forLoopInUse = false;
};

inline Parallel::Parallel(const int32_t numPoolThreads) noexcept
  : m_threadPool{
        (numPoolThreads <= 0)
            ? static_cast<size_t>(std::max(
                  1, static_cast<int32_t>(std::thread::hardware_concurrency()) + numPoolThreads))
            : static_cast<size_t>(std::min(
                  numPoolThreads, static_cast<int32_t>(std::thread::hardware_concurrency())))}
{
}

inline auto Parallel::GetNumThreadsUsed() const -> size_t
{
  return m_threadPool.GetNumWorkers();
}

inline auto Parallel::GetThreadIds() const -> std::vector<std::thread::id>
{
  return m_threadPool.GetThreadIds();
}

template<typename Callable>
void Parallel::ForLoop(const size_t numIters, const Callable loopFunc)
{
  if (0 == numIters)
  {
    throw std::logic_error("ForLoop: numIters == 0.");
  }
  if (m_forLoopInUse)
  {
    throw std::logic_error("ForLoop: already in use.");
  }
  m_forLoopInUse = true;

  const size_t numThreads = std::min(numIters, m_threadPool.GetNumWorkers());

  if (1 == numThreads)
  {
    for (size_t i = 0; i < numIters; ++i)
    {
      loopFunc(i);
    }
    m_forLoopInUse = false;
    return;
  }

  const size_t chunkSize = numIters / numThreads; // >= 1
  const size_t numLeftoverIters = numIters - (numThreads * chunkSize);

  const auto loopContents =
      [&chunkSize, &numThreads, &numLeftoverIters, &loopFunc](const uint32_t threadIndex)
  {
    const size_t inclusiveStartIndex = threadIndex * chunkSize;
    const size_t exclusiveEndIndex =
        inclusiveStartIndex + chunkSize + (threadIndex < (numThreads - 1) ? 0 : numLeftoverIters);

    for (size_t k = inclusiveStartIndex; k < exclusiveEndIndex; ++k)
    {
      loopFunc(k);
    }
  };

  std::vector<std::future<void>> futures{};
  for (uint32_t j = 0; j < numThreads; ++j)
  {
    futures.emplace_back(m_threadPool.ScheduleAndGetFuture(loopContents, j));
  }

  for (const auto& future : futures)
  {
    future.wait();
  }

  m_forLoopInUse = false;
}

} // namespace GOOM::UTILS
