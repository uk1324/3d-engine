#include <engine/EngineUpdateLoop.hpp>
#include <engine/Engine.hpp>
#include <water_rendering/MainLoop.hpp>

#include <engine/Utils/Array2d.hpp>

float g = 9.81f;
float dt = 0.1f;

//---------------------------------------------------------------

//static constexpr int GRID_SIZE = 200;
//float dxdy = 0.4f;
//float pix_step = 5.0f;
static constexpr int GRID_SIZE = 400;
float dxdy = 0.4f;
float pix_step = 5.0f / 2.0;

/*

u	is the velocity in the x direction, or zonal velocity
v	is the velocity in the y direction, or meridional velocity
H	is the mean height of the horizontal pressure surface
h	is the height deviation of the horizontal pressure surface from its mean height, where h: η(x, y, t) = H(x, y) + h(x, y, t)

*/


//---------------------------------------------------------------

//struct Grid
//{
//
//	/*std::vector<std::vector<float>> mesh;*/
//	float mesh[GRID_SIZE][GRID_SIZE];
//
//	Grid(float init) {
//		for (int ii = 0; ii < GRID_SIZE; ii++)
//		{
//			for (int jj = 0; jj < GRID_SIZE; jj++)
//			{
//
//				mesh[ii][jj] = init;
//			}
//		}
//	}
//
//};

#include <engine/Input/Input.hpp>

int main() {
	Engine::initAll(Window::Settings{ .multisamplingSamplesPerPixel = 4 });

	//Grid water_h(1.0f); // water hight
	//Grid vel_u(0.0f);	// water velocity in x direction
	//Grid vel_v(0.0f);	// water velocity in y dirction
	auto water_h = Array2d<float>::withAllSetTo(GRID_SIZE, GRID_SIZE, 1.0f);
	auto vel_u = Array2d<float>::withAllSetTo(GRID_SIZE, GRID_SIZE, 0.0f);
	auto vel_v = Array2d<float>::withAllSetTo(GRID_SIZE, GRID_SIZE, 0.0f);

	// source

	//water_h(25, 25) = 20.0f;

	EngineUpdateLoop loop(60.0);
	//auto mainLoop = MainLoop::make();

	while (loop.isRunning()) {
		ImDrawList* draw_list = ImGui::GetWindowDrawList();


		/*if (Input::isKeyDown(KeyCode::H)) {
			water_h(100, 100) = 10.0f;
		}*/

		if (Input::isKeyDown(KeyCode::H)) {
			/*for (int ii = 100; ii < 110; ii++)
			{
				for (int jj = 100; jj < 110; jj++)
				{
					water_h(ii, jj) = 5.0f;
				}
			}*/
			i64 rectangleSize = 40;
			Vec2T<i64> offset{ GRID_SIZE / 2 - rectangleSize / 2 };
			for (i64 y = 0; y < rectangleSize; y++) {
				for (i64 x = 0; x < rectangleSize; x++) {
					if ((Vec2(x + 0.5f, y + 0.5f) / rectangleSize - Vec2(0.5f)).length() < 0.49f) {
						water_h(offset.x + x, offset.y + y) = 5.0f;
					}
				}
			}
		}

		

		static bool update = true;
		ImGui::Begin("test");
		ImGui::Checkbox("update", &update);
		ImGui::End();

		if (update) {
			for (i64 y = 1; y < GRID_SIZE - 1; y++) {
				for (i64 x = 1; x < GRID_SIZE - 1; x++) {

					float dh_dx = (water_h(x + 1, y) - water_h(x - 1, y)) / (2 * dxdy);
					float dh_dy = (water_h(x, y + 1) - water_h(x, y - 1)) / (2 * dxdy);

					vel_u(x, y) = vel_u(x, y) - dt * g * dh_dx;
					vel_v(x, y) = vel_v(x, y) - dt * g * dh_dy;

					/*float du_dx = (vel_u(ii + 1, jj) - vel_u(ii - 1, jj)) / (2 * dxdy);
					float dv_dy = (vel_v(ii, jj + 1) - vel_v(ii, jj - 1)) / (2 * dxdy);

					water_h(ii, jj) = water_h(ii, jj) + dt * (-(du_dx + dv_dy));*/
				}
			}

			for (i64 y = 1; y < GRID_SIZE - 1; y++) {
				for (i64 x = 1; x < GRID_SIZE - 1; x++) {
					float du_dx = (vel_u(x + 1, y) - vel_u(x - 1, y)) / (2 * dxdy);
					float dv_dy = (vel_v(x, y + 1) - vel_v(x, y - 1)) / (2 * dxdy);

					water_h(x, y) = water_h(x, y) + dt * (-(du_dx + dv_dy));
				}
			}

			
		}

		for (int ii = 0; ii < GRID_SIZE; ii++)
		{
			for (int jj = 0; jj < GRID_SIZE; jj++)
			{
				float D = 0.0;
				ImVec2 p0;
				ImVec2 p1;
				p0 = { (float)ii * pix_step, (float)jj * pix_step };
				p1 = { ((float)ii * pix_step) + pix_step, ((float)jj * pix_step) + pix_step };

				float hi = water_h(ii, jj);

				ImVec4 pix = ImVec4(0.0f, 0.0f, hi - 0.3f, 1.0f);

				draw_list->AddRectFilled(p0, p1, ImColor(pix));

			}
		}

		//mainLoop.update();
	}

	Engine::terminateAll();
}