#pragma once

#include <vector>
#include <unordered_map>
#include <engine/Utils/Types.hpp>

// For geometric algorithm would it be better to take for example arrays of positions, normals, indices or would take a vector of vertex structs. The first case could also take vertex structs by using custom strides.
// void calculateNormals(StridedSpan<const Vec3> positions, StridedSpan<Vec3> outputNormals);
// template<typename Vertex>
// void calculateNormals(Span<Vertex>);

template<typename Vertex>
struct IndexedMeshBuilder {
	std::unordered_map<Vertex, u32> vertexToIndex;
	std::vector<Vertex> vertices;
	std::vector<u32> indices;

	void addVertex(const Vertex& vertex);
	void addQuad(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Vertex& v3);
};

template<typename Vertex>
struct IndexedMesh {
	IndexedMesh(IndexedMeshBuilder<Vertex>&& builder) noexcept;

	std::vector<Vertex> vertices;
	std::vector<u32> indices;
};

template<typename Vertex>
void IndexedMeshBuilder<Vertex>::addVertex(const Vertex& vertex) {
	auto vertexIt = vertexToIndex.find(vertex);
	if (vertexIt == vertexToIndex.end()) {
		const auto index = vertices.size();
		vertices.push_back(vertex);
		indices.push_back(index);
		vertexToIndex.insert({ vertex, index });
	} else {
		indices.push_back(vertexIt->second);
	}
}

template<typename Vertex>
void IndexedMeshBuilder<Vertex>::addQuad(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Vertex& v3) {
	addVertex(v0);
	addVertex(v1);
	addVertex(v2);

	addVertex(v0);
	addVertex(v2);
	addVertex(v3);
}

template<typename Vertex>
IndexedMesh<Vertex>::IndexedMesh(IndexedMeshBuilder<Vertex>&& builder) noexcept
	: vertices(std::move(builder.vertices))
	, indices(std::move(builder.indices)) {}
