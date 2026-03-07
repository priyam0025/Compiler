#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <iostream>

class ArenaAllocator {
public:
    explicit ArenaAllocator(std::size_t size = 1024 * 1024)
        : m_size(size), m_offset(0)
    {
        m_data = std::malloc(m_size);
        if (!m_data) {
            std::cerr << "Failed to allocate arena memory\n";
            std::exit(EXIT_FAILURE);
        }
    }

    ~ArenaAllocator() {
        std::free(m_data);
    }

    template<typename T>
    T* alloc() {
        const std::size_t align = alignof(T);
        std::uintptr_t base = reinterpret_cast<std::uintptr_t>(m_data);
        std::uintptr_t current = base + m_offset;
        std::uintptr_t aligned = (current + (align - 1)) & ~(align - 1);
        std::size_t used = static_cast<std::size_t>(aligned - base);
        if (used + sizeof(T) > m_size) {
            std::cerr << "Arena out of memory\n";
            std::exit(EXIT_FAILURE);
        }
        void* ptr = static_cast<char*>(m_data) + used;
        m_offset = used + sizeof(T);
        return new (ptr) T();
    }

    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

private:
    void* m_data = nullptr;
    std::size_t m_size = 0;
    std::size_t m_offset = 0;
};