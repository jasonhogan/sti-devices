
#ifndef PICOSCOPE5000_H
#define PICOSCOPE5000_H

#include <stdint.h>
#include <mutex>
#include <condition_variable>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <fstream>


struct ChannelSettings;
struct TriggerConfig;


class PicoScope5000
{
public:

	PicoScope5000();
	~PicoScope5000();

	double getTimeInterval();
	void setTimeInterval(double time_ns);

	bool setResolution(const std::string& resolution);
	bool getResolution(std::string& resolution);
	void getAllowedResolutions(std::vector<std::string>& resolutions);

	bool enableChannel(unsigned channel, bool enable);
	bool getChannelEnableStatus(unsigned channel);
	bool enableDigitalPort(unsigned digitalPort, bool enable);

	bool setRange(unsigned channel, const std::string& range);
	std::string getRange(unsigned channel);
	void getAllowedRanges(std::vector<std::string>& ranges);

	bool setCoupling(unsigned channel, const std::string& coupling);
	std::string getCoupling(unsigned channel);
	void getAllowedCouplings(std::vector<std::string>& couplings);

	bool setTriggerChannel(const std::string& trigger);
	std::string getTriggerChannel();
	void getAllowedTriggerChannels(std::vector<std::string>& channels);

	bool setDigitalTriggerDirection(const std::string& direction);
	std::string getDigitalTriggerDirection();
	void getAllowedDigitalTriggerDirections(std::vector<std::string>& directions);

	bool setAnalogTriggerDirection(const std::string& direction);
	std::string getAnalogTriggerDirection();
	void getAllowedAnalogTriggerDirections(std::vector<std::string>& directions);

	int getTriggerThresholdUpper();
	bool setTriggerThresholdUpper(double upper_mv);

	int getTriggerThresholdLower();
	bool setTriggerThresholdLower(double lower_mv);

	void setupScopeBuffers(int32_t pre_trigger_samples, int32_t post_trigger_samples);

	void setupTrigger();

	void runBlock();
	void cancelRun();
	void waitForRunComplete();
	bool isRunning();

	void writeDataToFile(std::ofstream& file);

	void runBlockComplete(bool success);	//used by callback -- needs to be public so global callback can access

private:

	bool _getChannelEnableStatus(unsigned channel);
	bool _enableChannel(unsigned channel, bool enable);

	uint32_t getMinimumTimebase();
	float calculateTimeInterval(uint32_t timebase);
	double getMinimumTimeInterval();
	bool calculateMinimumTimebase(uint32_t& timebase, double& timeInterval_ns);

	static int32_t adc_to_mv(int32_t raw, int32_t rangeIndex, int16_t maxADCValue);
	static int16_t mv_to_adc(int16_t mv, int16_t rangeIndex, int16_t maxADCValue);

	void openUnit();
	void closeUnit();

	bool isOpen();

	bool getUnitInfo(unsigned infoIndex, std::string& result);
	bool getChannelCount(int& analog_channel_count, int& digital_port_count);
	bool updateChannel(unsigned channel);

	bool getBlockData();

	bool getUnitInformation(int16_t& handle, int32_t& channelCount);

	bool rangeCheck(unsigned channelIndex, double value);

	template<class T>
	std::string reverseLookup(const std::map<std::string, T>& stringMap, const T& value)
	{
		//reverse lookup
		for (auto& e : stringMap) {
			if (e.second == value) {
				return e.first;
			}
		}
		return "";	//error
	}

	int analogChannelCount;
	int digitalPortCount;

	std::vector<std::unique_ptr<ChannelSettings>> channelSettings;
	std::vector<bool> digitalPortEnabled;

	std::vector<std::unique_ptr<int16_t[]>> buffers;

	std::unique_ptr<TriggerConfig> triggerConfig;

	int32_t preTriggerSamples;
	int32_t postTriggerSamples;
	bool running;
	bool dataAvailable;

	int16_t lowerTriggerThreshold;
	int16_t upperTriggerThreshold;

	double _timeInterval_ns;
	uint32_t _timeBase;


	int16_t maxADCValue;
	int16_t minADCValue;

	int16_t handle;
	bool unitOpen;

	mutable std::mutex picoMutex;
	mutable std::condition_variable runCondition;

};


#endif
