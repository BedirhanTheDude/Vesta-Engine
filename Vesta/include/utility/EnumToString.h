#pragma once

#define ENUM_TO_STRING(x) case x: return #x;

/* Example
   switch (type) {
			ENUM_TO_STRING(Static);
			ENUM_TO_STRING(Dynamic);
			ENUM_TO_STRING(Kinematic);
		}
*/