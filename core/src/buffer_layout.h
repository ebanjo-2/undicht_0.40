#ifndef BUFFER_LAYOUT_H
#define BUFFER_LAYOUT_H

#include <types.h>
#include <vector>
#include <cstdint>

namespace undicht {

    class BufferLayout {
        /** layout of the data structure in a buffer */
      public:
      
        std::vector<FixedType> m_types;

        BufferLayout() = default;
        BufferLayout(const std::vector<FixedType> &types);
        virtual ~BufferLayout() = default;

        void setType(uint32_t index, const FixedType& type);
        const FixedType& getType(uint32_t index) const;

        uint32_t getOffset(uint32_t index) const;

        unsigned int getTotalSize() const;
    };

} // namespace undicht

#endif // BUFFER_LAYOUT_H
