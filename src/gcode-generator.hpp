#pragma once

#include <src/semi-gcode.hpp>
#include <fstream>

void generate_gcode(semi_gcodes &&gcodes, std::function<void(std::string &&gcode)> &&instruction);
