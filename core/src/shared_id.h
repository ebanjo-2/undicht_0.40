#ifndef SHARED_ID_H
#define SHARED_ID_H

#include "shared.h"

namespace undicht {

    class SharedID : public Shared<unsigned int> {
        /** a class that stores the id to an object which may be referenced in multiple objects
            * and needs to be deleted when the last referencing object is destroyed */
      public:
        void setID(unsigned int id);

        const unsigned int &getID();

        SharedID();
        SharedID(unsigned int id);
        virtual ~SharedID();
    };

} // namespace undicht

#endif // SHARED_ID_H
