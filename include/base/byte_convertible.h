#pragma once
#include <vector>
#include <cstring>
#include <stdexcept>
#include <cstdint>

//base class for converting byte data to structs
template<typename T>
class ByteConvertible {
public:
    static T from_bytes(const std::vector<std::byte>& bytes) {
        if (bytes.size() < sizeof(T)) {
            throw std::runtime_error("Insufficient bytes to cast to struct");
        }
        
        T result;
        std::memcpy(&result, bytes.data(), sizeof(T));
        return result;
    }
    
    static T from_bytes(const std::vector<uint8_t>& bytes) {
        if (bytes.size() < sizeof(T)) {
            throw std::runtime_error("Insufficient bytes to cast to struct");
        }
        
        T result;
        std::memcpy(&result, bytes.data(), sizeof(T));
        return result;
    }
    
    static T from_data_ptr(const void* data, size_t size) {
        if (size < sizeof(T)) {
            throw std::runtime_error("Insufficient data to cast to struct");
        }
        
        T result;
        std::memcpy(&result, data, sizeof(T));
        return result;
    }
    
    //convert current struct to bytes
    std::vector<std::byte> to_bytes() const {
        const T* derived = static_cast<const T*>(this);
        std::vector<std::byte> result(sizeof(T));
        std::memcpy(result.data(), derived, sizeof(T));
        return result;
    }
};

//template base class for variable-length array chunks
template<typename T, typename ElementType>
class VariableLengthArrayChunk : public ByteConvertible<T> {
public:
    //static factory method for creating from bytes with count
    static T from_bytes(const std::vector<std::byte>& bytes, size_t element_count) {
        T result(element_count);
        
        if (bytes.size() < element_count * sizeof(ElementType)) {
            throw std::runtime_error("Insufficient bytes for array elements");
        }
        
        auto& container = result.get_container();
        for (size_t i = 0; i < element_count; ++i) {
            ElementType element;
            std::memcpy(&element, bytes.data() + i * sizeof(ElementType), sizeof(ElementType));
            container[i] = element;
        }
        
        return result;
    }
  
    //pure virtual function to get the container reference, each derived class must implement this to return their specific container
    virtual std::vector<ElementType>& get_container() = 0;
    virtual const std::vector<ElementType>& get_container() const = 0;
    
    size_t ByteSize() const {
        return get_container().size() * sizeof(ElementType);
    }
};
