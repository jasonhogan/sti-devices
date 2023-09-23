
#ifndef BLACKFLYCONFIGFILE_H
#define BLACKFLYCONFIGFILE_H

#include "ConfigFile.h"

#include <map>
#include <string>
#include <sstream>
#include <vector>


class BlackflyConfigFile
{
public:

	BlackflyConfigFile(const std::string& filename);
	~BlackflyConfigFile() {}

	ConfigFile getConfigFile(const std::string& section);	//temp

	std::vector<std::string> getSectionNames();

	template <class T>
	bool getParameter(const std::string& name, T& value) const
	{
		getParameter("", name, value)
	}

	template <class T>
	bool getParameter(const std::string& section, const std::string& name, T& value) const
	{
		std::string strValue;
		if (!getStringValue(section, name, strValue)) {
			return false;
		}

		return stringToValue(strValue, value);
	}

	void parse();

private:
	
	void copyParameters(const std::string& section, ConfigFile& configFile);

	bool assignStringValue(const std::string& section, std::string line);

	bool setStringValue(const std::string& section, const std::string& name, const std::string& value);

	bool getStringValue(const std::string& name, std::string &value) const;
	bool getStringValue(const std::string& section, const std::string& name, std::string &value) const;

	std::string trim(const std::string& input) const;

	std::string filename;
	bool parsed;

	struct ConfigSection
	{
		std::string section;
		std::map<std::string, std::string> parameters;
	};
	
	std::map<std::string, ConfigSection> configData;


	bool stringToValue(std::string inString, std::string& outValue, std::ios::fmtflags numBase = std::ios::dec) const
	{
		outValue = inString;
		return true;
	}

	template<typename T> bool stringToValue(std::string inString, T& outValue, std::ios::fmtflags numBase = std::ios::dec) const
	{
		//Returns true if the conversion is successful
		std::stringstream tempStream;
		tempStream.setf(numBase, std::ios::basefield);

		tempStream << inString;
		tempStream >> outValue;

		return !tempStream.fail();
	};

	template<typename T> std::string valueToString(T inValue, std::string Default = "", std::ios::fmtflags numBase = std::ios::dec) const
	{
		std::string outString;
		std::stringstream tempStream;
		tempStream.setf(numBase, std::ios::basefield);

		tempStream << inValue;
		outString = tempStream.str();

		if (!tempStream.fail())
			return outString;
		else
			return Default;
	};

};



#endif


