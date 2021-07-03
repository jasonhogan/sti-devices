
#include "BlackflyConfigFile.h"

#include <fstream>
#include <filesystem>
#include <vector>

namespace fs = std::experimental::filesystem;

BlackflyConfigFile::BlackflyConfigFile(const std::string& filename)
: filename(filename), parsed(false)
{
	parse();
}

std::vector<std::string> BlackflyConfigFile::getSectionNames()
{
	std::vector<std::string> sections;

	for (auto& data : configData) {
		sections.push_back(data.first);
	}
	return sections;
}

ConfigFile BlackflyConfigFile::getConfigFile(const std::string& section)
{
	//Hack:
	//Need to successfully parse a fake temp file before we can use a ConfigFile object
	auto basefilename = fs::path(filename).filename();
	auto tempPath = fs::path(filename).parent_path();
	std::string tempFilename = "tmp-" + basefilename.string();

	tempPath /= tempFilename;

	//Make the temp file
	std::fstream dummyFile(tempPath.string(), std::fstream::out);
	dummyFile << "# ";
	dummyFile.close();

	ConfigFile configFile(tempPath.string());

	fs::remove(tempPath);	//delete temp file

	copyParameters("", configFile);
	copyParameters(section, configFile);

	return configFile;
}

void BlackflyConfigFile::copyParameters(const std::string& section, ConfigFile& configFile)
{
	auto data = configData.find(section);

	if (data != configData.end()) {
		for (auto p : data->second.parameters) {
			configFile.setParameter(p.first, p.second);
		}
	}	
}

void BlackflyConfigFile::parse()
{
	std::fstream configFile(filename.c_str(), std::fstream::in);

	if (!configFile.is_open())
	{
		parsed = false;
		//std::cerr << "Error opening config file '" << filename_ << "'." << std::endl;
		return;
	}

	std::string line;
	bool success = true;
	std::size_t commentLoc, sectionHeadStart, sectionHeadEnd;
	std::string section = "";	//default section is blank
	parsed = true;	//unless there's a problem

	while (success && getline(configFile, line))
	{
		sectionHeadStart = line.find_first_of("[");
		if (sectionHeadStart != std::string::npos) {
			//new section found
			sectionHeadEnd = line.find_first_of("]");
			section = line.substr(sectionHeadStart + 1, sectionHeadEnd - 1);
		}
		else {
			commentLoc = line.find_first_of("#");
			success = assignStringValue(section, line.substr(0, commentLoc));
		}

	}

	if (!success)
	{
//		std::cerr << "Error parsing config file '" << filename_ << "' at line" << std::endl
//			<< ">>> " << line << std::endl;
	}

	parsed = success;
	configFile.close();
}


bool BlackflyConfigFile::assignStringValue(const std::string& section, std::string line)
{
	
	std::size_t nameStart = line.find_first_not_of(" ");

	if (line.length() == 0 || nameStart == std::string::npos)	//blank line
		return true;

	std::size_t equalsLoc = line.find_first_of("=");

	if (equalsLoc < 1 || equalsLoc == std::string::npos || equalsLoc == nameStart)
		return false;

	std::size_t nameEnd = line.find_last_not_of(" ", equalsLoc - 1);

	if (equalsLoc == line.length() - 1)
		line.append("");

	std::size_t valueStart = line.find_first_not_of(" ", equalsLoc + 1);

	if (valueStart == std::string::npos)
		valueStart = equalsLoc + 1;

	return setStringValue(section, line.substr(nameStart, nameEnd + 1), line.substr(valueStart));
}


bool BlackflyConfigFile::setStringValue(const std::string& section, const std::string& name, const std::string& value)
{
	configData[section].section = section;
	configData[section].parameters[trim(name)] = trim(value);
//	parameters[trim(name)] = trim(value);

	return true;
}

bool BlackflyConfigFile::getStringValue(const std::string& name, std::string &value) const
{
	return getStringValue("", name, value);
}

bool BlackflyConfigFile::getStringValue(const std::string& section, const std::string& name, std::string &value) const
{
	if (!parsed)
		return false;
	
	auto sectionData = configData.find(section);
	
	if (sectionData == configData.end()) {
		return false;
	}

	auto param = sectionData->second.parameters.find(name);

	if (param == sectionData->second.parameters.end()) {
		return false;
	}

	value = param->second;
	return true;
}


std::string BlackflyConfigFile::trim(const std::string& input) const
{
	if (input.length() == 0) {
		return input;
	}

	std::string white = " \n\r\t";

	std::size_t start = input.find_first_not_of(white);
	std::size_t end = input.find_last_not_of(white);

	if (start == std::string::npos || end == std::string::npos) {
		return input;
	}

	return input.substr(start, end + 1);
}

