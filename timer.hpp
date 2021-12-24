/**
 * @file epoll_timer.hpp
 * @author xueweiwujxw (xueweiwujxw@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2021-12-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once
#include <functional>
#include <algorithm>
#include <queue>
#include <vector>
#include <iostream>
#include <atomic>
#include <sys/epoll.h>
#include <future>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

#define _MAJOR_VERSION_ 0
#define _MINOR_VERSION_ 0
#define _REVISION_VERSION_ 0
#define _STATUS_VERSION_ "Alpha"
#define AUX_STR_EXP(__A) #__A
#define AUX_STR(__A) AUX_STR_EXP(__A)

#ifndef EPOLLTIMER_VERSION
#define EPOLLTIMER_VERSION AUX_STR(_MAJOR_VERSION_) "." AUX_STR(_MINOR_VERSION_) "." AUX_STR(_REVISION_VERSION_) " " _STATUS_VERSION_
#endif

namespace Timer
{
    class timer
    {
    private:
        std::function<void(void *)> fun;
        unsigned long long expire_time;
        void *args;

    public:
        timer(unsigned long long expire, std::function<void(void *)> fun, void *args) : expire_time(expire), fun(fun), args(args) {}
        inline void active() {
            this->fun(this->args);
        }
        inline unsigned long long getExpire() const { return this->expire_time; }
        ~timer() {}
    };

    class timerManager
    {
    private:
        struct cmp
        {
            bool operator()(timer *&a, timer *&b) const {
                return a->getExpire() > b->getExpire();
            }
        };
        std::priority_queue<timer *, std::vector<timer *>, cmp> timerQueue;
        std::atomic<bool> is_running;
        std::future<void> m_future;
        int pipefd[2];
        int readfd;
        int writefd;
        char buf[2];

        /**
         * @brief 定时器监听循环
         * 
         */
        void dispatch() {
            struct epoll_event ev, events[10];
            int epollfd = epoll_create(10);
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = this->readfd;
            std::cout << epoll_ctl(epollfd, EPOLL_CTL_ADD, this->readfd, &ev) << std::endl;
            while (this->is_running) {
                epoll_wait(epollfd, events, 10, this->getRecentTimeout());
                this->activeTimeout();
            }
        }

        /**
         * @brief 获取最小的超时时间
         * 
         * @return unsigned long long 
         */
        unsigned long long getRecentTimeout() {
            unsigned long long timeout = -1;
            if (this->timerQueue.empty())
                return timeout;
            unsigned long long now = this->getCurrentSecs();
            timeout = this->timerQueue.top()->getExpire() - now;
            if (timeout < 0)
                timeout = 0;
            return timeout;
        }

        /**
         * @brief 激活超时的回调
         * 
         */
        void activeTimeout() {
            unsigned long long now = this->getCurrentSecs();
            while (!this->timerQueue.empty()) {
                timer *t = this->timerQueue.top();
                if (t->getExpire() <= now) {
                    t->active();
                    // printf("%p\n\n", t);
                    delete t;
                    this->timerQueue.pop();
                    continue;
                }
                return;
            }
        }

    public:
        timerManager() {
            if (pipe(this->pipefd) < 0) {
                std::cerr << "init pipe failed" << std::endl;
                this->~timerManager();
            }
            this->readfd = this->pipefd[0];
            this->writefd = this->pipefd[1];
        }
        ~timerManager() {}

        /**
         * @brief 获取当前时间，unix时间戳，秒计数
         * 
         * @return unsigned long long 
         */
        unsigned long long getCurrentSecs() {
            time_t t = time(&t);
            return t;
        }

        /**
         * @brief 创建定时器
         * 
         * @param expire_time 超时时间（unix 时间戳）
         * @param fun 回调函数
         * @param args 其他参数
         * @return timer* 
         */
        timer *addTimer(int expire_time, std::function<void(void *)> fun, void *args = nullptr) {
            if (expire_time < this->getCurrentSecs())
                return nullptr;
            timer *t = new timer(expire_time, fun, args);
            // printf("%p\n\n", t);
            if (this->timerQueue.empty()) {
                write(this->writefd, buf, 2);
            }
            this->timerQueue.push(t);
            return t;
        }

        /**
         * @brief 删除一个定时器
         * 
         * @param t 
         */
        void delTimer(timer *t) {
            std::priority_queue<timer *, std::vector<timer *>, cmp> newqueue;
            while (!this->timerQueue.empty()) {
                timer *top = this->timerQueue.top();
                this->timerQueue.pop();
                if (top != t)
                    newqueue.push(top);
            }
            this->timerQueue = newqueue;
        }

        /**
         * @brief 开启定时
         * 
         */
        void loopstart() {
            this->is_running = true;
            this->m_future = std::async(&timerManager::dispatch, this);
        }

        /**
         * @brief 结束定时
         * 
         */
        void loopbreak() {
            this->is_running = false;
            if (this->timerQueue.empty()) {
                write(this->writefd, buf, 2);
            }
            this->m_future.wait();
            while (!this->timerQueue.empty()) {
                timer *t = this->timerQueue.top();
                delete t;
                this->timerQueue.pop();
            }
        }
    };

} // namespace Timer
