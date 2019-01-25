#pragma once

#include <src/semi-gcode.hpp>
#include <fstream>

void generate_gcode(std::string &&dir, semi_gcodes &&gcodes);
