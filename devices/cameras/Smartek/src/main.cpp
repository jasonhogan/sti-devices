
#include "smcs_cpp/CameraSDK.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <array>
#include <locale>

#include <string>
#include <ORBManager.h>
#include "SmartekDevice.h"

ORBManager* orbManager;

namespace common
{
std::string IpAddrToString(UINT32 ipAddress)
{
	std::stringstream stream;

	UINT32 temp1 = ((ipAddress >> 24) & 0xFF);
	UINT32 temp2 = ((ipAddress >> 16) & 0xFF);
	UINT32 temp3 = ((ipAddress >> 8) & 0xFF);
	UINT32 temp4 = ((ipAddress)& 0xFF);

	stream << temp1 << "." << temp2 << "." << temp3 << "." << temp4;

	return stream.str();
}

std::string MacAddrToString(UINT64 macAddress)
{
	std::array<UINT32, 6> positions = { 40, 32, 24, 16, 8, 0 };

	std::stringstream stream;
	// MAC Address has 8 Bytes
	for (int i = 0; i < positions.size(); ++i)
	{
		UINT32 numb = (UINT32)((macAddress >> positions[i]) & 0xFF);

		if (numb < 16) {
			stream << "0";
		}
		stream << std::hex << numb;

		if (i < positions.size() - 1) {
			stream << ":";
		}
	}

	std::string result(stream.str());

	std::locale loc;
	for (int i = 0; i < result.size(); ++i) {
		result[i] = std::toupper(result[i], loc);
	}

	return result;
}

}

int main(int argc, char* argv[])
{
	orbManager = new ORBManager(argc, argv);
	std::string configFilename = "SmartekCamera.ini"; //default

	ConfigFile configFile(configFilename);


	// initialize Camera API
	smcs::InitCameraAPI();
	smcs::ICameraAPI smcsApi = smcs::GetCameraAPI();

	if (!smcsApi->IsUsingKernelDriver()) {
		std::cout << "Warning: Smartek Filter Driver not loaded." << std::endl;
	}

	smcsApi->SetHeartbeatTime(3);

	// discover all devices on network
	smcsApi->FindAllDevices(3.0);
	const smcs::DevicesList devices = smcsApi->GetAllDevices();

	if (devices.size() == 0) {
		std::cout << "No cameras found." << std::endl;
		return 0;
	}

	smcs::IDevice device = devices.at(0);

	if (!device->IsConnected()) {	
		device->Connect();
	}

	SmartekDevice smartekCamera(orbManager, configFile, devices.at(0));
	smartekCamera.setSaveAttributesToFile(true);

	orbManager->run();	//Blocks while device is alive


	if (device->IsConnected()) {
		device->Disconnect();
	}
	smcs::ExitCameraAPI();

	return 0;
}
//
//int main_org(int argc, char* argv[])
//{
//	// initialize Camera API
//	smcs::InitCameraAPI();
//	smcs::ICameraAPI smcsApi = smcs::GetCameraAPI();
//	
//	if (!smcsApi->IsUsingKernelDriver()) {
//		std::cout << "Warning: Smartek Filter Driver not loaded." << std::endl;
//	}
//
//	smcsApi->SetHeartbeatTime(3);
//
//	// discover all devices on network
//	smcsApi->FindAllDevices(3.0);
//	const smcs::DevicesList devices = smcsApi->GetAllDevices();
//
//	if (devices.size() > 0) {
//		std::cout << std::setw(5) << "Index" << "    " << std::setw(10) << std::left << "Vendor" << std::setw(16) << "Model" << std::setw(18) << "IP" << std::setw(18) << "Subnetmask" << std::setw(20) << "MAC" << std::setw(9) << "Available" << std::endl;
//		for (int n = 0; n < (int)devices.size(); n++) {
//			smcs::IDevice device = devices[n];
//
//			std::string available = "No";
//			if (!device->IsConnected()) {
//				device->Connect();
//				if (device->IsConnected()) {
//					available = "Yes";
//					device->Disconnect();
//				}
//			}
//			std::cout << std::setw(5) << std::right << n << "    " << std::setw(10) << std::left << device->GetManufacturerName() << std::setw(16) << device->GetModelName() << std::setw(18) << common::IpAddrToString(device->GetIpAddress()) << std::setw(18) << common::IpAddrToString(device->GetSubnetMask()) << std::setw(20) << common::MacAddrToString(device->GetMacAddress()) << std::setw(9) << available << std::endl;
//		}
//		std::cout << std::endl;
//		std::cout << "Select index number to connect to camera, make sure camera is available." << std::endl;
//		int index;
//		std::string read;
//
//		while (true) {
//			std::cin >> read;
//			char *end;
//
//			strtol(read.c_str(), &end, 10);
//			if (*end == '\0') {
//				index = atoi(read.c_str());
//				break;
//			}
//			else {
//				std::cout << "Not a valid number, try again." << std::endl;
//			}
//		}
//
//		if (index >= 0 && index < (int)devices.size()) {
//			smcs::IDevice device = devices[index];
//
//			if (device != NULL && device->Connect()) {
//				std::cout << "Connected to " << device->GetModelName() << " camera with IP number " << common::IpAddrToString(device->GetIpAddress()) << std::endl;
//				INT64 packetSize = 0;
//				device->GetIntegerNodeValue("GevSCPSPacketSize", packetSize);
//				packetSize = packetSize & 0xFFFF;
//				std::cout << "PacketSize: " << packetSize << std::endl;
//
//
//
//				// enable trigger mode
//				bool status = device->SetStringNodeValue("TriggerMode", "On");
//				// set software trigger mode
//				status = device->SetStringNodeValue("TriggerSource", "Software");
//				// set continuous acquisition mode
//				status = device->SetStringNodeValue("AcquisitionMode", "Continuous");
//				// set trigger selector to frame start
//				status = device->SetStringNodeValue("TriggerSelector", "FrameStart");
//
//
//				status = device->SetIntegerNodeValue("TLParamsLocked", 1);
//
//				status = device->SetFloatNodeValue("ExposureTime", 300);
//				double val;
//				status = device->GetFloatNodeValue("ExposureTime", val);
//				std::cout << "ExposureTime set to " << val << std::endl;
//
//				// start acquisition
//				status = device->SetIntegerNodeValue("TLParamsLocked", 1);
//				status = device->CommandNodeExecute("AcquisitionStart");
//
//				std::string value;
//				status = device->GetStringNodeValue("AcquisitionMode", value);
//				std::cout << "AcquisitionMode set to " << value << std::endl;
//				status = device->GetStringNodeValue("TriggerMode", value);
//				std::cout << "TriggerMode set to " << value << std::endl;
//				status = device->GetStringNodeValue("TriggerSource", value);
//				std::cout << "TriggerSource set to " << value << std::endl;
//
//				status = device->GetStringNodeValue("DeviceUserID", value);
//				std::cout << "DeviceUserID set to " << value << std::endl;
//
////				std::thread threadImage(ProcessImage, device);
////				ProcessTrigger(device);
////				threadImage.join();
//			}
//			else {
//				std::cout << "Can not connect to " << device->GetModelName() << " camera with IP number " << common::IpAddrToString(device->GetIpAddress()) << " press any key to exit." << std::endl;
//			}
//		}
//		else {
//			std::cout << "Not valid index number, press any key to exit." << std::endl;
//		}
//	}
//	else {
//		std::cout << "No cameras found, press any key to exit." << std::endl;
//	}
//
//
//	smcs::ExitCameraAPI();
//
//	return 0;
//}
//
