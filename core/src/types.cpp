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

    bool FixedType::operator== (const FixedType& t) const {

        if(t.m_type != m_type)
            return false;

        if(t.m_size != m_size)
            return false;

        if(t.m_num_components != m_num_components)
            return false;

        if(t.m_little_endian != m_little_endian)
            return false;

        return true;
    }

    unsigned int FixedType::getSize() const {

        return m_size * m_num_components;
    }

} // namespace undicht
