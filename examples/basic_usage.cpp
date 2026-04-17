#include "thread_pool.h"

#include <chrono>
#include <iostream>
#include <numeric>
#include <vector>

int main() {
    std::cout << "=== ThreadPool 基本用法示例 ===\n\n";

    // 创建一个 4 线程的线程池
    ThreadPool pool(4);
    std::cout << "线程池已创建，工作线程数: " << pool.Size() << "\n\n";

    // ---- 示例 1: 提交 lambda，获取返回值 ----
    std::cout << "--- 示例 1: 提交单个任务 ---\n";
    auto future1 = pool.Submit([](int a, int b) {
        return a + b;
    }, 3, 4);
    std::cout << "3 + 4 = " << future1.get() << "\n\n";

    // ---- 示例 2: 提交多个任务并发执行 ----
    std::cout << "--- 示例 2: 并发计算 ---\n";
    constexpr int kTaskCount = 8;
    std::vector<std::future<int>> futures;
    futures.reserve(kTaskCount);

    for (int i = 0; i < kTaskCount; ++i) {
        futures.push_back(pool.Submit([](int n) {
            // 模拟耗时计算
            int sum = 0;
            for (int j = 1; j <= n; ++j) sum += j;
            return sum;
        }, (i + 1) * 100));
    }

    for (int i = 0; i < kTaskCount; ++i) {
        std::cout << "  sum(1.." << (i + 1) * 100 << ") = " << futures[i].get() << "\n";
    }
    std::cout << "\n";

    // ---- 示例 3: 提交无返回值的任务 ----
    std::cout << "--- 示例 3: 无返回值任务 ---\n";
    auto future3 = pool.Submit([] {
        std::cout << "  Hello from thread "
                  << std::this_thread::get_id() << "!\n";
    });
    future3.get();  // 等待完成
    std::cout << "\n";

    // ---- 示例 4: 提交普通函数 ----
    std::cout << "--- 示例 4: 提交普通函数 ---\n";
    auto square = [](double x) -> double { return x * x; };
    auto future4 = pool.Submit(square, 3.14);
    std::cout << "  3.14^2 = " << future4.get() << "\n\n";

    std::cout << "=== 所有示例完成，线程池将在析构时优雅关闭 ===\n";
    return 0;
}  // pool 析构 → 等待所有工作线程结束
