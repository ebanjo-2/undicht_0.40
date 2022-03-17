#ifndef UNIQUE_OBJECT_H
#define UNIQUE_OBJECT_H

namespace undicht {

    namespace core {

        class UniqueObject {
            // classes that inherit from this object cant be copied

          private:
            // copy constructor and operator are private

            UniqueObject(const UniqueObject &u){};

            void operator=(const UniqueObject &u){};

          public:

            UniqueObject() = default;
            ~UniqueObject() = default;
        };

    } // namespace core

} // namespace undicht

#endif // UNIQUE_OBJECT_H
