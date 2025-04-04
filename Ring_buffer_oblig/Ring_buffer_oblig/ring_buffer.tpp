#pragma once

#include "ringbuffer.h"

// Push an item into the ring buffer
template <typename T>
void ring_buffer<T>::push(const T& item)
{
    // Store the item
}

// Pop an item from the ring buffer
template <typename T>
T ring_buffer<T>::pop()
{
    // Retrieve an item
    T item = std::move(m_buffer[m_tail]);
    m_tail = (m_tail + 1) % m_capacity;
    
    m_size -= 1;
    
    return item;
}

// Get the size of the ring buffer
template <typename T>
size_t ring_buffer<T>::size() const
{
    return m_size;
}

// Returns true if the buffer is empty, false otherwise.
template <typename T>
bool ring_buffer<T>::empty() const
{
    return (m_size == 0);
}

// Returns true if the buffer is full, false otherwise.
template <typename T>
bool ring_buffer<T>::full() const
{
    return (m_size == m_capacity);
}

// Returns the capacity (maximum number of items this ring can hold).
template <typename T>
size_t ring_buffer<T>::capacity() const noexcept
{
    return m_capacity;
}