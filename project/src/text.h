#ifndef NANO_EDR_TEXT_H
#define NANO_EDR_TEXT_H

#include <string>

namespace nano_edr {

bool TextEquals(const std::string& left, const std::string& right);
bool TextContains(const std::string& text, const std::string& needle);
bool TextEndsWith(const std::string& text, const std::string& suffix);

}  // namespace nano_edr

#endif  // NANO_EDR_TEXT_H
