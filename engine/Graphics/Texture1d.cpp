//#include "Texture1d.hpp"
//#include <glad/glad.h>
//
//Texture1d::Texture1d(uint32_t handle) 
//	: handle_(handle) {
//}
//
//Texture1d::~Texture1d() {
//	glDeleteTextures(1, &handle_);
//}
//
//Texture1d::Texture1d(Texture1d&& other) noexcept 
//	: handle_(other.handle_) {
//	other.handle_ = 0;
//}
//
//Texture1d& Texture1d::operator=(Texture1d&& other) noexcept {
//	glDeleteTextures(1, &handle_);
//	handle_ = other.handle_;
//	other.handle_ = 0;
//}
//
//void Texture1d::bind() {
//	glBindTexture(GL_TEXTURE_1D, handle_);
//}
//
//void Texture1d::unbind() {
//	glBindTexture(GL_TEXTURE_1D, 0);
//}
//
//u32 Texture1d::handle() const {
//	return handle_;
//}
//
//Texture1d Texture1d::generate() {
//	u32 handle;
//	glGenTextures(1, &handle);
//	return Texture1d(handle);
//}
//
//Texture1d Texture1d::null() {
//	return Texture1d(0);
//}
