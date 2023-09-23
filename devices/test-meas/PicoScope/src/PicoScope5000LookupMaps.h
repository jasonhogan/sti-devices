
#ifndef PICOSCOPE5000LOOKUPMAPS_H
#define PICOSCOPE5000LOOKUPMAPS_H

#include <map>
#include <string>




static const std::map<std::string, enPS5000AThresholdDirection> thresholdDirectionLookup
{
	{ "Above",				PS5000A_ABOVE },		//using upper threshold
	{ "Below",				PS5000A_BELOW },		//using upper threshold
	{ "Rising",				PS5000A_RISING },		// using upper threshold
	{ "Falling",			PS5000A_FALLING },		// using upper threshold
	{ "Rising or Falling",	PS5000A_RISING_OR_FALLING }, // using both threshold
	{ "Above Lower",		PS5000A_ABOVE_LOWER },	// using lower threshold
	{ "Below Lower",		PS5000A_BELOW_LOWER },	// using lower threshold
	{ "Rising Lower",		PS5000A_RISING_LOWER }, // using lower threshold
	{ "Falling Lower",		PS5000A_FALLING_LOWER },// using lower threshold
	{ "None",				PS5000A_NONE }
};


static const std::map<enPS5000AChannel, enPS5000AChannelFlags> channelFlagLookup
{
	{ PS5000A_CHANNEL_A, PS5000A_CHANNEL_A_FLAGS },
	{ PS5000A_CHANNEL_B, PS5000A_CHANNEL_B_FLAGS },
	{ PS5000A_CHANNEL_C, PS5000A_CHANNEL_C_FLAGS },
	{ PS5000A_CHANNEL_D, PS5000A_CHANNEL_D_FLAGS },

	{ PS5000A_DIGITAL_PORT0, PS5000A_PORT0_FLAGS },
	{ PS5000A_DIGITAL_PORT1, PS5000A_PORT1_FLAGS },
	{ PS5000A_DIGITAL_PORT2, PS5000A_PORT2_FLAGS },
	{ PS5000A_DIGITAL_PORT3, PS5000A_PORT3_FLAGS }
};

static const std::map<std::string, enPS5000ADigitalDirection> digitalDirectionLookup
{
	{ "None", PS5000A_DIGITAL_DONT_CARE },
	{ "Low", PS5000A_DIGITAL_DIRECTION_LOW },
	{ "High", PS5000A_DIGITAL_DIRECTION_HIGH },
	{ "Rising", PS5000A_DIGITAL_DIRECTION_RISING },
	{ "Falling", PS5000A_DIGITAL_DIRECTION_FALLING },
	{ "Rising or Falling", PS5000A_DIGITAL_DIRECTION_RISING_OR_FALLING },
	{ "Max Direction", PS5000A_DIGITAL_MAX_DIRECTION }
};


static const std::map<std::string, enPS5000ADeviceResolution> resolutionLookup
{
	{ "8 bit", PS5000A_DR_8BIT },
	{ "12 bit", PS5000A_DR_12BIT },
	{ "14 bit", PS5000A_DR_14BIT },
	{ "15 bit", PS5000A_DR_15BIT },
	{ "16 bit", PS5000A_DR_16BIT }
};


static const std::map<std::string, enPS5000ARange> rangeLookup
{
	{ "10 mV",  PS5000A_10MV },
	{ "20 mV",  PS5000A_20MV },
	{ "50 mV",  PS5000A_50MV },
	{ "100 mV", PS5000A_100MV },
	{ "200 mV", PS5000A_200MV },
	{ "500 mV", PS5000A_500MV },
	{ "1 V",    PS5000A_1V },
	{ "2 V",    PS5000A_2V },
	{ "5 V",    PS5000A_5V },
	{ "10 V",   PS5000A_10V },
	{ "20 V",   PS5000A_20V },
	{ "50 V",   PS5000A_50V },
	{ "Max Ranges", PS5000A_MAX_RANGES }
};

static const std::map<std::string, enPS5000ACoupling> couplingLookup
{
	{ "AC", PS5000A_AC },
	{ "DC", PS5000A_DC }
};

static const std::map<std::string, enPS5000AChannel> analogChannelLookup
{
	{ "A0", PS5000A_CHANNEL_A },
	{ "A1", PS5000A_CHANNEL_B },
	{ "A2", PS5000A_CHANNEL_C },
	{ "A3", PS5000A_CHANNEL_D }
};

static const std::map<std::string, enPS5000ADigitalChannel> digitalChannelLookup_0
{
	{ "D0", PS5000A_DIGITAL_CHANNEL_0 },
	{ "D1", PS5000A_DIGITAL_CHANNEL_1 },
	{ "D2", PS5000A_DIGITAL_CHANNEL_2 },
	{ "D3", PS5000A_DIGITAL_CHANNEL_3 },
	{ "D4", PS5000A_DIGITAL_CHANNEL_4 },
	{ "D5", PS5000A_DIGITAL_CHANNEL_5 },
	{ "D6", PS5000A_DIGITAL_CHANNEL_6 },
	{ "D7", PS5000A_DIGITAL_CHANNEL_7 }
};
static const std::map<std::string, enPS5000ADigitalChannel> digitalChannelLookup_1
{
	{ "D8", PS5000A_DIGITAL_CHANNEL_8 },
	{ "D9", PS5000A_DIGITAL_CHANNEL_9 },
	{ "D10", PS5000A_DIGITAL_CHANNEL_10 },
	{ "D11", PS5000A_DIGITAL_CHANNEL_11 },
	{ "D12", PS5000A_DIGITAL_CHANNEL_12 },
	{ "D13", PS5000A_DIGITAL_CHANNEL_13 },
	{ "D14", PS5000A_DIGITAL_CHANNEL_14 },
	{ "D15", PS5000A_DIGITAL_CHANNEL_15 }
};
static const std::map<std::string, enPS5000ADigitalChannel> digitalChannelLookup_2
{
	{ "D16", PS5000A_DIGITAL_CHANNEL_16 },
	{ "D17", PS5000A_DIGITAL_CHANNEL_17 },
	{ "D18", PS5000A_DIGITAL_CHANNEL_18 },
	{ "D19", PS5000A_DIGITAL_CHANNEL_19 },
	{ "D20", PS5000A_DIGITAL_CHANNEL_20 },
	{ "D21", PS5000A_DIGITAL_CHANNEL_21 },
	{ "D22", PS5000A_DIGITAL_CHANNEL_22 },
	{ "D23", PS5000A_DIGITAL_CHANNEL_23 }
};
static const std::map<std::string, enPS5000ADigitalChannel> digitalChannelLookup_3
{
	{ "D24", PS5000A_DIGITAL_CHANNEL_24 },
	{ "D25", PS5000A_DIGITAL_CHANNEL_25 },
	{ "D26", PS5000A_DIGITAL_CHANNEL_26 },
	{ "D27", PS5000A_DIGITAL_CHANNEL_27 },
	{ "D28", PS5000A_DIGITAL_CHANNEL_28 },
	{ "D29", PS5000A_DIGITAL_CHANNEL_29 },
	{ "D30", PS5000A_DIGITAL_CHANNEL_30 },
	{ "D31", PS5000A_DIGITAL_CHANNEL_31 }

};



#endif
