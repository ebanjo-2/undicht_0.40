#include "debug.h"
#include "buffer_layout.h"

namespace undicht {

    BufferLayout::BufferLayout(const std::vector<FixedType> &types) {

        m_types = types;
    }

    void BufferLayout::setType(uint32_t index, const FixedType& type) {

        // making sure the type can be stored
        if(m_types.size() <= index) {
            // adding new attributes
            m_types.resize(index + 1);
        }

        // storing the attribute
        m_types.at(index) = type;

    }

    const FixedType& BufferLayout::getType(uint32_t index) const {

        return m_types.at(index);
    }

    uint32_t BufferLayout::getOffset(uint32_t index) const {

        uint32_t offset = 0;
        for(int i = 0; i < index; i++) {
            offset += m_types.at(i).getSize();
        }

        return offset;
    }

    unsigned int BufferLayout::getTotalSize() const {
        // @return The size of the data struct described by thisBufferLayout

        unsigned int size = 0;

        for (const FixedType &t : m_types)
            size += t.getSize();

        return size;
    }

} // namespace undicht