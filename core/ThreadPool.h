#pragma once
#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

class ThreadPool
{
public:
    static std::size_t DefaultWorkerCount()
    {
        const unsigned int hw = std::thread::hardware_concurrency();
        return hw > 1 ? hw - 1 : 0;
    }

    explicit ThreadPool(std::size_t workerCount = DefaultWorkerCount())
    {
        m_workers.reserve(workerCount);
        for (std::size_t i = 0; i < workerCount; ++i)
            m_workers.emplace_back([this] { WorkerLoop(); });
    }

    ~ThreadPool()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_shutdown = true;
        }
        m_condition.notify_all();

        for (auto& worker : m_workers)
            worker.join();
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    std::size_t GetWorkerCount() const { return m_workers.size(); }

    std::size_t GetChunkCount() const { return m_workers.size() + 1; }

    template <class F, class... Args>
    auto AddTask(F&& f, Args&&... args) -> std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
    {
        using Result = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

        auto task = std::make_shared<std::packaged_task<Result()>>(
            [fn = std::forward<F>(f), ... captured = std::forward<Args>(args)]() mutable -> Result
            { return std::invoke(std::move(fn), std::move(captured)...); });

        std::future<Result> future = task->get_future();

        if (m_workers.empty())
        {
            (*task)(); 
            return future;
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_tasks.emplace([task] { (*task)(); });
        }
        m_condition.notify_one();
        return future;
    }

    template <class F>
    void ParallelFor(std::size_t count, F&& fn)
    {
        if (count == 0)
            return;

        const std::size_t chunks = std::min(GetChunkCount(), count);
        const std::size_t chunkSize = (count + chunks - 1) / chunks;

        std::vector<std::future<void>> futures;
        futures.reserve(chunks - 1);

        for (std::size_t i = 1; i < chunks; ++i)
        {
            const std::size_t begin = i * chunkSize;
            if (begin >= count)
                break;
            const std::size_t end = std::min(begin + chunkSize, count);
            futures.push_back(AddTask([&fn, begin, end, i] { fn(begin, end, i); }));
        }

        std::exception_ptr error;
        try
        {
            fn(std::size_t{ 0 }, std::min(chunkSize, count), std::size_t{ 0 }); 
        }
        catch (...)
        {
            error = std::current_exception();
        }

        for (auto& future : futures)
        {
            try
            {
                future.get();
            }
            catch (...)
            {
                if (!error)
                    error = std::current_exception();
            }
        }

        if (error)
            std::rethrow_exception(error);
    }

private:
    void WorkerLoop()
    {
        for (;;)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_condition.wait(lock, [this] { return m_shutdown || !m_tasks.empty(); });
                if (m_shutdown && m_tasks.empty())
                    return; 
                task = std::move(m_tasks.front());
                m_tasks.pop();
            }
            task();
        }
    }

    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    bool m_shutdown = false; 
};
