#pragma once

#include <src/gcode-generator.hpp>

#include <QString>

using string = QString;

namespace upload {
auto to_file(string &&path) -> upload_instruction;
}
