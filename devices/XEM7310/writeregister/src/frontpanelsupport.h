#ifndef FRONTPANELSUPPORT_H
#define FRONTPANELSUPPORT_H

#include "okFrontPanelDLL.h"
#include <iostream>
#include <string>

okTDeviceInfo* g_devInfo;

// Exits on critical failure
// Returns true if there is no error
// Returns false on non-critical error
bool exitOnError(okCFrontPanel::ErrorCode error)
{
	switch (error) {
	case okCFrontPanel::DeviceNotOpen:
		printf("Device no longer available.\n");
		exit(EXIT_FAILURE);
	case okCFrontPanel::Failed:
		printf("Transfer failed.\n");
		exit(EXIT_FAILURE);
	case okCFrontPanel::Timeout:
		printf("   ERROR: Timeout\n");
		return false;
	case okCFrontPanel::TransferError:
		std::cout << "   ERROR: TransferError" << '\n';
		return false;
	case okCFrontPanel::UnsupportedFeature:
		printf("   ERROR: UnsupportedFeature\n");
		return false;
	case okCFrontPanel::DoneNotHigh:
	case okCFrontPanel::CommunicationError:
	case okCFrontPanel::InvalidBitstream:
	case okCFrontPanel::FileError:
	case okCFrontPanel::InvalidEndpoint:
	case okCFrontPanel::InvalidBlockSize:
	case okCFrontPanel::I2CRestrictedAddress:
	case okCFrontPanel::I2CBitError:
	case okCFrontPanel::I2CUnknownStatus:
	case okCFrontPanel::I2CNack:
	case okCFrontPanel::FIFOUnderflow:
	case okCFrontPanel::FIFOOverflow:
	case okCFrontPanel::DataAlignmentError:
	case okCFrontPanel::InvalidParameter:
		std::cout << "   ERROR: " << error << '\n';
		return false;
	default:
		return true;
	}
}



OpalKelly::FrontPanelPtr initializeFPGA(const std::string& config_filename)
{
	long long g_nMems, g_nMemSize;

	// Open the first XEM - try all board types.
	OpalKelly::FrontPanelPtr dev = OpalKelly::FrontPanelDevices().Open();
	if (!dev.get()) {
		printf("Device could not be opened.  Is one connected?\n");
		return(dev);
	}

	printf("Found a device: %s\n", dev->GetBoardModelString(dev->GetBoardModel()).c_str());
	g_devInfo = new okTDeviceInfo;
	dev->GetDeviceInfo(g_devInfo);

	// Set memory configuration
	switch (dev->GetBoardModel()) {
	case okCFrontPanel::brdXEM3005:
	case okCFrontPanel::brdXEM3010:
		g_nMemSize = 32 * 1024 * 1024;
		g_nMems = 1;
		break;
	case okCFrontPanel::brdXEM3050:
		g_nMemSize = 32 * 1024 * 1024;
		g_nMems = 2;
		break;
	case okCFrontPanel::brdXEM5010:
	case okCFrontPanel::brdXEM5010LX110:
		g_nMemSize = 128 * 1024 * 1024;
		g_nMems = 2;
		break;
	case okCFrontPanel::brdXEM6320LX130T:
		g_nMemSize = 512 * 1024 * 1024;
		g_nMems = 1;
		break;
	case okCFrontPanel::brdXEM7350K70T:
	case okCFrontPanel::brdXEM7350K160T:
	case okCFrontPanel::brdXEM7350K410T:
	case okCFrontPanel::brdXEM7010A200:
	case okCFrontPanel::brdXEM7010A50:
	case okCFrontPanel::brdXEM7305:
		g_nMemSize = 512 * 1024 * 1024;
		g_nMems = 1;
		break;
	case okCFrontPanel::brdXEM7360K160T:
	case okCFrontPanel::brdXEM7360K410T:
#ifdef ARCH_X64
		g_nMemSize = 4LL * 512LL * 1024LL * 1024LL;
#else // only use 1 GiB due to 32-bit memory allocation restrictions
		g_nMemSize = 2LL * 512LL * 1024LL * 1024LL;
#endif
		g_nMems = 1;
		break;
	case okCFrontPanel::brdXEM8350KU060:
	case okCFrontPanel::brdXEM8350KU085:
	case okCFrontPanel::brdXEM8350KU115:
#ifdef ARCH_X64
		g_nMemSize = 8LL * 512LL * 1024LL * 1024LL;
#else // only use 1 GiB due to 32-bit memory allocation restrictions
		g_nMemSize = 2LL * 512LL * 1024LL * 1024LL;
#endif
		g_nMems = 1;
		break;
	case okCFrontPanel::brdXEM7310A75:
	case okCFrontPanel::brdXEM7310A200:
	case okCFrontPanel::brdXEM7310MTA75:
	case okCFrontPanel::brdXEM7310MTA200:
	case okCFrontPanel::brdXEM7320A75T:
	case okCFrontPanel::brdXEM7320A200T:
		g_nMemSize = 2 * 512 * 1024 * 1024;
		g_nMems = 1;
		break;
	case okCFrontPanel::brdXEM6006LX9:
	case okCFrontPanel::brdXEM6006LX16:
	case okCFrontPanel::brdXEM6006LX25:
	case okCFrontPanel::brdXEM6010LX45:
	case okCFrontPanel::brdXEM6010LX150:
	case okCFrontPanel::brdXEM6310LX45:
	case okCFrontPanel::brdXEM6310LX150:
	case okCFrontPanel::brdXEM6310MTLX45T:
	case okCFrontPanel::brdXEM6110LX45:
	case okCFrontPanel::brdXEM6110LX150:
	case okCFrontPanel::brdXEM6110v2LX45:
	case okCFrontPanel::brdXEM6110v2LX150:
	case okCFrontPanel::brdZEM4310:
		g_nMemSize = 128 * 1024 * 1024;
		g_nMems = 1;
		break;
	case okCFrontPanel::brdZEM5305A2:
	case okCFrontPanel::brdZEM5310A4:
		g_nMemSize = 512 * 1024 * 1024;
		g_nMems = 1;
		break;
	default:
		printf("Unsupported device.\n");
		dev.reset();
		return(dev);
	}

	// Configure the PLL appropriately
	dev->LoadDefaultPLLConfiguration();

	// Get some general information about the XEM.
	printf("Device firmware version: %d.%d\n", dev->GetDeviceMajorVersion(), dev->GetDeviceMinorVersion());
	printf("Device serial number: %s\n", dev->GetSerialNumber().c_str());
	printf("Device ID string: %s\n", dev->GetDeviceID().c_str());

	// Download the configuration file.
	if (okCFrontPanel::NoError != dev->ConfigureFPGA(config_filename)) {
		printf("FPGA configuration failed.\n");
		dev.reset();
		return(dev);
	}

	// Check for FrontPanel support in the FPGA configuration.
	if (false == dev->IsFrontPanelEnabled()) {
		printf("FrontPanel support is not enabled.\n");
		dev.reset();
		return(dev);
	}

	printf("FrontPanel support is enabled.\n");

	return(dev);
}




#endif // !FRONTPANELSUPPORT_H
