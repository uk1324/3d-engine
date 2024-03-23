//#pragma once
//
//#include <Types.hpp>
//
//class Texture1d {
//public:
//	Texture1d(uint32_t handle);
//	~Texture1d();
//
//	Texture1d(const Texture1d&) = delete;
//	Texture1d& operator= (const Texture1d&) = delete;
//
//	Texture1d(Texture1d&& other) noexcept;
//	Texture1d& operator=(Texture1d&& other) noexcept;
//
//	void bind();
//	static void unbind();
//
//	u32 handle() const;
//
//public:
//	static Texture1d generate();
//	static Texture1d null();
//
//private:
//	u32 handle_;
//};