#include "StringRefStream.hpp"

StringRefStream::StringRefStream(std::string& stringToOutputTo) 
	: std::ostream(&buffer)
	, buffer(stringToOutputTo) {}

std::string& StringRefStream::string() {
	return buffer.buffer;
}

const std::string& StringRefStream::string() const {
	return buffer.buffer;
}

StringRefStream::StringRefStreamBuf::StringRefStreamBuf(std::string& str)
	: buffer(str) {
}

std::stringbuf::int_type StringRefStream::StringRefStreamBuf::overflow(int_type c) {
	buffer += static_cast<char>(c);
	return 0;
}
