#include <stdio.h>
#include "byte_search/byte_pattern.h"

__attribute__((constructor))
void hooook() {

	BytePattern::StartLog("hook");

	BytePattern::temp_instance().find_pattern("74 61 72 67 65 74 20");
	if (BytePattern::temp_instance().count() > 0) {
		printf("match pattern");
	}

	printf("hello world");
}