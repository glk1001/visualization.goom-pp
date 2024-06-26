module;

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

// clang-format off
// NOLINTBEGIN: Not my code

export module Goom.Utils.Parallel:ThreadPool;

import Goom.Lib.AssertUtils;

export namespace GOOM::UTILS
{

#define INVOKE_MACRO(CALLABLE, ARGS_TYPE, ARGS) \
  std::invoke(CALLABLE, std::forward<ARGS_TYPE>(ARGS)...)

class ThreadPool
{
public:
  explicit ThreadPool(size_t numWorkers) noexcept;
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) noexcept = delete;
  // The destructor blocks until all outstanding work is complete.
  ~ThreadPool() noexcept;
  auto operator=(const ThreadPool&) -> ThreadPool& = delete;
  auto operator=(ThreadPool&&) noexcept -> ThreadPool& = delete;

  //   Given a callable 'func' that returns a value of type 'RetT', this
  //   function returns a std::future<RetT> that can be used to access
  //   func's results.
  template<typename FuncT, typename... ArgsT>
  auto ScheduleAndGetFuture(FuncT&& func, ArgsT&&... args)
      -> std::future<decltype(INVOKE_MACRO(func, ArgsT, args))>;

  // Wait for all outstanding work to be completed.
  void Wait();

  auto GetNumWorkers() const -> size_t;
  auto GetThreadIds() const -> std::vector<std::thread::id>;

  auto GetOutstandingWorkSize() const -> size_t;
  void SetWorkDoneCallback(std::function<void(int)> func);

private:
  size_t m_numWorkers = 0;

  // The destructor sets 'finished' to true and then notifies all workers.
  // 'finished' causes each thread to break out of their work loop.
  bool m_finished = false;

  // Work queue. Guarded by 'm_mutex'.
  mutable std::mutex m_mutex{};
  struct WorkItem
  {
    std::function<void(void)> func{};
  };
  std::queue<WorkItem> m_workQueue{};

  std::vector<std::thread> m_workers{};
  std::condition_variable m_newWorkCondition{};
  std::condition_variable m_workDoneCondition{};

  // Whenever a work item is complete, we call this callback. If this is empty,
  // nothing is done.
  std::function<void(int)> m_workDoneCallback{};

  void ThreadLoop();
};

} // namespace GOOM::UTILS

//module : private;

namespace GOOM::UTILS
{

namespace THREAD_POOL_IMPL
{

// This helper class simply returns a std::function that executes:
//
//   ReturnT x = func();
//   promise->set_value(x);
//
// However, this is tricky in the case where T == void. The code above won't
// compile if ReturnT == void, and neither will
//
//   promise->set_value(func());
//
// To workaround this, we use a template specialization for the case where ReturnT
// is void. If the "regular void" proposal is accepted, this could be simpler:
//   http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0146r1.html.

// The following non-specialized 'FuncWrapper' handles callables that return a
// non-void value.
template<typename ReturnT>
struct FuncWrapper
{
  template<typename FuncT, typename... ArgsT>
  auto GetWrapped(FuncT&& func, std::shared_ptr<std::promise<ReturnT>> promise, ArgsT&&... args)
      -> std::function<void()>
  {
    // Capturing by value is inefficient. It would be more efficient to move-capture
    // everything - can a generalized lambda capture help??
    return
        [promise, func, args...]() mutable { promise->set_value(INVOKE_MACRO(func, ArgsT, args)); };
  }
};

template<typename FuncT, typename... ArgsT>
void InvokeVoidRet(FuncT&& func,
                   const std::shared_ptr<std::promise<void>>& promise,
                   ArgsT&&... args)
{
  INVOKE_MACRO(func, ArgsT, args);
  promise->set_value();
}

// Specialization to handle callables that return void.
template<>
struct FuncWrapper<void>
{
  template<typename FuncT, typename... ArgsT>
  auto GetWrapped(FuncT&& func, const std::shared_ptr<std::promise<void>>& promise, ArgsT&&... args)
      -> std::function<void()>
  {
    return [promise, func, args...]() mutable {
      INVOKE_MACRO(func, ArgsT, args);
      promise->set_value();
    };
  }
};

} // namespace THREAD_POOL_IMPL

template<typename FuncT, typename... ArgsT>
auto ThreadPool::ScheduleAndGetFuture(FuncT&& func, ArgsT&&... args)
    -> std::future<decltype(INVOKE_MACRO(func, ArgsT, args))>
{
  using ReturnT = decltype(INVOKE_MACRO(func, ArgsT, args));

  // We are only allocating this std::promise in a shared_ptr because
  // std::promise is non-copyable.
  std::shared_ptr<std::promise<ReturnT>> promise = std::make_shared<std::promise<ReturnT>>();
  std::future<ReturnT> retFuture = promise->get_future();

  THREAD_POOL_IMPL::FuncWrapper<ReturnT> funcWrapper{};
  std::function<void()> wrappedFunc = funcWrapper.GetWrapped(
      std::forward<FuncT>(func), std::move(promise), std::forward<ArgsT>(args)...);

  // Acquire the lock, and then push the WorkItem onto the queue.
  {
    const std::scoped_lock<std::mutex> lock{m_mutex};
    WorkItem work{};
    work.func = std::move(wrappedFunc);
    m_workQueue.emplace(std::move(work));
  }
  m_newWorkCondition.notify_one(); // Tell one worker we are ready.

  return retFuture;
}

ThreadPool::ThreadPool(const size_t numWorkers) noexcept : m_numWorkers(numWorkers)
{
  Expects(numWorkers > 0);

  m_workers.reserve(m_numWorkers);
  for (size_t i = 0; i < m_numWorkers; ++i)
  {
    m_workers.emplace_back(&ThreadPool::ThreadLoop, this);
  }
}

ThreadPool::~ThreadPool() noexcept
{
  {
    const std::scoped_lock<std::mutex> lock{m_mutex};
    m_finished = true;
  }
  // Tell all the workers we're ready.
  m_newWorkCondition.notify_all();

  for (std::thread& thread : m_workers)
  {
    thread.join();
  }
}

auto ThreadPool::GetThreadIds() const -> std::vector<std::thread::id>
{
  std::vector<std::thread::id> threadIds{};
  for (const std::thread& thread : m_workers)
  {
    threadIds.emplace_back(thread.get_id());
  }
  return threadIds;
}

auto ThreadPool::GetOutstandingWorkSize() const -> size_t
{
  const std::scoped_lock<std::mutex> lock{m_mutex};
  return m_workQueue.size();
}

auto ThreadPool::GetNumWorkers() const -> size_t
{
  return m_numWorkers;
}

void ThreadPool::SetWorkDoneCallback(std::function<void(int)> func)
{
  m_workDoneCallback = std::move(func);
}

void ThreadPool::Wait()
{
  std::unique_lock<std::mutex> lock{m_mutex};
  if (!m_workQueue.empty())
  {
    m_workDoneCondition.wait(lock, [this] { return m_workQueue.empty(); });
  }
}

void ThreadPool::ThreadLoop()
{
  // Wait until the ThreadPool sends us work.
  while (true)
  {
    WorkItem workItem{};

    int32_t prevWorkSize = -1;
    {
      std::unique_lock<std::mutex> lock{m_mutex};
      m_newWorkCondition.wait(lock, [this] { return m_finished || (!m_workQueue.empty()); });

      // If all the work is done and exit_ is true, then break out of the loop.
      if (m_finished && m_workQueue.empty())
      {
        break;
      }

      // Pop the work off of the queue - we are careful to execute the
      // work_item.func callback only after we have released the lock.
      prevWorkSize = static_cast<int32_t>(m_workQueue.size());
      workItem = std::move(m_workQueue.front());
      m_workQueue.pop();
    }

    // We are careful to do the work without the lock held!
    workItem.func(); // Do work.

    if (m_workDoneCallback)
    {
      m_workDoneCallback(prevWorkSize - 1);
    }

    // Notify if all work is done.
    {
      const std::unique_lock<std::mutex> lock{m_mutex};
      if (m_workQueue.empty() && (1 == prevWorkSize))
      {
        m_workDoneCondition.notify_all();
      }
    }
  }
}

} // namespace GOOM::UTILS

// NOLINTEND
// clang-format on
