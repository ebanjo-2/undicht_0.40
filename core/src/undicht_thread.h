#ifndef UNDICHT_THREAD_H
#define UNDICHT_THREAD_H

#include <chrono>
#include <thread>

namespace undicht {

    class Thread {

      private:
        std::thread *m_thread = 0;

      public:
        /** what this class does:
                *
                * runs a function on a new thread (in parallel to your main thread)
                * then calls detach() once the function is finished.
                * the thread then will no longer be joinable(), thats how the class knows it has finished
                * this classes destructor will be waiting until the thread has finished */

        // i dont even know half of what i am doing here

        template <typename Callable, typename... Args>
        Thread(Callable &&c, Args &&...args) {

            m_thread = new std::thread([&] {
                c(args...);
                m_thread->detach();
            });
        }

        virtual ~Thread() {

            while (m_thread->joinable()) {
                // waiting for the thread to finish
                std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(10));
            }

            delete m_thread;
        }

        void join() {
            // waits until the thread has finished executing its function

            if (!m_thread)
                return;

            while (m_thread->joinable()) {
                // waiting for the thread to finish
                std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(10));
            }
        }

        bool hasFinished() {
            if (m_thread)
                return !m_thread->joinable();

            return false;
        }
    };

} // namespace undicht

#endif // UNDICHT_THREAD_H
