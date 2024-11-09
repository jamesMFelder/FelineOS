#include <feline/minmax.h>
#include <feline/ranges.h>
#include <feline/tests.h>

int main() {
	initialize_loggers();

	range<size_t> test_range = range<size_t>{.start = 10, .end = 20};
	for (size_t a : {0, 5, 9, 10, 15, 20, 21, 15, 30}) {
		for (size_t b : {0, 1, 5, 10, 15, 20, 21, 30}) {
			auto start = min(a, b);
			auto end = max(a, b);
			// If the first range starts before the second …
			if (start < test_range.start) {
				// … and ends before the second: they don't overlap
				if (end < test_range.start) {
					REQUIRE_NOT(
						overlap(range{.start = a, .end = b}, test_range));
				}
				// … and ends after the second range starts: they do overlap
				else {
					REQUIRE(overlap(range{.start = a, .end = b}, test_range));
				}
			}
			// If the first range starts in the middle of the second range: they
			// overlap
			else if (start <= test_range.end) {
				REQUIRE(overlap(range{.start = a, .end = b}, test_range));
			}
			// If the first range starts after the second range: they do not
			// overlap
			else {
				REQUIRE_NOT(overlap(range{.start = a, .end = b}, test_range));
			}
		}
	}
}
