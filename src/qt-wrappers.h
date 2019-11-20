#pragma once

#include <QString>

#include <functional>
#include <future>
#include <thread>

#include <src/semi-gcode.hpp>

void qt_generate_progress_dialog(QString &&title, progress_t &progress);

template <typename return_type>
return_type qt_progress_task(QString &&title, std::function<return_type(progress_t &)> &&callable) {
	progress_t progress{};
	auto task = std::packaged_task<return_type()>([callable{std::move(callable)}, &progress]() {
		return callable(progress);
	});

	auto result = task.get_future();

	std::thread thread(std::move(task));

	qt_generate_progress_dialog(std::move(title), progress);
	result.wait();
	thread.join();

	return result.get();
}
