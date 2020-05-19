#ifndef BUFFERED_CHANNEL_BUFFERED_CHANNEL_H
#define BUFFERED_CHANNEL_BUFFERED_CHANNEL_H

#include <thread>
#include <future>
#include <queue>

template <typename T>
class concurrent_queue {
private:
    std::queue<T> queue;
    std::mutex mutex;

public:
    bool empty() {
        std::lock_guard<std::mutex> l(mutex);
        return queue.empty();
    }

    size_t size() {
        std::lock_guard<std::mutex> l(mutex);
        return queue.size();
    }

    bool try_pop(T& out) {
        std::lock_guard<std::mutex> l(mutex);

        if (queue.empty()) {
            return false;
        }

        out = std::move(queue.front());

        queue.pop();

        return true;
    }

    void push(T val) {
        std::lock_guard<std::mutex> l(mutex);
        queue.push(std::move(val));
    }
};

template <typename T>
class buffered_channel {
private:
    concurrent_queue<T> queue;
    std::atomic_uint size;
    std::atomic_bool open;

public:
    buffered_channel() {
        open.store(true);
        size.store(10);
    }

    explicit buffered_channel(unsigned int buffer_size) {
        open.store(true);
        size.store(buffer_size);
    }

    ~buffered_channel() {
        close();
    }

    void close() {
        open.store(false);
    }

    void send(T val) {
        if (!open) {
            throw std::runtime_error("send attempt while closed");
        }
        while (queue.size()==size)
        {
            std::this_thread::yield();
        }
        queue.push(std::move(val));
    }

    bool recv(T& val) {
        while (open || !queue.empty()) {
            if (queue.try_pop(val)) {
                return true;
            }
            std::this_thread::yield();
        }
        return false;
    }
};

#endif //BUFFERED_CHANNEL_BUFFERED_CHANNEL_H
