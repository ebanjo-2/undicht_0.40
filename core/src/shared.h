#ifndef SHARED_H
#define SHARED_H

#include "memory_watcher.h"

namespace undicht {

    template <typename ttype>
    class Shared : protected MemoryWatcher {
        /** a class to hold referencing objects such as pointers and ids shared between multiple objects
                * providing you with a way to keep track if there are other objects using the referenced object
                * or if it can be deleted */
      public:
        ttype m_ref = 0;

        /** to be called when an object referencing the id is destroyed
                * @return true when the object being destroyed is the last one referencing the id */
        bool removeUser() {
            return !deleteUser();
        }

        void setRef(ttype ref) {
            /// removeUser should be called separately to check if the referenced object should be deleted
            createNewUserCount();
            m_ref = ref;
        }

        const ttype &getRef() const {
            return m_ref;
        }

        void operator=(ttype ref) {
            if (ref) // could create a memory leak were there would be created more and more user count
                setRef(ref);
        }

        operator const ttype() const {
            return m_ref;
        }

        Shared() {
        }

        Shared(ttype ref) {
            setRef(ref);
        }

        virtual ~Shared() {
        }
    };

} // namespace undicht

#endif // SHARED_H