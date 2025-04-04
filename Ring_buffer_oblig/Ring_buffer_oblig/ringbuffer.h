#pragma once

#include <vector>
#include <stdexcept>
#include <utility>

template <typename T>
class ring_buffer {
public:
	explicit ring_buffer(size_t capacity):
		m_buffer(capacity),
		m_capacity(capacity),
		m_head(0),
		m_tail(0),
		m_size(0) {}

	void push(const T& item);
	T pop();

	bool empty() const;
	bool full() const;
	size_t size() const;
	size_t capacity() const noexcept;

pritvate:
	std::vector<T> m_buffer;
	const size_t m_capacity;
	size_t m_head;
	size_t m_tail;
	size_t m_size;
};

#include "ring_buffer.tpp"