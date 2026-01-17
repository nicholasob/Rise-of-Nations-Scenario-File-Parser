#pragma once
#include <string>
#include <vector>
#include <cstddef>
#include <type_traits>

// Forward declaration
struct FieldInfo;

/**
 * @brief Base class for reflected chunks
 *
 * Provides compile-time reflection capabilities for chunk structures.
 * Derived classes define metadata using static descriptor methods.
 */
template<typename Derived>
class ReflectedChunk {
public:
    /**
     * @brief Get field metadata for this chunk type
     *
     * Derived classes should provide a static getFieldDescriptors() method
     * that returns a vector of FieldDescriptors.
     */
    static std::vector<FieldInfo> getFieldMetadata();
};

/**
 * @brief Field descriptor for reflection
 */
struct FieldDescriptor {
    const char* name;
    size_t offset;
    size_t size;
    const char* type;
    const char* description;

    FieldDescriptor(const char* n, size_t off, size_t sz, const char* t, const char* desc = "")
        : name(n), offset(off), size(sz), type(t), description(desc) {}
};

/**
 * @brief Macro to begin field descriptors for a struct
 */
#define BEGIN_FIELD_DESCRIPTORS(StructName) \
    static std::vector<FieldDescriptor> getFieldDescriptors() { \
        using SelfType = StructName; \
        std::vector<FieldDescriptor> fields;

/**
 * @brief Macro to add a field descriptor
 */
#define DESCRIBE_FIELD(Type, Name, Description) \
        fields.emplace_back( \
            #Name, \
            offsetof(SelfType, Name), \
            sizeof(Type), \
            #Type, \
            Description \
        );

/**
 * @brief Macro to end field descriptors
 */
#define END_FIELD_DESCRIPTORS() \
        return fields; \
    }

/**
 * @brief Trait to check if a type has reflection
 */
template<typename T, typename = void>
struct has_reflection : std::false_type {};

template<typename T>
struct has_reflection<T, std::void_t<decltype(T::getFieldDescriptors())>> : std::true_type {};

template<typename T>
inline constexpr bool has_reflection_v = has_reflection<T>::value;
