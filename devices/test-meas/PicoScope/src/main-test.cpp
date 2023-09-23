
#include "ps5000aApi.h"
#define QUAD_SCOPE		4
#define DUAL_SCOPE		2

#include <memory>
#include <vector>

#include <iostream>
//#include <stdio.h>
//#include <string.h>

#include <sstream>



struct ChannelSettings
{
    PS5000A_COUPLING coupling;
    PS5000A_RANGE range;
    bool enabled;
    float analogueOffset;

};


// Display unit information and determine if it is a 4-channel model
bool getUnitInformation(int16_t& handle, int32_t& channelCount)
{
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


int main(int argc, char* argv[])
{
	PICO_STATUS status = PICO_OK;
	int16_t handle = 0;

    std::cout << "PicoScope 5000 Series (ps5000a) Driver MSO Block Capture Example Program\n" << std::endl;

	// Establish connection to device
	// ------------------------------

	// Open the connection to the device
	status = ps5000aOpenUnit(&handle, NULL, PS5000A_DR_8BIT);

    // Handle power status codes
    if (status != PICO_OK) {
        if (status == PICO_POWER_SUPPLY_NOT_CONNECTED || PICO_USB3_0_DEVICE_NON_USB3_0_PORT) {
            status = ps5000aChangePowerSource(handle, status);
        }
        else {
            std::cout << "No device found - status code " << status << std::endl;
            return -1;
        }
    }

    int channelCount = 0;
    if (!getUnitInformation(handle, channelCount)) {
        return -1;
    }

    // Channel setup
      // -------------

      // Setup analogue channels

    ChannelSettings channelSettings[PS5000A_MAX_CHANNELS];

    // Find the current power source - to determine if the power supply is connected for 4-channel models
    //
    // If the power supply is not connected on a 4-channel device, only channels A and B can be used. 
    // MSO digital ports will still be available.

    int numAvailableChannels = channelCount;

    if (channelCount == QUAD_SCOPE) {
        status = ps5000aCurrentPowerSource(handle);

        if (status == PICO_POWER_SUPPLY_NOT_CONNECTED) {
            numAvailableChannels = DUAL_SCOPE;
        }
    }

    for (int ch = 0; ch < numAvailableChannels; ++ch) {
        channelSettings[ch].enabled = true;
        channelSettings[ch].coupling = PS5000A_DC;
        channelSettings[ch].range = PS5000A_5V;
        channelSettings[ch].analogueOffset = 0.0f;

        status = ps5000aSetChannel(handle, (PS5000A_CHANNEL)ch, (channelSettings[ch].enabled ? 1 : 0), channelSettings[ch].coupling,
            channelSettings[ch].range, channelSettings[ch].analogueOffset);

        if (status != PICO_OK)
        {
            fprintf(stderr, "ps5000aSetChannel (ch %d) ------ 0x%08lx \n", ch, status);
            return -1;
        }
    }


    std::unique_ptr<int16_t[]> buffer;
    buffer.reset(new int16_t[10]);

    std::vector< std::unique_ptr<int16_t[]> > buffers(PS5000A_MAX_CHANNELS);

    int32_t preTriggerSamples = 100;
    int32_t postTriggerSamples = 10000;
    int32_t totalSamples = preTriggerSamples + postTriggerSamples;  

    PS5000A_RATIO_MODE ratioMode = PS5000A_RATIO_MODE_NONE;

    // Set buffers for the analogue channels
    for (int ch = 0; ch < numAvailableChannels; ch++) {
        if (channelSettings[ch].enabled) {
            //buffers.at(ch) = (int16_t*)calloc(totalSamples, sizeof(int16_t));

            //buffers.at(ch).reset(new int16_t[totalSamples]);
            buffers.at(ch) = std::make_unique<int16_t[]>(totalSamples);

            status = ps5000aSetDataBuffer(handle, (PS5000A_CHANNEL)ch, buffers.at(ch).get(), totalSamples, 0, ratioMode);

            if (status != PICO_OK) {
                fprintf(stderr, "ps5000aSetDataBuffer ------ 0x%08lx \n", status);
                return -1;
            }
        }
    }

	return 0;
}
