add_executable(test "./engine/Math/Matrix/GaussianElimination.cpp" "engine/Math/Matrix/Arithmetic.cpp" "../engine/Utils/Assertions.cpp" "engine/Utils/Span2d.cpp")

enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

target_include_directories(
	test PRIVATE
	"../"
	"../engine/Utils"
)

target_link_libraries(test gtest gtest_main)

# add_test(voxel-game-test voxel-game-test)