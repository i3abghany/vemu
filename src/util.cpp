#include "../include/util.h"

uint64_t sext_byte(uint8_t b) {
	return static_cast<uint64_t>(
		static_cast<int64_t>(
			static_cast<int8_t>(b)
		)
	);
}

uint64_t sext_hword(uint16_t hw) {
	return static_cast<uint64_t>(
		static_cast<int64_t>(
			static_cast<int16_t>(hw)
		)
	);
}

uint64_t sext_word(uint32_t w) {
	return static_cast<uint64_t>(
		static_cast<int64_t>(
			static_cast<int32_t>(w)
		)
	);
}
