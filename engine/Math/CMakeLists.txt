add_library(
	math
	"Aabb.cpp" "Line.cpp" "LineSegment.cpp"  "Polygon.cpp" "Transform.cpp" "Triangulate.cpp" "Triangle.cpp" "Utils.cpp"
 "Color.cpp" "Random.cpp" "Frustum.cpp" "Plane.cpp" "Aabb3.cpp" "FourierTransform.cpp")

target_sources(math PUBLIC "math.natvis")

target_include_directories(math PUBLIC "../Utils")