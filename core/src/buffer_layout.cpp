
#include "buffer_layout.h"

namespace undicht {

    BufferLayout::BufferLayout(const std::vector<FixedType> &types) {

        m_types = types;
    }

    unsigned int BufferLayout::getTotalSize() {
        /** @return The size of the data struct described by this
             * BufferLayout
             */

        unsigned int size = 0;

        for (FixedType &t : m_types)
            size += t.getSize();

        return size;
    }

} // namespace undicht