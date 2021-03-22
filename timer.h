//
// logger.h
// HighPerformanceLogger
//
// Created by _ChingC on 2021/03/22.
// GitHub: https://github.com/ChingCdesu
// Copyright © 2021 _ChingC. All right reserved.
//

#ifndef TIMER_H
#define TIMER_H

#include <functional>
#include <chrono>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>

class timer
{
public:
    timer() : expired_(true), try_to_expire_(false)
    {
    }

    timer(const timer &t)
    {

        expired_ = t.expired_.load();

        try_to_expire_ = t.try_to_expire_.load();
    }

    ~timer()
    {

        Expire();

        //      std::cout << "timer destructed!" << std::endl;
    }

    void StartTimer(int interval, std::function<void()> task)
    {

        if (!expired_)
        {

            //          std::cout << "timer is currently running, please expire it first..." << std::endl;

            return;
        }

        expired_ = false;

        std::thread([this, interval, task]() {
            while (!try_to_expire_)
            {

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));

                task();
            }

            //          std::cout << "stop task..." << std::endl;

            {

                std::lock_guard<std::mutex> locker(mutex_);

                expired_ = true;

                expired_cond_.notify_one();
            }
        }).detach();
    }

    void Expire()
    {

        if (expired_)
        {

            return;
        }

        if (try_to_expire_)
        {

            //std::cout << "timer is trying to expire, please wait..." << std::endl;

            return;
        }

        try_to_expire_ = true;

        {

            std::unique_lock<std::mutex> locker(mutex_);

            expired_cond_.wait(locker, [this] { return expired_ == true; });

            if (expired_)
            {

                //std::cout << "timer expired!" << std::endl;

                try_to_expire_ = false;
            }
        }
    }

private:
    std::atomic<bool> expired_;

    std::atomic<bool> try_to_expire_;

    std::mutex mutex_;

    std::condition_variable expired_cond_;
};

#endif