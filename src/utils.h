#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>
#include <vector>
#include <map>

namespace util {
	std::vector<std::string> splitString(std::string, char delimeter);

	template <class T>
	unsigned char firstUnusedKey(std::map<unsigned char, T*> map);

	template <class T>
	unsigned short firstUnusedKey(std::map<unsigned short, T*> map);
}

template <class T>
unsigned char util::firstUnusedKey(std::map<unsigned char, T*> map) {
	for (unsigned char i = 0; i < 255; ++i) {
		if (map.count(i) == 0) {
			return i;
		}
	}

	return 255;
}

template <class T>
unsigned short util::firstUnusedKey(std::map<unsigned short, T*> map) {
	for (unsigned short i = 0; i < 65535; ++i) {
		if (map.count(i) == 0) {
			return i;
		}
	}

	return 65535;
}

#endif
