#include <iostream>
#include <map>
#include <thread>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <queue>
#include <vector>
#include <condition_variable>

std::mutex print_mutex;

void secure_print(std::string msg)
{
    std::lock_guard<std::mutex> print_lock(print_mutex);
    std::cout << "[ "<< std::this_thread::get_id() << " ]"<< msg << std::endl;
}

template<typename T>
class MQueue
{
public:
    template<typename cont>
    explicit MQueue(const cont& c)
    {
        for(auto e: c)
            _collection.push(e);
    }

    T dequeue()
    {
        std::lock_guard<std::mutex> lock{_mutex};
        auto c = _collection.front();
        _collection.pop();

        return c;
    }

    bool empty()
    {
        return _collection.empty();
    }

private:
    std::queue<T> _collection;
    std::mutex _mutex;
};


int main()
{
    std::vector<std::thread> pool;

    int x{7}, y{11}, z{3};
    std::mutex x_mutex, y_mutex, z_mutex, g_mutex;

    std::condition_variable pump_condition;

    std::vector<int> cars{2, 8, 4, 3, 2};
    std::vector<int> wait_times(cars.size());
    MQueue<int> q{cars};

    std::atomic<int> wait_times_counter{0};

    for(auto i(0); i < 3; ++i)
    {
        auto begin = std::chrono::steady_clock::now();

        pool.emplace_back([&]() {
            while(!q.empty()) {

                auto ltr_reqd = q.dequeue();
                std::cout << ltr_reqd<< std::endl;

                auto idx = wait_times_counter++;

                while (true) {
                    if (x_mutex.try_lock()) {
                        if (x >= ltr_reqd) {

                            auto now = std::chrono::steady_clock::now();
                            wait_times[idx] =  std::chrono::duration_cast<std::chrono::seconds>(now - begin).count();

                            secure_print(std::to_string(ltr_reqd) + " serviced by x");
                            std::this_thread::sleep_for(std::chrono::seconds(ltr_reqd));

                            x -= ltr_reqd;

                            x_mutex.unlock();
                            pump_condition.notify_all();
                            break;
                        }
                        x_mutex.unlock();
                        pump_condition.notify_all();
                    }

                    if (y_mutex.try_lock()) {

                        if (y >= ltr_reqd) {

                            auto now = std::chrono::steady_clock::now();
                            wait_times[idx] = std::chrono::duration_cast<std::chrono::seconds>(now - begin).count();

                            secure_print(std::to_string(ltr_reqd) + " serviced by y");
                            std::this_thread::sleep_for(std::chrono::seconds(ltr_reqd));

                            y -= ltr_reqd;

                            y_mutex.unlock();
                            pump_condition.notify_all();
                            break;
                        }
                        y_mutex.unlock();
                        pump_condition.notify_all();
                    }

                    if (z_mutex.try_lock()) {
                        if (z >= ltr_reqd) {

                            auto now = std::chrono::steady_clock::now();
                            wait_times[idx] = std::chrono::duration_cast<std::chrono::seconds>(now - begin).count();

                            secure_print(std::to_string(ltr_reqd) + " serviced by z");
                            std::this_thread::sleep_for(std::chrono::seconds(ltr_reqd));

                            z -= ltr_reqd;

                            z_mutex.unlock();
                            pump_condition.notify_all();
                            break;
                        }
                        z_mutex.unlock();
                        pump_condition.notify_all();
                    }

                    std::unique_lock<std::mutex> g_lock(g_mutex);
                    secure_print("wait for pumps to be free");

                    pump_condition.wait(g_lock);
                }
            }
        });
    }

    for(auto& t : pool)
        t.join();


    for(auto& wtime : wait_times)
        std::cout << wtime << std::endl;

    return 0;
}

