#ifndef BUFFER_LAYOUT_H
#define BUFFER_LAYOUT_H

#include <types.h>
#include <vector>

namespace undicht {

    class BufferLayout {
        /** layout of the data structure in a buffer */
      public:
      
        std::vector<FixedType> m_types;

        BufferLayout(const std::vector<FixedType> &types);

        unsigned int getTotalSize();

        BufferLayout() = default;
        virtual ~BufferLayout() = default;
    };

} // namespace undicht

#endif // BUFFER_LAYOUT_H