#include "types.h"

namespace undicht {

    //////////////////////////////////////// Fixed Type //////////////////////////////////////////////

    FixedType::FixedType(Type t, unsigned int size, unsigned int num_comp, bool little_endian) {

        m_type = t;
        m_size = size;
        m_num_components = num_comp;
        m_little_endian = little_endian;
    }

    FixedType::operator Type() {
        // allows for tests like this: t == Type::INT

        return m_type;
    }

    unsigned int FixedType::getSize() {

        return m_size * m_num_components;
    }

} // namespace undicht
