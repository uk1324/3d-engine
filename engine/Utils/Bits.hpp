#pragma once

#include <Types.hpp>

//u8 a[] = { 0b00110010, 0b10101011, 5 };
//
//for (int i = 0; i < 24; i++) {
//	setBitInBitset(a, i, i % 2);
//}
//
//for (int i = 0; i < 24; i++) {
//	std::cout << readBitFromBitset(a, i);
//}

bool readBitFromByte(u8 byte, i64 bitIndex);
u8 setBitInByte(u8 byte, i64 bitIndex, bool value);
u8 toggleBitInByte(u8 byte, i64 bitIndex);

bool readBitFromBitset(u8* data, i64 bitIndex);
void setBitInBitset(u8* data, i64 bitIndex, bool value);
void toggleBitInBitset(u8* data, i64 bitIndex);

//void setBitsInBitsetArray