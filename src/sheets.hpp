#pragma once

#include <array>

namespace sheet {

enum class iso216_category_a {
	A0,
	A1,
	A2,
	A3,
	A4,
	A5,
	A6,
	A7,
	A8,
	A9,
	A10
};

constexpr auto all_iso216_category() noexcept {
	return std::array{
		iso216_category_a::A0,
		iso216_category_a::A1,
		iso216_category_a::A2,
		iso216_category_a::A3,
		iso216_category_a::A4,
		iso216_category_a::A5,
		iso216_category_a::A6,
		iso216_category_a::A7,
		iso216_category_a::A8,
		iso216_category_a::A9,
		iso216_category_a::A10};
}

constexpr auto name(const iso216_category_a category) noexcept -> const char * {
	switch (category) {
		case iso216_category_a::A0: return "A0";
		case iso216_category_a::A1: return "A1";
		case iso216_category_a::A2: return "A2";
		case iso216_category_a::A3: return "A3";
		case iso216_category_a::A4: return "A4";
		case iso216_category_a::A5: return "A5";
		case iso216_category_a::A6: return "A6";
		case iso216_category_a::A7: return "A7";
		case iso216_category_a::A8: return "A8";
		case iso216_category_a::A9: return "A9";
		case iso216_category_a::A10: return "A10";
	}

	return nullptr;
}

struct metrics {
	const char *name{nullptr};
	double w{};
	double h{};
};

constexpr metrics make_metric(const iso216_category_a category) noexcept {
	switch (category) {
		case iso216_category_a::A0: return {name(category), 841, 1189};
		case iso216_category_a::A1: return {name(category), 594, 841};
		case iso216_category_a::A2: return {name(category), 420, 594};
		case iso216_category_a::A3: return {name(category), 297, 420};
		case iso216_category_a::A4: return {name(category), 210, 297};
		case iso216_category_a::A5: return {name(category), 148, 210};
		case iso216_category_a::A6: return {name(category), 105, 148};
		case iso216_category_a::A7: return {name(category), 74, 105};
		case iso216_category_a::A8: return {name(category), 52, 74};
		case iso216_category_a::A9: return {name(category), 37, 52};
		case iso216_category_a::A10: return {name(category), 26, 37};
	}

	return {};
}
} // namespace sheet

template <typename type>
struct inverter {
	type value;
	bool inverted{false};
};
