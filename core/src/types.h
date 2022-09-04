#ifndef TYPES_H
#define TYPES_H

namespace undicht {

    enum class Type {
        UNDEFINED,
        INT, // group of all integer types (short, char, int, long, ...)
        UNSIGNED_INT,
        FLOAT,
        COLOR_BGRA, // reverse order of channels
        COLOR_RGBA,
        DEPTH_BUFFER,
        DEPTH_STENCIL_BUFFER,
    };

    class FixedType {
        /** defines a type of fixed size, layout, ...*/
      public:

        Type m_type;
        unsigned int m_size; // size of every component in bytes
        unsigned int m_num_components = 1; // for composite types such as 3D vectors

        // the bytes of the type are arranged from lowest to highest in memory
        bool m_little_endian = true;

      public:
        // Constructors
        FixedType() = default;
        FixedType(Type t, unsigned int size, unsigned int num_comp = 1, bool little_endian = true);

      public:
        // functions for cleaner usage of the class

        // allows for tests like this: t == Type::INT
        operator Type();

        bool operator== (const FixedType& t) const;

        // size of the complete type (number of components * size of each component)
        unsigned int getSize() const;
    };


// some pre defined FixedTypes
#define UND_UNDEFINED_TYPE FixedType(Type::UNDEFINED, 0)

#define UND_INT8 FixedType(Type::INT, 1)
#define UND_INT16 FixedType(Type::INT, 2)
#define UND_INT32 FixedType(Type::INT, 4)
#define UND_INT64 FixedType(Type::INT, 8)

#define UND_UINT8 FixedType(Type::UNSIGNED_INT, 1)
#define UND_UINT16 FixedType(Type::UNSIGNED_INT, 2)
#define UND_UINT32 FixedType(Type::UNSIGNED_INT, 4)
#define UND_UINT64 FixedType(Type::UNSIGNED_INT, 8)

#define UND_FLOAT32 FixedType(Type::FLOAT, 4) // float
#define UND_FLOAT64 FixedType(Type::FLOAT, 8) // double

#define UND_VEC2F FixedType(Type::FLOAT, 4, 2)
#define UND_VEC3F FixedType(Type::FLOAT, 4, 3)
#define UND_VEC4F FixedType(Type::FLOAT, 4, 4)

#define UND_VEC2I FixedType(Type::INT, 4, 2)
#define UND_VEC3I FixedType(Type::INT, 4, 3)
#define UND_VEC4I FixedType(Type::INT, 4, 4)

#define UND_MAT3F FixedType(Type::FLOAT, 4, 9)
#define UND_MAT4F FixedType(Type::FLOAT, 4, 16)

#define UND_R8 FixedType(Type::COLOR_RGBA, 1, 1)
#define UND_R8G8 FixedType(Type::COLOR_RGBA, 1, 2)
#define UND_R8G8B8 FixedType(Type::COLOR_RGBA, 1, 3)
#define UND_R8G8B8A8 FixedType(Type::COLOR_RGBA, 1, 4)

#define UND_B8G8R8 FixedType(Type::COLOR_BGRA, 1, 3)
#define UND_B8G8R8A8 FixedType(Type::COLOR_BGRA, 1, 4)

#define UND_DEPTH32F undicht::FixedType(Type::DEPTH_BUFFER, 4, 1)
#define UND_DEPTH32f_STENCIL8 undicht::FixedType(Type::DEPTH_STENCIL_BUFFER, 5, 1)
#define UND_DEPTH24_STENCIL8 undicht::FixedType(Type::DEPTH_STENCIL_BUFFER, 4, 1)

} // namespace undicht

#endif // TYPES_H