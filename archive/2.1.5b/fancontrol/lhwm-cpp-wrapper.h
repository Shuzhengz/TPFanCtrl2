//#pragma once

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#ifndef LHWM_CPP_WRAPPER_H
#define LHWM_CPP_WRAPPER_H

#include <vector>
#include <map>
#include <string>

class LHWM
{
public:
	static std::map<std::string, std::vector<std::tuple<std::string, std::string, std::string>>> GetHardwareSensorMap();
	static float GetSensorValue(std::string identifier);
};

#endif // LHWM_CPP_WRAPPER_H
