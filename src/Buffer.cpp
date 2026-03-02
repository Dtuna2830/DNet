#include "DNet/Buffer.h"

namespace DNet
{

Buffer::Buffer(size_t size) : size_(size), stack_(size <= STACK_SIZE)
{
	if (stack_) capacity_ = STACK_SIZE;
	else capacity_ = size, data_.heapData = new char[size];
}

Buffer::Buffer(const char *data, size_t size) : Buffer(size)
{
	if (data) memcpy(this->data(), data, size);
}

Buffer::~Buffer()
{
	if (!stack_) delete[] data_.heapData;
}

Buffer::Buffer(Buffer &&o) noexcept : size_(o.size_), capacity_(o.capacity_), stack_(o.stack_)
{
	if (stack_) memcpy(data_.stackData, o.data_.stackData, size_);
	else data_.heapData = o.data_.heapData, o.data_.heapData = nullptr;
	o.size_ = 0;
	o.capacity_ = 0;
	o.stack_ = true;
}

Buffer &Buffer::operator=(Buffer &&o) noexcept
{
	if (this == &o) return *this;
	if (!stack_) delete[] data_.heapData;
	size_ = o.size_;
	capacity_ = o.capacity_;
	stack_ = o.stack_;
	if (stack_) memcpy(data_.stackData, o.data_.stackData, size_);
	else data_.heapData = o.data_.heapData, o.data_.heapData = nullptr;
	o.size_ = 0;
	o.capacity_ = 0;
	o.stack_ = true;
	return *this;
}

char *Buffer::data()
{
	return stack_ ? data_.stackData : data_.heapData;
}

const char *Buffer::data() const
{
	return stack_ ? data_.stackData : data_.heapData;
}

size_t Buffer::size() const
{
	return size_;
}

size_t Buffer::capacity() const
{
	return capacity_;
}

void Buffer::reserve(size_t newCapacity)
{
	if (newCapacity <= capacity_) return;
	char *newData = new char[newCapacity];
	memcpy(newData, data(), size_);
	if (!stack_) delete[] data_.heapData;
	data_.heapData = newData;
	stack_ = false;
	capacity_ = newCapacity;
}

void Buffer::resize(size_t newSize)
{
	if (newSize > capacity_) reserve(newSize);
	size_ = newSize;
}

void Buffer::assign(const char *data, size_t size)
{
	resize(size);
	if (data) memcpy(this->data(), data, size);
}

} // namespace DNet