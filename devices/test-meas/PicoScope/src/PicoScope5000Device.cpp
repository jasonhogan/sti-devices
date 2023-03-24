
#include "PicoScope5000Device.h"

#include <sstream>
#include <iomanip>
#include <fstream>

std::string trim(const std::string& str, const std::string& whitespace = " \t");
std::string listToString(const std::vector<std::string>& tokens);

PicoScope5000Device::PicoScope5000Device(ORBManager* orb_manager, const ConfigFile& configFile)
: STI_Device_Adapter(orb_manager, configFile)
{
	configFile.getParameter("Trigger partner name", trigger.name);
	configFile.getParameter("Trigger partner IP", trigger.ip);
	configFile.getParameter("Trigger partner module", trigger.module);
	configFile.getParameter("Trigger partner channel", trigger.channel);

	configFile.getParameter("Trigger Low Value", trigger.low);
	configFile.getParameter("Trigger High Value", trigger.high);
	configFile.getParameter("Trigger duration (ns)", trigger.duration_ns);
	configFile.getParameter("Trigger Reset Holdoff (ns)", trigger.resetHoldoff_ns);		//holdoff time before event when LOW is asserted

	configFile.getParameter("STI Data Base Directory", baseDirectory);

	picoScope.enableChannel(1, true);
	picoScope.enableChannel(2, true);

	picoScope.enableDigitalPort(0, true);
}

PicoScope5000Device::~PicoScope5000Device()
{
}

void PicoScope5000Device::defineChannels()
{
	addInputChannel(1, DataVector, ValueVector, "Ch A");
	addInputChannel(2, DataVector, ValueVector, "Ch B");
}

void PicoScope5000Device::definePartnerDevices()
{
	addPartnerDevice("External Trigger", trigger.ip, trigger.module, trigger.name);
	partnerDevice("External Trigger").enablePartnerEvents();
}

void PicoScope5000Device::defineAttributes()
{
	addAttribute("Generate Trigger Events", (externalTriggerEventsOn ? "True" : "False"), "True, False");

	addAttribute("Time Interval", picoScope.getTimeInterval());

	std::string resolution;
	std::vector<std::string> resolutions;
	picoScope.getResolution(resolution);
	picoScope.getAllowedResolutions(resolutions);
	addAttribute("Vertical Resolution", resolution, listToString(resolutions));

	addAttribute("Enable A", "True", "True, False");
	addAttribute("Enable B", "False", "True, False");

	std::vector<std::string> ranges;
	picoScope.getAllowedRanges(ranges);
	addAttribute("A Range", picoScope.getRange(0), listToString(ranges));
	addAttribute("B Range", picoScope.getRange(1), listToString(ranges));

	std::vector<std::string> couplings;
	picoScope.getAllowedCouplings(couplings);
	addAttribute("A Coupling", picoScope.getCoupling(0), listToString(couplings));
	addAttribute("B Coupling", picoScope.getCoupling(1), listToString(couplings));

	std::vector<std::string> channels;
	picoScope.getAllowedTriggerChannels(channels);
	addAttribute("Trigger", picoScope.getTriggerChannel(), listToString(channels));

	std::vector<std::string> directions;
	picoScope.getAllowedAnalogTriggerDirections(directions);
	addAttribute("Analog Trigger Direction", picoScope.getAnalogTriggerDirection(), listToString(directions));

	directions.clear();
	picoScope.getAllowedDigitalTriggerDirections(directions);
	addAttribute("Digital Trigger Direction", picoScope.getDigitalTriggerDirection(), listToString(directions));

	addAttribute("Trigger Upper Threshold", picoScope.getTriggerThresholdUpper());
	addAttribute("Trigger Lower Threshold", picoScope.getTriggerThresholdLower());
}

bool PicoScope5000Device::updateAttribute(std::string key, std::string value)
{
	bool success = false;

	if (key.compare("Generate Trigger Events") == 0) {
		externalTriggerEventsOn = value.compare("True") == 0;
		success = true;
	}

	if (key.compare("Time Interval") == 0) {
		std::vector<std::string> tokens;
		STI::Utils::splitString(value, " ", tokens);
		picoScope.setTimeInterval(
			convertTime_ns(tokens.at(0), tokens.size() > 1 ? trim(tokens.at(1)) : "ns")
			);
		success = true;
	}
	if (key.compare("Vertical Resolution") == 0) {
		success = picoScope.setResolution(value);
	}
	if (key.compare("Enable A") == 0) {
		success = picoScope.enableChannel(0, value.compare("True") == 0);
	}
	if (key.compare("Enable B") == 0) {
		success = picoScope.enableChannel(1, value.compare("True") == 0);
	}
	if (key.compare("A Range") == 0) {
		success = picoScope.setRange(0, value);
	}
	if (key.compare("B Range") == 0) {
		success = picoScope.setRange(1, value);
	}
	if (key.compare("A Coupling") == 0) {
		success = picoScope.setCoupling(0, value);
	}
	if (key.compare("B Coupling") == 0) {
		success = picoScope.setCoupling(1, value);
	}
	if (key.compare("Trigger") == 0) {
		success = picoScope.setTriggerChannel(value);
	}
	if (key.compare("Analog Trigger Direction") == 0) {
		success = picoScope.setAnalogTriggerDirection(value);
	}
	if (key.compare("Digital Trigger Direction") == 0) {
		success = picoScope.setDigitalTriggerDirection(value);
	}
	if (key.compare("Trigger Upper Threshold") == 0) {

		std::vector<std::string> tokens;
		STI::Utils::splitString(value, " ", tokens);
		success = picoScope.setTriggerThresholdUpper(
			convertValue_mV(tokens.at(0), tokens.size() > 1 ? trim(tokens.at(1)) : "mV")
		);
	}
	if (key.compare("Trigger Lower Threshold") == 0) {

		std::vector<std::string> tokens;
		STI::Utils::splitString(value, " ", tokens);
		success = picoScope.setTriggerThresholdLower(
			convertValue_mV(tokens.at(0), tokens.size() > 1 ? trim(tokens.at(1)) : "mV")
		);
	}

	return success;
}


void PicoScope5000Device::refreshAttributes()
{
	setAttribute("Generate Trigger Events", (externalTriggerEventsOn ? "True" : "False"));
	setAttribute("Time Interval", picoScope.getTimeInterval());

	std::string res;
	if (picoScope.getResolution(res)) {
		setAttribute("Vertical Resolution", res);
	}

	setAttribute("Enable A", picoScope.getChannelEnableStatus(0));
	setAttribute("Enable B", picoScope.getChannelEnableStatus(1));

	setAttribute("A Range", picoScope.getRange(0));
	setAttribute("B Range", picoScope.getRange(1));

	setAttribute("A Coupling", picoScope.getCoupling(0));
	setAttribute("B Coupling", picoScope.getCoupling(1));
	
	setAttribute("Trigger", picoScope.getTriggerChannel());
	setAttribute("Analog Trigger Direction", picoScope.getAnalogTriggerDirection());
	setAttribute("Digital Trigger Direction", picoScope.getDigitalTriggerDirection());
	setAttribute("Trigger Upper Threshold", picoScope.getTriggerThresholdUpper());
	setAttribute("Trigger Lower Threshold", picoScope.getTriggerThresholdLower());
}


std::string PicoScope5000Device::getDeviceHelp()
{
	std::stringstream help;

	help << "Syntax for the Value field of meas() command:" << std::endl;
	help << "(<# pre trigger samples>, <# post trigger samples>)" << std::endl;

	return help.str();
}

bool PicoScope5000Device::parseEventValue(const std::vector<RawEvent>& rawEvents, PicoScope5000DeviceEventValue& value, std::string& message)
{
//	if (rawEvents.size() != 1) {
//		message = "Only one scope trace event at a time is allowed.";
//		return false;
//	}

	for (const auto& e : rawEvents) {
		value.channels.push_back(e.channel());
	}

	if (! (rawEvents.at(0).getValueType() == MixedValue::Vector
		&& rawEvents.at(0).vectorValue().size() == 3) ) {
		message = "Value must be a tuple of the form (filename, preTriggerSamples, postTriggerSamples)";
		return false;
	}

	value.filename = rawEvents.at(0).vectorValue().at(0).getString();
	value.preTriggerSamples = rawEvents.at(0).vectorValue().at(1).getInt();
	value.postTriggerSamples = rawEvents.at(0).vectorValue().at(2).getInt();

	return true;
}


void PicoScope5000Device::parseDeviceEvents(const RawEventMap& eventsIn, SynchronousEventVector& eventsOut) throw(std::exception)
{
	double minimumEventTime = 1000;	//1 us
	double eventTime = minimumEventTime;	//from timing file
	double holdoff = 0 * 10000000;		//10 ms

	std::string filenamebase = "test";
	std::string filenameext = ".csv";
	std::string filename;

	std::string err_message;

	picoScope.enableChannel(0, false);
	picoScope.enableChannel(1, false);

	for (auto events = eventsIn.begin(); events != eventsIn.end(); events++)
	{
		if (events->first < minimumEventTime) {
			throw EventParsingException(events->second.at(0), "Minimum event time is " + STI::Utils::valueToString(minimumEventTime));
		}

		PicoScope5000DeviceEventValue value;
		if (!parseEventValue(events->second, value, err_message)) {
			throw EventParsingException(events->second.at(0), err_message);
		}

		eventTime = events->first - holdoff;	//time of the event requested in timing file

		for (auto& ch : value.channels) {
			picoScope.enableChannel(ch - 1, true);
		}

		filename = value.filename + filenameext;

		auto scopeEvt = std::make_unique<PicoScope5000DeviceEvent>(events->first, this, filename, value.preTriggerSamples, value.postTriggerSamples);
		
		for (auto& e : events->second) {
			scopeEvt->addMeasurement(e);		//register the measurement with the source RawEvent
		}

		eventsOut.push_back(scopeEvt.release());
	}
}



std::string PicoScope5000Device::generateDataDestinationDirectory()
{
	//	std::string baseDirectory = "C:\\Users\\Jason\\Code\\dev\\stidatatest\\";
	char dirSepChar = baseDirectory.back();

	std::stringstream path;

	std::time_t t = std::time(0);   // get time now
	struct tm * now = std::localtime(&t);

	path << baseDirectory << (now->tm_year + 1900) << dirSepChar
		<< (now->tm_mon + 1) << dirSepChar
		<< now->tm_mday << dirSepChar
		<< "data";

	return path.str();
}

std::string PicoScope5000Device::generateFileTimestamp()
{
	std::stringstream file;

	std::time_t t = std::time(0);   // get time now
	struct tm * now = std::localtime(&t);

	char sepChar = '_';

	//Example:  2018_7_3-19_03_55-test.tif
	file
		<< (now->tm_year + 1900) << sepChar
		<< std::setfill('0') << std::setw(2) << (now->tm_mon + 1) << sepChar
		<< std::setfill('0') << std::setw(2) << now->tm_mday
		<< std::setw(1) << "-"
		<< std::setfill('0') << std::setw(2) << now->tm_hour << sepChar
		<< std::setfill('0') << std::setw(2) << now->tm_min << sepChar
		<< std::setfill('0') << std::setw(2) << now->tm_sec
		<< std::setw(1) << "-";

	return file.str();
}


double PicoScope5000Device::convertTime_ns(const std::string& value, const std::string& units)
{
	double basetime;
	double scale = 1;

	STI::Utils::stringToValue(value, basetime);

	if (units == "s") {
		scale = 1e9;
	}
	else if (units == "ms") {
		scale = 1e6;
	}
	else if (units == "us") {
		scale = 1e3;
	}
	else if (units == "ns") {
		scale = 1;
	}

	return basetime * scale;
}

double PicoScope5000Device::convertValue_mV(const std::string& value, const std::string& units)
{
	double basevalue;
	double scale = 1;

	STI::Utils::stringToValue(value, basevalue);

	if (units == "V") {
		scale = 1e3;
	}
	else if (units == "mV") {
		scale = 1;
	}

	return basevalue * scale;
}

std::string listToString(const std::vector<std::string>& tokens)
{
	std::string str;
	bool first = true;

	for (auto& token : tokens) {
		if (!first) {
			str += ",";
		}
		first = false;
		str += token;
	}
	return str;
}

std::string trim(const std::string& str, const std::string& whitespace)
{
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return ""; // no content

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}



PicoScope5000Device::PicoScope5000DeviceEvent::PicoScope5000DeviceEvent(
	double time, PicoScope5000Device* scopeDevice, const std::string& filename, 
	unsigned preTriggerSamples, unsigned postTriggerSamples)
: SynchronousEventAdapter(time, scopeDevice), scopeDevice(scopeDevice), basefilename(filename),
preTriggerSamples(preTriggerSamples), postTriggerSamples(postTriggerSamples), runStarted(false), abort(false)
{
}

PicoScope5000Device::PicoScope5000DeviceEvent::~PicoScope5000DeviceEvent()
{
	std::unique_lock<std::mutex> runLock(runMutex);

	runStarted = true;
	abort = true;
	runCondition.notify_all();

	scopeDevice->picoScope.cancelRun();
}

void PicoScope5000Device::PicoScope5000DeviceEvent::loadEvent()
{
	runStarted = false;
	scopeDevice->picoScope.setupScopeBuffers(preTriggerSamples, postTriggerSamples);
	scopeDevice->picoScope.setupTrigger();
}

void PicoScope5000Device::PicoScope5000DeviceEvent::playEvent()
{
	std::unique_lock<std::mutex> runLock(runMutex);

	scopeDevice->picoScope.runBlock();

	runStarted = true;
	runCondition.notify_all();
}

void PicoScope5000Device::PicoScope5000DeviceEvent::collectMeasurementData()
{
	//std::string baseDir = scopeDevice->generateDataDestinationDirectory();

	//filename is specific to this shot, at the time of data collection
	std::string filename =
		scopeDevice->generateDataDestinationDirectory() + "\\" +
		scopeDevice->generateFileTimestamp() + basefilename;

	MixedData data;

	if (eventMeasurements.size() > 0) {
		data.addValue(scopeDevice->makeLabeledDataPair("Filename", filename));
		data.addValue(scopeDevice->makeLabeledDataPair("preTriggerSamples", static_cast<int>(preTriggerSamples)));
		data.addValue(scopeDevice->makeLabeledDataPair("postTriggerSamples", static_cast<int>(postTriggerSamples)));
		
		for (auto& e : eventMeasurements) {
			e->setData(data);
		}
//		eventMeasurements.at(0)->setData(data);

		std::ofstream dataFilestream(filename);

		scopeDevice->picoScope.writeDataToFile(dataFilestream);
	}

}

void PicoScope5000Device::PicoScope5000DeviceEvent::waitBeforeCollectData()
{
	std::unique_lock<std::mutex> runLock(runMutex);

	while (!runStarted) {
		runCondition.wait(runLock);
	}

	if (abort) return;

	scopeDevice->picoScope.waitForRunComplete();
}

void PicoScope5000Device::PicoScope5000DeviceEvent::setupEvent()
{
	scopeDevice->picoScope.cancelRun();
}