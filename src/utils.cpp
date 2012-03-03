#include "utils.h"

std::vector<std::string> util::splitString(std::string string, char delimeter) {
	std::vector<std::string> items;
	std::string item;
	std::stringstream stream(string);

	while (std::getline(stream, item, delimeter)) {
		items.push_back(item);
	}

	return items;
}

std::string util::toString(int i) {
	std::ostringstream stream;
	stream << i;
	return stream.str();
}
