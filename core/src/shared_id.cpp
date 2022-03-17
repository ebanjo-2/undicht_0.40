#include "shared_id.h"

namespace undicht {

    SharedID::SharedID() {
        //ctor
    }

    SharedID::SharedID(unsigned int id) {

        setID(id);
    }

    SharedID::~SharedID() {
        //dtor
    }

    void SharedID::setID(unsigned int id) {

        setRef(id);
    }

    const unsigned int &SharedID::getID() {

        return getRef();
    }

} // namespace undicht
