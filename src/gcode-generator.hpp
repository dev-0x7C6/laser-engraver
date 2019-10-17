#pragma once

#include <src/semi-gcode.hpp>
#include <fstream>

enum class upload_instruction_ret {
	keep_going,
	cancel
};

using upload_instruction = std::function<upload_instruction_ret(std::string &&gcode, double)>;

void generate_gcode(semi_gcodes &&gcodes, const upload_instruction &instruction);
