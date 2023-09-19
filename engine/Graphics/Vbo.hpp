#pragma once

#include <Types.hpp>
#include <span>

void boundVboSetData(intptr_t offset, const void* data, usize dataByteSize);
void boundVboReadData(intptr_t offset, void* readData, usize readDataBufferSize);
void boundVboAllocateData(const void* data, usize dataByteSize);

class Vbo {
public:
	// Dynamic draw
	explicit Vbo(usize dataByteSize); // On 32 bit systems this would be the same constructor as the private one maybe rename to staitc function dynamic draw and static draw.
	// Static draw
	Vbo(const void* data, size_t dataByteSize);
	template<typename T>
	Vbo(std::span<const T> vertices);
	~Vbo();

	static Vbo generate();

	Vbo(const Vbo&) = delete;
	Vbo& operator= (const Vbo&) = delete;

	Vbo(Vbo&& other) noexcept;
	Vbo& operator= (Vbo&& other) noexcept;

	// The VertexBuffer must be bound before calling.
	void setData(intptr_t offset, const void* data, usize dataByteSize);
	void allocateData(const void* data, usize dataByteSize);

	void bind() const;
	void bindAsIndexBuffer() const;

	const u32 handle() const;

private:
	explicit Vbo(u32 handle);
	u32 handle_;
};

template<typename T>
Vbo::Vbo(std::span<const T> vertices) 
	: Vbo(vertices.data(), vertices.size() * sizeof(T)) {}
