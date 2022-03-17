#include "memory_watcher.h"
#include "debug.h"
#include <iostream>

namespace undicht {

    /////////////////////////////////////////// constructor / destructor /////////////////////////////////////////

    MemoryWatcher::MemoryWatcher() {
        //ctor
    }

    MemoryWatcher::~MemoryWatcher() {
        //dtor
    }

    ///////////////////////////////////// automating the user count on copy ////////////////////////////////////////

    void MemoryWatcher::operator=(const MemoryWatcher &c) {

        if (m_user_count) {
            // this object now no longer uses the memory

            *m_user_count = std::max(0, *m_user_count - 1);

            if (!(*m_user_count))
                delete m_user_count;
        }

        // copying the user count address + adding one user
        m_user_count = c.m_user_count;

        if (m_user_count)
            *m_user_count += 1;
    }

    MemoryWatcher::MemoryWatcher(const MemoryWatcher &c) {

        // calling the = operator to do what needs to be done
        *this = c;
    }

    /////////////////////////////////////// using the user count ///////////////////////////////////////////////////

    void MemoryWatcher::createNewUserCount() {
        /** reserves a new user count & updates m_id
            * should be called when the data to be watched over is initialized or changed
            * when there was old data stored in the object it should also be checked if this data
            * has any users left */

        if (m_user_count) {
            // this object already was watching over some memory
            *m_user_count = std::max(0, *m_user_count - 1);

            if (!(*m_user_count))
                delete m_user_count;
        }

        m_user_count = new int;
        *m_user_count = 1;
    }

    int MemoryWatcher::deleteUser() {
        /** deletes the user & returns the number of users left over
            * if no user (return: 0) is left, the data should be deleted */

        if (!m_user_count) {
            // the data was not initialized
            // so it should not get deleted
            return 1;
        }

        *m_user_count = std::max(0, *m_user_count - 1);

        if (!(*m_user_count)) {

            delete m_user_count;
            return 0;
        }

        return *m_user_count;
    }

} // undicht