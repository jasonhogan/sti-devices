
#include "PicoScope5000.h"

#include "ps5000aApi.h"
#define QUAD_SCOPE		4
#define DUAL_SCOPE		2

#include "PicoScope5000LookupMaps.h"

#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <math.h> 
#include <algorithm>

struct ChannelSettings
{
	ChannelSettings(PS5000A_COUPLING coupling, PS5000A_RANGE range, bool enabled, float analogOffset)
	: coupling(coupling), range(range), enabled(enabled), analogOffset(analogOffset) {}

	enPS5000AChannel channel;

	PS5000A_COUPLING coupling;
	PS5000A_RANGE range;
	bool enabled;
	float analogOffset;
};


struct TriggerConfig
{
	bool usingAnalogChannel;
	
	unsigned analogChannelIndex;
	unsigned digitalPortIndex;

	enPS5000AChannel channel;	//digital port number or analog channel number

	enPS5000ADigitalChannel digitalChannel;	//digitial channel number

	tPS5000ADirection analogDirection;
	tPS5000ADigitalChannelDirections digitalDirection;

};


uint16_t inputRanges[PS5000A_MAX_RANGES] = {
	10,
	20,
	50,
	100,
	200,
	500,
	1000,
	2000,
	5000,
	10000,
	20000,
	50000 };

enPS5000AChannel getPS5000aChannel(unsigned channel)
{
	enPS5000AChannel ch;

	switch (channel) {
	case 0:
		ch = PS5000A_CHANNEL_A;
		break;
	case 1:
		ch = PS5000A_CHANNEL_B;
		break;
	case 2:
		ch = PS5000A_CHANNEL_C;
		break;
	case 3:
		ch = PS5000A_CHANNEL_D;
		break;
	}

	return ch;
}


unsigned getChannelIndex(enPS5000AChannel channel)
{
	unsigned ch;

	switch (channel) {
	case PS5000A_CHANNEL_A:
		ch = 0;
		break;
	case PS5000A_CHANNEL_B:
		ch = 1;
		break;
	case PS5000A_CHANNEL_C:
		ch = 2;
		break;
	case PS5000A_CHANNEL_D:
		ch = 3;
		break;
	}

	return ch;
}

enPS5000AChannel digitalChannelFromPort(unsigned digitalPort)
{
	enPS5000AChannel channel;

	switch (digitalPort) {
	case 0:
		channel = PS5000A_DIGITAL_PORT0;
		break;
	case 1:
		channel = PS5000A_DIGITAL_PORT1;
		break;
	case 2:
		channel = PS5000A_DIGITAL_PORT2;
		break;
	case 3:
		channel = PS5000A_DIGITAL_PORT3;
		break;
	default:
		channel = PS5000A_DIGITAL_PORT0;
		break;
	}
	return channel;
}


//global callback function
void PREF4 callBackBlock(int16_t handle, PICO_STATUS status, void* pParameter)
{
	PicoScope5000* self = (PicoScope5000*) pParameter;	//this pointer to calling class instance

	bool success = false;

	if (status != PICO_CANCELLED) {
		success = true;
	}

	self->runBlockComplete(success);
}


//////////// PicoScope5000 ///////////////


PicoScope5000::PicoScope5000()
: handle(0), unitOpen(false), running(false), dataAvailable(false), buffers(2)
{
	openUnit();
	
	if (getChannelCount(analogChannelCount, digitalPortCount)) {
		
		channelSettings.clear();

		for (int i = 0; i < analogChannelCount; ++i) {

			auto chSetting = std::make_unique<ChannelSettings>(PS5000A_DC, PS5000A_5V, false, 0.0f);
			chSetting->channel = getPS5000aChannel(i);

			channelSettings.push_back(std::move(chSetting));

			//std::unique_ptr<int16_t[]> buffer;
			//buffer.reset(new int16_t[1]);
			//buffers.push_back(std::move(buffer));
		}

		digitalPortEnabled.clear();

		for (int i = 0; i < digitalPortCount; ++i) {
			digitalPortEnabled.push_back(false);
		}
	}

	// Find max ADC value
	maxADCValue = 0;
	PICO_STATUS status = ps5000aMaximumValue(handle, &maxADCValue);

	triggerConfig = std::make_unique<TriggerConfig>();
	triggerConfig->analogDirection.mode = PS5000A_LEVEL;

	setTimeInterval(8);		//ns
}

PicoScope5000::~PicoScope5000()
{
	closeUnit();
}

bool PicoScope5000::isOpen()
{
	return unitOpen;
}

void PicoScope5000::openUnit()
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	PICO_STATUS status = PICO_OK;

//	std::cout << "PicoScope 5000 Series (ps5000a)" << std::endl;

	// Establish connection to device
	// ------------------------------

	// Open the connection to the device
	status = ps5000aOpenUnit(&handle, NULL, PS5000A_DR_8BIT);

	// Handle power status codes
	if (status != PICO_OK) {
		if (status == PICO_POWER_SUPPLY_NOT_CONNECTED || PICO_USB3_0_DEVICE_NON_USB3_0_PORT) {
			status = ps5000aChangePowerSource(handle, status);
			unitOpen = (status == PICO_OK);
		}
		else {
			unitOpen = false;
			std::cout << "No device found - status code " << status << std::endl;
		}
	}
	else {
		unitOpen = true;
	}
}

void PicoScope5000::closeUnit()
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	if (unitOpen) {
		PICO_STATUS status = PICO_OK;
		status = ps5000aCloseUnit(handle);
	}
}


bool PicoScope5000::setResolution(const std::string& resolution)
{
	auto it = resolutionLookup.find(resolution);

	if (it == resolutionLookup.end()) return false;

	PICO_STATUS status = ps5000aSetDeviceResolution(handle, it->second);

	return (status == PICO_OK);
}


bool PicoScope5000::getResolution(std::string& resolution)
{
	enPS5000ADeviceResolution currentResolution;

	PICO_STATUS status = ps5000aGetDeviceResolution(handle, &currentResolution);

	if (status != PICO_OK) return false;

	resolution = reverseLookup(resolutionLookup, currentResolution);
	
	return resolution != "";
}

void PicoScope5000::getAllowedResolutions(std::vector<std::string>& resolutions)
{
	for (auto& res : resolutionLookup) {
		resolutions.push_back(res.first);
	}
}


bool PicoScope5000::getChannelCount(int& analog_channel_count, int& digital_port_count)
{
	std::string info;
	
	if (!getUnitInfo(PICO_VARIANT_INFO, info)) {
		return false;
	}

	analog_channel_count = std::stoi(info.substr(1, 1));

	auto findMSO = info.find_first_of("MSO");

	if (findMSO != std::string::npos) {
		digital_port_count = 2;
	}
	else {
		digital_port_count = 0;
	}

	return true;
}


bool PicoScope5000::setRange(unsigned channel, const std::string& range)
{
	if (channel >= channelSettings.size() || channelSettings.at(channel) == 0) return false;

	auto it = rangeLookup.find(range);

	if (it == rangeLookup.end()) return false;

	channelSettings.at(channel)->range = it->second;

	return updateChannel(channel);
}

std::string PicoScope5000::getRange(unsigned channel)
{
	return reverseLookup(rangeLookup, channelSettings.at(channel)->range);
}

void PicoScope5000::getAllowedRanges(std::vector<std::string>& ranges)
{
	ranges.clear();

	for (auto& r : rangeLookup) {
		ranges.push_back(r.first);
	}
}

std::string PicoScope5000::getCoupling(unsigned channel)
{
	return reverseLookup(couplingLookup, channelSettings.at(channel)->coupling);
}


bool PicoScope5000::setCoupling(unsigned channel, const std::string& coupling)
{
	if (channel >= channelSettings.size() || channelSettings.at(channel) == 0) return false;

	auto it = couplingLookup.find(coupling);

	if (it == couplingLookup.end()) return false;

	channelSettings.at(channel)->coupling = it->second;

	return updateChannel(channel);
}

void PicoScope5000::getAllowedCouplings(std::vector<std::string>& couplings)
{
	couplings.clear();

	for (auto& c : couplingLookup) {
		couplings.push_back(c.first);
	}
}
bool PicoScope5000::enableChannel(unsigned channel, bool enable)
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	return _enableChannel(channel, enable);
}

bool PicoScope5000::_enableChannel(unsigned channel, bool enable)
{
	bool success = false;

	if (channel < channelSettings.size() && channelSettings.at(channel) != 0) {
		channelSettings.at(channel)->enabled = enable;
		success = updateChannel(channel);
	}
	return success;
}

bool PicoScope5000::getChannelEnableStatus(unsigned channel)
{
	std::unique_lock<std::mutex> picoLock(picoMutex);
	return _getChannelEnableStatus(channel);
}

bool PicoScope5000::_getChannelEnableStatus(unsigned channel)
{
	if (channel < channelSettings.size() && channelSettings.at(channel) != 0) {
		return channelSettings.at(channel)->enabled;
	}
	return false;
}

bool PicoScope5000::updateChannel(unsigned channel)
{
	enPS5000AChannel ch = getPS5000aChannel(channel);
	
	if (channel >= channelSettings.size()) return false;

	if (channelSettings.at(channel) == 0) return false;		//error, null ChannelSettings

	PICO_STATUS status = ps5000aSetChannel(handle, ch, 
		(channelSettings.at(channel)->enabled ? 1 : 0), channelSettings.at(channel)->coupling,
		channelSettings.at(channel)->range, channelSettings.at(channel)->analogOffset);

	bool success = (status == PICO_OK);

	//refresh min/max value
	ps5000aMaximumValue(handle, &maxADCValue);
	ps5000aMinimumValue(handle, &minADCValue);

	return success;
}

bool PicoScope5000::enableDigitalPort(unsigned digitalPort, bool enable)
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	int16_t logicLevel = (int16_t)((1.5 / 5) * maxADCValue);
	
	if (digitalPort >= static_cast<unsigned>(digitalPortCount)) return false;	//error, bad port number

	enPS5000AChannel port = digitalChannelFromPort(digitalPort);

	PICO_STATUS status = ps5000aSetDigitalPort(handle, port, (enable ? 1 : 0), logicLevel);

	return (status == PICO_OK);
}

void setDigitalTriggerChannel(std::unique_ptr<TriggerConfig>& triggerConfig, unsigned port, enPS5000ADigitalChannel digitalChannel)
{
	triggerConfig->usingAnalogChannel = false;
	triggerConfig->digitalPortIndex = port;
	triggerConfig->channel = digitalChannelFromPort(port);
	triggerConfig->digitalChannel = digitalChannel;
	triggerConfig->digitalDirection.channel = digitalChannel;
}


std::string PicoScope5000::getTriggerChannel()
{
	unsigned analogChannelIndex = triggerConfig->analogChannelIndex;

	if (triggerConfig->usingAnalogChannel) {
		return reverseLookup(analogChannelLookup, triggerConfig->channel);
	}
	else if (triggerConfig->channel == PS5000A_DIGITAL_PORT0) {
		return reverseLookup(digitalChannelLookup_0, triggerConfig->digitalChannel);
	}
	else if (triggerConfig->channel == PS5000A_DIGITAL_PORT1) {
		return reverseLookup(digitalChannelLookup_1, triggerConfig->digitalChannel);
	}
	else if (triggerConfig->channel == PS5000A_DIGITAL_PORT2) {
		return reverseLookup(digitalChannelLookup_2, triggerConfig->digitalChannel);
	}
	else if (triggerConfig->channel == PS5000A_DIGITAL_PORT3) {
		return reverseLookup(digitalChannelLookup_3, triggerConfig->digitalChannel);
	}
	return "";	//error
}


bool PicoScope5000::setTriggerChannel(const std::string& trigger)
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	auto it_a = analogChannelLookup.find(trigger);
	if (it_a != analogChannelLookup.end()) {
		triggerConfig->usingAnalogChannel = true;
		triggerConfig->analogChannelIndex = getChannelIndex(it_a->second);
		triggerConfig->channel = it_a->second;
		return true;
	}
	
	auto it_d0 = digitalChannelLookup_0.find(trigger);
	if (digitalPortCount > 0 && it_d0 != digitalChannelLookup_0.end()) {
		setDigitalTriggerChannel(triggerConfig, 0, it_d0->second);
		return true;
	}
	
	auto it_d1 = digitalChannelLookup_1.find(trigger);
	if (digitalPortCount > 1 && it_d1 != digitalChannelLookup_1.end()) {
		setDigitalTriggerChannel(triggerConfig, 1, it_d1->second);
		return true;
	}

	auto it_d2 = digitalChannelLookup_2.find(trigger);
	if (digitalPortCount > 2 && it_d2 != digitalChannelLookup_2.end()) {
		setDigitalTriggerChannel(triggerConfig, 2, it_d2->second);
		return true;
	}

	auto it_d3 = digitalChannelLookup_3.find(trigger);
	if (digitalPortCount > 3 && it_d3 != digitalChannelLookup_3.end()) {
		setDigitalTriggerChannel(triggerConfig, 3, it_d3->second);
		return true;
	}

	return false;
}

void PicoScope5000::getAllowedTriggerChannels(std::vector<std::string>& channels)
{
	channels.clear();

	for (auto& ch : analogChannelLookup) {
		if (ch.second < analogChannelCount) {
			channels.push_back(ch.first);
		}
	}

	if (digitalPortCount > 0) {
		for (auto& ch : digitalChannelLookup_0) {
			channels.push_back(ch.first);
		}
	}
	if (digitalPortCount > 1) {
		for (auto& ch : digitalChannelLookup_1) {
			channels.push_back(ch.first);
		}
	}
	if (digitalPortCount > 2) {
		for (auto& ch : digitalChannelLookup_2) {
			channels.push_back(ch.first);
		}
	}
	if (digitalPortCount > 3) {
		for (auto& ch : digitalChannelLookup_3) {
			channels.push_back(ch.first);
		}
	}
}

std::string PicoScope5000::getDigitalTriggerDirection()
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	return reverseLookup(digitalDirectionLookup, triggerConfig->digitalDirection.direction);
}

bool PicoScope5000::setDigitalTriggerDirection(const std::string& direction)
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	auto it = digitalDirectionLookup.find(direction);
	if (it != digitalDirectionLookup.end()) {
		triggerConfig->digitalDirection.direction = it->second;
		return true;
	}
	return false;
}

void PicoScope5000::getAllowedDigitalTriggerDirections(std::vector<std::string>& directions)
{
	directions.clear();

	for (auto& d : digitalDirectionLookup) {
		directions.push_back(d.first);
	}
}

std::string PicoScope5000::getAnalogTriggerDirection()
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	return reverseLookup(thresholdDirectionLookup, triggerConfig->analogDirection.direction);
}

bool PicoScope5000::setAnalogTriggerDirection(const std::string& direction)
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	auto it = thresholdDirectionLookup.find(direction);
	if (it != thresholdDirectionLookup.end()) {
		triggerConfig->analogDirection.direction = it->second;
		return true;
	}
	return false;
}

void PicoScope5000::getAllowedAnalogTriggerDirections(std::vector<std::string>& directions)
{
	directions.clear();

	for (auto& d : thresholdDirectionLookup) {
		directions.push_back(d.first);
	}
}


void PicoScope5000::setupScopeBuffers(int32_t pre_trigger_samples, int32_t post_trigger_samples)
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	preTriggerSamples = pre_trigger_samples;
	postTriggerSamples = post_trigger_samples;

//	int32_t preTriggerSamples = 100;
//	int32_t postTriggerSamples = 10000;
	int32_t totalSamples = preTriggerSamples + postTriggerSamples;

	uint32_t downSampleRatio = 1;
	PS5000A_RATIO_MODE ratioMode = PS5000A_RATIO_MODE_NONE;

	for (int ch = 0; ch < analogChannelCount; ++ch) {
		if (channelSettings.at(ch) != 0 && channelSettings.at(ch)->enabled) {
			//std::unique_ptr<int16_t[]> buffer;
			//buffer.reset(new int16_t[totalSamples]);
			//buffers.at(ch) = std::move(buffer);

			buffers.at(ch).reset(new int16_t[totalSamples]);

			PICO_STATUS status = ps5000aSetDataBuffer(handle, (PS5000A_CHANNEL)ch, buffers.at(ch).get(), totalSamples, 0, ratioMode);
		}
	}

}
uint32_t PicoScope5000::getMinimumTimebase()
{
	uint32_t timebase = 0;
	double timeInterval = 0;

	calculateMinimumTimebase(timebase, timeInterval);

	return timebase;
}

double PicoScope5000::getMinimumTimeInterval()
{
	uint32_t timebase = 0;
	double timeInterval = 0;

	calculateMinimumTimebase(timebase, timeInterval);

	return timeInterval;
}

bool PicoScope5000::calculateMinimumTimebase(uint32_t& timebase, double& timeInterval_ns)
{
	PICO_STATUS status = PICO_OK;

	bool first = true;
	enPS5000AChannelFlags enabledChannelOrPortFlags;

	//bitwise OR active analog channels
	for (auto& cs : channelSettings) {
		auto it = channelFlagLookup.find(cs->channel);

		if (cs->enabled && it != channelFlagLookup.end()) {
			if (first) {
				enabledChannelOrPortFlags = it->second;
				first = false;
			}
			else {
				enabledChannelOrPortFlags =
					static_cast<enPS5000AChannelFlags>(enabledChannelOrPortFlags | it->second);
			}
		}
	}

	//bitwise OR active digital ports
	for (unsigned port = 0; port < digitalPortEnabled.size(); ++port) {
		auto it = channelFlagLookup.find(digitalChannelFromPort(port));
		if (digitalPortEnabled.at(port) && it != channelFlagLookup.end()) {
			if (first) {
				enabledChannelOrPortFlags = it->second;
				first = false;
			}
			else {
				enabledChannelOrPortFlags =
					static_cast<enPS5000AChannelFlags>(enabledChannelOrPortFlags | it->second);
			}
		}
	}

	enPS5000ADeviceResolution resolution;
	std::string res;
	if (getResolution(res)) {
		auto it = resolutionLookup.find(res);
		if (it != resolutionLookup.end()) {
			resolution = it->second;
		}
	}

	double timeInterval_s;
	status = ps5000aGetMinimumTimebaseStateless(handle, enabledChannelOrPortFlags, &timebase, &timeInterval_s, resolution);

	timeInterval_ns = timeInterval_s * (1e9);

	return (status == PICO_OK);

}

float PicoScope5000::calculateTimeInterval(uint32_t timebase)
{
	PICO_STATUS status = PICO_OK;

	int32_t maxSamples = 0;
	float timeInterval = 0;
	int32_t totalSamples = preTriggerSamples + postTriggerSamples;
	int32_t timeIndisposed = 0;

	status = ps5000aGetTimebase2(handle, timebase, totalSamples, &timeInterval, &maxSamples, 0);

	return timeInterval;	//in ns
}

double PicoScope5000::getTimeInterval()
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	return _timeInterval_ns;
}

void PicoScope5000::setTimeInterval(double time_ns)
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	double targetInterval = time_ns;
	uint32_t targetTimebase;

	double minInterval = getMinimumTimeInterval();

	if (_getChannelEnableStatus(0) && _getChannelEnableStatus(1)) {
		//two channels enable, min time is 2 ns
		if (minInterval < 2) {
			minInterval = 2.0;
		}
	}

	if (targetInterval < minInterval) {
		targetInterval = minInterval;
	}

	//get nearest timebase int for targetInterval
	std::string resolution;
	getResolution(resolution);
	auto res = resolutionLookup.find(resolution);
	
	uint32_t maxTimebase;

	switch (res->second) {
	case PS5000A_DR_8BIT:
		maxTimebase = (0xFFFFFFFF);	//2^32 - 1
		if (targetInterval < 5) {
			targetTimebase = static_cast<uint32_t>(round(log2(targetInterval)));
		}
		else {
			targetTimebase = std::min(static_cast<uint32_t>(round( (targetInterval / 8) + 2 )), maxTimebase);
		}
		break;
	case PS5000A_DR_12BIT:
		maxTimebase = (0xFFFFFFFF) - 1;	//2^32 - 2
		if (targetInterval < 12) {
			targetTimebase = static_cast<uint32_t>(round(log2(targetInterval / 2) + 1));
		}
		else {
			targetTimebase = std::min(static_cast<uint32_t>(round((targetInterval / 16) + 3)), maxTimebase);
		}
		break;
	case PS5000A_DR_14BIT:
		maxTimebase = (0xFFFFFFFF);	//2^32 - 1
		targetTimebase = std::min(static_cast<uint32_t>(round((targetInterval / 8) + 2)), maxTimebase);
		break;
	case PS5000A_DR_15BIT:
		maxTimebase = (0xFFFFFFFF);	//2^32 - 1
		targetTimebase = std::min(static_cast<uint32_t>(round((targetInterval / 8) + 2)), maxTimebase);
		break;
	case PS5000A_DR_16BIT:		
		maxTimebase = (0xFFFFFFFF) - 1;	//2^32 - 2
		targetTimebase = std::min(static_cast<uint32_t>(round((targetInterval / 16) + 3)), maxTimebase);
		break;
	}

	uint32_t minTimeBase = getMinimumTimebase();

	if (targetTimebase < minTimeBase) {
		targetTimebase = minTimeBase;
	}

	//ps5000aNearestSampleIntervalStateless(handle, enabledChannelOrPortFlags, time_ns, res->second, 0, targetTimebase, _timeInterval_ns);

	_timeBase = targetTimebase;
	_timeInterval_ns = static_cast<double>( calculateTimeInterval(_timeBase) );

}

void PicoScope5000::runBlock()
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	PICO_STATUS status = PICO_OK;

	float timeInterval = 0;
	int32_t maxSamples = 0;
	uint32_t timebase = _timeBase;
	int32_t totalSamples = preTriggerSamples + postTriggerSamples;
	int32_t timeIndisposed = 0;

	status = ps5000aGetTimebase2(handle, timebase, totalSamples, &timeInterval, &maxSamples, 0);

	running = true;

	status = ps5000aRunBlock(handle, preTriggerSamples, postTriggerSamples, timebase, &timeIndisposed, 0, callBackBlock, this);
}

void PicoScope5000::runBlockComplete(bool success)
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	running = false;
	dataAvailable = success;

	runCondition.notify_all();
}

bool PicoScope5000::isRunning()
{
	std::unique_lock<std::mutex> picoLock(picoMutex);
	return running;
}

void PicoScope5000::cancelRun()
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	PICO_STATUS status = ps5000aStop(handle);
}

void PicoScope5000::waitForRunComplete()
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	while (running) {
		runCondition.wait(picoLock);
	}
}

bool PicoScope5000::getBlockData()
{
//	std::unique_lock<std::mutex> picoLock(picoMutex);

	if (!dataAvailable) return false;

	PICO_STATUS status = PICO_OK;
	int32_t totalSamples = preTriggerSamples + postTriggerSamples;
	uint32_t downSampleRatio = 1;
	PS5000A_RATIO_MODE ratioMode = PS5000A_RATIO_MODE_NONE;
	int16_t	g_overflow = 0;

	status = ps5000aGetValues(handle, 0, (uint32_t*)&totalSamples, downSampleRatio, ratioMode, 0, &g_overflow);

	return (status == PICO_OK);
}

void PicoScope5000::writeDataToFile(std::ofstream& file)
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	if (!getBlockData()) return;
	
	int32_t totalSamples = preTriggerSamples + postTriggerSamples;

	std::vector<int> enabledChannels;

	for (int ch = 0; ch < analogChannelCount; ch++) {
		if (channelSettings.at(ch)->enabled) {
			enabledChannels.push_back(ch);
		}
	}
	
	for (int i = 0; i < totalSamples; i++) {
		for (int ch = 0; ch < enabledChannels.size(); ++ch) {
			if (ch == 1) {
				file << ",";
			}
			file << adc_to_mv(
				buffers.at( enabledChannels.at(ch) )[i], 
				channelSettings.at( enabledChannels.at(ch) )->range, 
				maxADCValue);
		}
		file << "\n";
	}
}

int PicoScope5000::getTriggerThresholdUpper()
{
	return upperTriggerThreshold;
}

bool PicoScope5000::rangeCheck(unsigned channelIndex, double value)
{
	auto valueADC = mv_to_adc(static_cast<int16_t>(value), channelSettings.at(channelIndex)->range, maxADCValue);

	return valueADC <= maxADCValue && valueADC >= minADCValue;
}

bool PicoScope5000::setTriggerThresholdUpper(double upper_mv)
{
	if (!rangeCheck(triggerConfig->analogChannelIndex, upper_mv)) return false;

	upperTriggerThreshold = mv_to_adc(static_cast<int16_t>(upper_mv),
		channelSettings.at(triggerConfig->analogChannelIndex)->range,
		maxADCValue);

	return true;
}

int PicoScope5000::getTriggerThresholdLower()
{
	return lowerTriggerThreshold;
}

bool PicoScope5000::setTriggerThresholdLower(double lower_mv)
{
	if (!rangeCheck(triggerConfig->analogChannelIndex, lower_mv)) return false;

	lowerTriggerThreshold = mv_to_adc(static_cast<int16_t>(lower_mv),
		channelSettings.at(triggerConfig->analogChannelIndex)->range,
		maxADCValue);
	return true;
}

void PicoScope5000::setupTrigger()
{
	std::unique_lock<std::mutex> picoLock(picoMutex);

	PICO_STATUS status = PICO_OK;


	tPS5000ACondition triggerCondition;
	triggerCondition.source = triggerConfig->channel;
	triggerCondition.condition = PS5000A_CONDITION_TRUE;

	//clear existing trigger
	status = ps5000aSetTriggerChannelConditionsV2(handle, &triggerCondition, 1, PS5000A_CLEAR);

	// Set trigger condition
	status = ps5000aSetTriggerChannelConditionsV2(handle, &triggerCondition, 1, (PS5000A_CONDITIONS_INFO)(PS5000A_CLEAR | PS5000A_ADD));

	
	//Analog channels only
	tPS5000ADirection analogDirection = triggerConfig->analogDirection;
	status = ps5000aSetTriggerChannelDirectionsV2(handle, &analogDirection, 1);

	tPS5000ATriggerChannelPropertiesV2 triggerChannelProperties;
	triggerChannelProperties.channel = triggerConfig->channel;
	triggerChannelProperties.thresholdLower = lowerTriggerThreshold;
	triggerChannelProperties.thresholdLowerHysteresis = 0;
	triggerChannelProperties.thresholdUpper = upperTriggerThreshold;
	triggerChannelProperties.thresholdUpperHysteresis = 0;

	status = ps5000aSetTriggerChannelPropertiesV2(handle, &triggerChannelProperties, 1, 0);

	//Digital channels
	tPS5000ADigitalChannelDirections digitalDirection = triggerConfig->digitalDirection;
	status = ps5000aSetTriggerDigitalPortProperties(handle, &digitalDirection, 1);
}
//
//void setupTriggerOld()
//{
//	// Trigger setup
//	// -------------
//
//	// Set up trigger on digital channel 15, falling edge, with auto trigger of 1 second
//
//	PICO_STATUS status = PICO_OK;
//
//	// Set the condition for the port to which the channel belongs
//	struct tPS5000ACondition digitalCondition;
//
//	digitalCondition.source = PS5000A_DIGITAL_PORT1;
//	digitalCondition.condition = PS5000A_CONDITION_TRUE;
//
//	status = ps5000aSetTriggerChannelConditionsV2(handle, &digitalCondition, 1, (PS5000A_CONDITIONS_INFO)(PS5000A_CLEAR | PS5000A_ADD));
//
//	if (status != PICO_OK)
//	{
//		fprintf(stderr, "ps5000aSetTriggerChannelConditionsV2 ------ 0x%08lx \n", status);
//		return -1;
//	}
//
//	// Set digital channel properties
//	struct tPS5000ADigitalChannelDirections digitalDirection;
//	digitalDirection.channel = PS5000A_DIGITAL_CHANNEL_15;
//	digitalDirection.direction = PS5000A_DIGITAL_DIRECTION_FALLING;
//
//	status = ps5000aSetTriggerDigitalPortProperties(handle, &digitalDirection, 1);
//
//	if (status != PICO_OK)
//	{
//		fprintf(stderr, "ps5000aSetTriggerDigitalPortProperties ------ 0x%08lx \n", status);
//		return -1;
//	}
//
//	status = ps5000aSetAutoTriggerMicroSeconds(handle, 1000000);
//
//}

/****************************************************************************
* adc_to_mv
*
* Convert an 16-bit ADC count into millivolts
****************************************************************************/
int32_t PicoScope5000::adc_to_mv(int32_t raw, int32_t rangeIndex, int16_t maxADCValue)
{
	return (raw * inputRanges[rangeIndex]) / maxADCValue;
}


/****************************************************************************
* mv_to_adc
*
* Convert a millivolt value into a 16-bit ADC count
*
*  (useful for setting trigger thresholds)
****************************************************************************/
int16_t PicoScope5000::mv_to_adc(int16_t mv, int16_t rangeIndex, int16_t maxADCValue)
{
	return (mv * maxADCValue) / inputRanges[rangeIndex];
}

bool PicoScope5000::getUnitInfo(unsigned infoIndex, std::string& result)
{
	if (infoIndex > 11) return false;

	PICO_STATUS status = PICO_OK;
	int16_t requiredSize = 0;
	int8_t	line[80];

	status = ps5000aGetUnitInfo(handle, line, 80, &requiredSize, infoIndex);

	if (status != PICO_OK) return false;

	std::stringstream lineS;
	lineS << line;
	result = lineS.str();

	return true;
}

bool PicoScope5000::getUnitInformation(int16_t& handle, int32_t& channelCount)
{
    // Display unit information and determine if it is a 4-channel model
    
    PICO_STATUS status = PICO_OK;

    int16_t requiredSize = 0;
    int8_t	line[80];
    int32_t digitalPortCount = 0;
    int32_t i = 0;

    int8_t description[11][25] = { "Driver Version",
                                    "USB Version",
                                    "Hardware Version",
                                    "Variant Info",
                                    "Serial",
                                    "Cal Date",
                                    "Kernel Version",
                                    "Digital HW Version",
                                    "Analogue HW Version",
                                    "Firmware 1",
                                    "Firmware 2" };

    printf("Device information:-\n\n");

    for (i = 0; i < 11; i++) {
        status = ps5000aGetUnitInfo(handle, line, 80, &requiredSize, i);

        // info = 3 - PICO_VARIANT_INFO
        if (i == PICO_VARIANT_INFO) {
            channelCount = (int16_t)line[1];
            channelCount = channelCount - 48; // Subtract ASCII 0 (48)

            std::cout << line << std::endl;

            std::stringstream lineS;
            lineS << line;

            auto findMSO = lineS.str().find_first_of("MSO");

            // Determine if the device is an MSO
            if (findMSO != std::string::npos) {
                //            if (strstr(line, "MSO") != NULL) {
                digitalPortCount = 2;
            }
            else {
                digitalPortCount = 0;
                fprintf(stderr, "This example is for PicoScope 5000 Series Mixed Signal Oscilloscopes.\n");
                printf("Exiting application...\n");
                return false;
            }
        }

        printf("%s: %s\n", description[i], line);
    }
    return true;
}
