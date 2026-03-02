#pragma once
#include <cstring>

namespace DNet
{

class Buffer
{
public:
	static constexpr size_t STACK_SIZE = 4096;

	explicit Buffer(size_t size);
	Buffer(const char *data, size_t size);
	~Buffer();

	Buffer(const Buffer &) = delete;
	Buffer &operator=(const Buffer &) = delete;

	Buffer(Buffer &&o) noexcept;
	Buffer &operator=(Buffer &&o) noexcept;

	char *data();
	const char *data() const;

	size_t size() const;
	size_t capacity() const;

	void reserve(size_t newCapacity);
	void resize(size_t newSize);
	void assign(const char *data, size_t size);

private:
	size_t size_;
	size_t capacity_;
	bool stack_;

	union
	{
		char stackData[STACK_SIZE];
		char *heapData;
	} data_;
};

} // namespace DNet