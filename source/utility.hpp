#include <cassert>
#include <optional>
#include <array>
#include <algorithm>

#define ASSERT_UNREACHABLE() assert(false && "Unreachable code reached.")

// H/T Jason Turner for teaching me this one
template<typename Key, typename Value, std::size_t Size>
struct ConstMap {
	std::array<std::pair<Key, Value>, Size> data;

	// intentionally linear search for GCC optimizations
	[[nodiscard]] constexpr std::optional<Value>
	maybe_at(const Key& key) const
	{
		const auto itr = std::find_if(begin(data), end(data),
			[&key](const auto& v) { return v.first == key; });

		if (itr != end(data)) {
			return std::optional<Value>(itr->second);
		} else {
			return std::nullopt;
		}
	}
};
