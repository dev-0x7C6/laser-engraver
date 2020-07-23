#pragma once

constexpr auto inch = 25.4;

constexpr double calculate_precision(const double dpi = 600.0) {
	return dpi / inch;
}

template <typename return_type = double, typename T, typename U>
constexpr auto divide(T &&x, U &&u) -> return_type {
	return static_cast<return_type>(static_cast<double>(x) / static_cast<double>(u));
}

template <typename return_type = double, typename T, typename U>
constexpr auto multiply(T &&x, U &&u) -> return_type {
	return static_cast<return_type>(static_cast<double>(x) * static_cast<double>(u));
}
