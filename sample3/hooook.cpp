#include <stdio.h>
#include "byte_search/byte_pattern.h"
#include "injector/injector.hpp"
#include <sys/errno.h>


__attribute__((constructor))
void hooook() {
	BytePattern::StartLog("hook");

	BytePattern::temp_instance().find_pattern("74 61 72 67 65 74 20");
	if (BytePattern::temp_instance().count() > 0) {
		printf("match pattern\n");

		uintptr_t address = BytePattern::temp_instance().get_first().address();

		auto ptr = Injector::ReadMemory<char>(address, true);

		printf("%d\n", ptr);
	}

	BytePattern::temp_instance().find_pattern("74 61 72 67 65 74 20");
	if (BytePattern::temp_instance().count() > 0) {
		printf("match pattern\n");

		auto address = BytePattern::temp_instance().get_first().address();

		uint8_t a = 64;

		Injector::WriteMemory<uint8_t>(address, a, true);

	}

	printf("hello world\n");
}