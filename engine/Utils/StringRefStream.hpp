#pragma once

#include <sstream>

// https://stackoverflow.com/questions/20774587/why-cant-stdostream-be-moved
// std::ostream can't be moved so for example you can't store it inside a struct that you call std::remove_if on, because the type has to be swappable which in turn requires it to be movable. I guess it might also be better not to store a stream, because it stores additional state that isn't required if you just want to store a string with the ability to '<<' into it

struct StringRefStream : std::ostream {
	struct StringRefStreamBuf : public std::stringbuf {
		StringRefStreamBuf(std::string& str);
		int_type overflow(int_type c) override;
		std::string& buffer;
	};
	StringRefStream(std::string& stringToOutputTo);

	StringRefStreamBuf buffer;

	std::string& string();
	const std::string& string() const;
};