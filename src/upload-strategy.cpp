#include "upload-strategy.hpp"

#include <QFile>
#include <QTextStream>

#include <memory>

auto upload::to_file(string &&path) -> upload_instruction {
	auto file = std::make_unique<QFile>(path);
	file->open(QIODevice::ReadWrite);
	auto out = std::make_unique<QTextStream>(file.get());
	return [file{std::shared_ptr(std::move(file))},
			   out(std::shared_ptr{std::move(out)})](std::string &&gcode, double) -> upload_instruction_ret {
		(*out) << QString::fromStdString(gcode) << "\n";
		return upload_instruction_ret::keep_going;
	};
}
