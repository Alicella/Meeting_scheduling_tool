#ifndef HELPER_FUNCS_H
#define HELPER_FUNCS_H

#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <cstring>

std::vector<std::vector<int>> find_intersection(std::vector<std::vector<int>> slots1, std::vector<std::vector<int>> slots2);
std::string slotVector_to_str(std::vector<std::vector<int>> slot);
std::vector<std::vector<int>> slotStr_to_vector(std::string slotStr);
std::string namesVector_to_str(std::vector<std::string> names);
// std::vector<std::string> nameStr_to_vector(std::stringstream ss);

#endif