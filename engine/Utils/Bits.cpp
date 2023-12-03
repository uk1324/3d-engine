#include "Bits.hpp"

struct BitPosition {
	i64 byteIndex;
	// The leftmost bit is the first bit. For 0b00000001 the leftmost bit is 0.
	i64 bitInByteIndex;
};

BitPosition calculateBitPosition(i64 bitIndex) {
	const auto byteIndex = bitIndex / 8;
	const auto bitInByteIndex = 7 - (bitIndex - byteIndex * 8); // = 7 - (byteIndex % 8)
	return BitPosition{
		.byteIndex = byteIndex,
		.bitInByteIndex = bitInByteIndex
	};
}

bool readBitFromByte(u8 byte, i64 bitIndex) {
	return (byte >> bitIndex) & 1;
}

u8 setBitInByte(u8 byte, i64 bitIndex, bool value) {
	const auto withClearedBit = byte & ~(1 << bitIndex); // bitmask
	return withClearedBit | (value << bitIndex);
}

u8 toggleBitInByte(u8 byte, i64 bitIndex) {
	return byte ^ (1 << bitIndex);
}

bool readBitFromBitset(u8* data, i64 bitIndex) {
	const auto p = calculateBitPosition(bitIndex);
	return readBitFromByte(data[p.byteIndex], p.bitInByteIndex);
}

void setBitInBitset(u8* data, i64 bitIndex, bool value) {
	const auto p = calculateBitPosition(bitIndex);
	data[p.byteIndex] = setBitInByte(data[p.byteIndex], p.bitInByteIndex, value);
}

void toggleBitInBitset(u8* data, i64 bitIndex) {
	const auto p = calculateBitPosition(bitIndex);
	data[p.byteIndex] = toggleBitInByte(data[p.byteIndex], p.bitInByteIndex);
}