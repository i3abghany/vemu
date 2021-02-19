#pragma once

#include "DRAM.h"

class Bus {
public:
	Bus();
	uint64_t load(uint64_t, size_t);
	void store(uint64_t, uint64_t, size_t);
private:
	DRAM dr;
};
