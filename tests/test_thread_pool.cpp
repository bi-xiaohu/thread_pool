#include "thread_pool.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

static int g_passed = 0;
static int g_failed = 0;

#define TEST(name)                                                        \
    do {                                                                  \
        std::cout << "  [RUN ] " << #name << "... ";                     \
        try {                                                             \
            test_##name();                                                \
            std::cout << "PASSED\n";                                      \
            ++g_passed;                                                   \
        } catch (const std::exception& e) {                              \
            std::cout << "FAILED: " << e.what() << "\n";                 \
            ++g_failed;                                                   \
        }                                                                 \
    } while (0)

#define ASSERT_EQ(expected, actual)                                       \
    do {                                                                  \
        auto e_ = (expected);                                             \
        auto a_ = (actual);                                               \
        if (e_ != a_) {                                                   \
            throw std::runtime_error(                                     \
                std::string("ASSERT_EQ failed: expected=") +             \
                std::to_string(e_) + " actual=" + std::to_string(a_) +   \
                " at line " + std::to_string(__LINE__));                  \
        }                                                                 \
    } while (0)

#define ASSERT_TRUE(cond)                                                 \
    do {                                                                  \
        if (!(cond)) {                                                    \
            throw std::runtime_error(                                     \
                std::string("ASSERT_TRUE failed: ") + #cond +            \
                " at line " + std::to_string(__LINE__));                  \
        }                                                                 \
    } while (0)

// ============================================================
// 测试用例
// ============================================================

// 测试 1: 基本构造和析构
void test_construct_and_destruct() {
    ThreadPool pool(4);
    ASSERT_EQ(4u, pool.Size());
}  // 析构不应崩溃

// 测试 2: 默认线程数
void test_default_thread_count() {
    ThreadPool pool;
    ASSERT_TRUE(pool.Size() >= 1);
}

// 测试 3: 线程数为 0 时至少创建 1 个
void test_zero_threads() {
    ThreadPool pool(0);
    ASSERT_EQ(1u, pool.Size());
}

// 测试 4: 提交单个任务并获取结果
void test_single_task() {
    ThreadPool pool(2);
    auto future = pool.Submit([](int a, int b) { return a + b; }, 10, 20);
    ASSERT_EQ(30, future.get());
}

// 测试 5: 提交多个任务
void test_multiple_tasks() {
    ThreadPool pool(4);
    constexpr int kCount = 100;
    std::vector<std::future<int>> futures;
    futures.reserve(kCount);

    for (int i = 0; i < kCount; ++i) {
        futures.push_back(pool.Submit([](int n) { return n * n; }, i));
    }

    for (int i = 0; i < kCount; ++i) {
        ASSERT_EQ(i * i, futures[i].get());
    }
}

// 测试 6: 无返回值任务
void test_void_task() {
    ThreadPool pool(2);
    std::atomic<int> counter{0};

    std::vector<std::future<void>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(pool.Submit([&counter] {
            counter.fetch_add(1, std::memory_order_relaxed);
        }));
    }

    for (auto& f : futures) {
        f.get();
    }
    ASSERT_EQ(10, counter.load());
}

// 测试 7: 任务中的异常能通过 future 传播
void test_exception_propagation() {
    ThreadPool pool(2);
    auto future = pool.Submit([] {
        throw std::runtime_error("task error");
        return 42;
    });

    bool caught = false;
    try {
        future.get();
    } catch (const std::runtime_error& e) {
        caught = (std::string(e.what()) == "task error");
    }
    ASSERT_TRUE(caught);
}

// 测试 8: 大量任务的并发安全性
void test_stress() {
    ThreadPool pool(8);
    constexpr int kCount = 10000;
    std::atomic<int> counter{0};

    std::vector<std::future<void>> futures;
    futures.reserve(kCount);

    for (int i = 0; i < kCount; ++i) {
        futures.push_back(pool.Submit([&counter] {
            counter.fetch_add(1, std::memory_order_relaxed);
        }));
    }

    for (auto& f : futures) {
        f.get();
    }
    ASSERT_EQ(kCount, counter.load());
}

// 测试 9: 返回 string 类型
void test_return_string() {
    ThreadPool pool(2);
    auto future = pool.Submit([](const std::string& name) {
        return "Hello, " + name + "!";
    }, std::string("ThreadPool"));

    auto result = future.get();
    ASSERT_TRUE(result == "Hello, ThreadPool!");
}

// 测试 10: 单线程池也能正常工作
void test_single_thread() {
    ThreadPool pool(1);
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 20; ++i) {
        futures.push_back(pool.Submit([](int n) { return n + 1; }, i));
    }
    for (int i = 0; i < 20; ++i) {
        ASSERT_EQ(i + 1, futures[i].get());
    }
}

int main() {
    std::cout << "=== ThreadPool 测试 ===\n\n";

    TEST(construct_and_destruct);
    TEST(default_thread_count);
    TEST(zero_threads);
    TEST(single_task);
    TEST(multiple_tasks);
    TEST(void_task);
    TEST(exception_propagation);
    TEST(stress);
    TEST(return_string);
    TEST(single_thread);

    std::cout << "\n=== 结果: " << g_passed << " passed, "
              << g_failed << " failed ===\n";

    return g_failed > 0 ? 1 : 0;
}