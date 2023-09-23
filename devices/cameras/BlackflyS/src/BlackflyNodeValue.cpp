#include "BlackflyNodeValue.h"

using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;


//bool BlackflyNodeValue::checkNode()
//{
//	bool nodeReady = IsAvailable(node) && IsReadable(node);
//	return nodeReady;
//}

bool BlackflyNodeValue::checkNode(const Node_ptr& node)
{
	bool nodeReady = node.IsValid() && IsAvailable(node) && IsReadable(node);
	return nodeReady;
}

bool BlackflyNodeValue::checkNode()
{
	return checkNode(node);
}

bool BlackflyStringNodeValue::setValue(const std::string& value) 
{ 
	if (!checkNode())
		return false;

	try {
		CStringPtr ptrStringNode = static_cast<CStringPtr>(node);

		gcstring gvValue = value.c_str();
	
		ptrStringNode->SetValue(gvValue);
	}
	catch (Spinnaker::Exception&)
	{
		return false;
	}

	return true;
}

bool BlackflyStringNodeValue::getValue(std::string& value)
{
	if (!checkNode())
		return false;

	try {
		CStringPtr ptrStringNode = static_cast<CStringPtr>(node);

		value = ptrStringNode->GetValue().c_str();
	}
	catch (Spinnaker::Exception&)
	{
		return false;
	}

	return true; 
}

bool BlackflyFloatNodeValue::setValue(const std::string& value)
{
	if (!checkNode())
		return false;

	double val;

	if (!STI::Utils::stringToValue(value, val))
		return false;

	try {
		CFloatPtr ptrFloatNode = static_cast<CFloatPtr>(node);

		ptrFloatNode->SetValue(val);

	}
	catch (Spinnaker::Exception& ex)
	{
		return false;
	}

	return true;
}

bool BlackflyFloatNodeValue::getValue(std::string& value)
{
	if (!checkNode())
		return false;

	try {
		CFloatPtr ptrFloatNode = static_cast<CFloatPtr>(node);

		double val = ptrFloatNode->GetValue();
		value = STI::Utils::valueToString(val);
		
		return true;
	}
	catch (Spinnaker::Exception&)
	{
		return false;
	}
}

bool BlackflyIntNodeValue::setValue(const std::string& value)
{
	if (!checkNode())
		return false;

	int val;

	if (!STI::Utils::stringToValue(value, val))
		return false;

	try {
		CIntegerPtr ptrIntegerNode = static_cast<CIntegerPtr>(node);

		ptrIntegerNode->SetValue(val);

	}
	catch (Spinnaker::Exception&)
	{
		return false;
	}

	return true;
}

bool BlackflyIntNodeValue::getValue(std::string& value)
{
	if (!checkNode())
		return false;

	try {
		CIntegerPtr ptrIntegerNode = static_cast<CIntegerPtr>(node);

		int64_t val = ptrIntegerNode->GetValue();
		value = STI::Utils::valueToString(val);

		return true;
	}
	catch (Spinnaker::Exception&)
	{
		return false;
	}
}


bool BlackflyEnumNodeValue::setValue(const std::string& value)
{
	if (!checkNode())
		return false;

	try {
		CEnumerationPtr ptrEnumNode = static_cast<CEnumerationPtr>(node);
		
		gcstring gcValue = value.c_str();
		CEnumEntryPtr ptrEntry = ptrEnumNode->GetEntryByName(gcValue);

		if (!checkNode(ptrEntry))
			return false;

		ptrEnumNode->SetIntValue(ptrEntry->GetValue());
	}
	catch (Spinnaker::Exception&)
	{
		return false;
	}

	return true;
}

bool BlackflyEnumNodeValue::getValue(std::string& value)
{
	if (!checkNode())
		return false;

	try {
		CEnumerationPtr ptrEnumNode = static_cast<CEnumerationPtr>(node);

		value = ptrEnumNode->ToString().c_str();
	}
	catch (Spinnaker::Exception&)
	{
		return false;
	}

	return true;
}


bool BlackflyEnumNodeValue::getEnumValues(std::vector<std::string>& values)
{
	values.clear();

//	if (!checkNode())
//		return false;

	try {
		CEnumerationPtr ptrEnumNode = static_cast<CEnumerationPtr>(node);
		
		StringList_t symbolicNames;
		//ptrEnumNode->GetSymbolics(symbolicNames);

		NodeList_t nodeEntries;
		ptrEnumNode->GetEntries(nodeEntries);

		for (auto& node : nodeEntries) {
			std::string rawName = node->GetName().c_str();

			//Raw node names are of the format: EnumEntry_<key>_<node name>
			//Here we strip the first two parts and keep only <node name>
			auto pos = rawName.find(key);
			auto nodeName = rawName.substr(pos + key.size() + 1);
			values.push_back(nodeName);
		}
	}
	catch (Spinnaker::Exception&)
	{
		return false;
	}

	return true;
}

void BlackflyEnumNodeValue::setupEnumValues()
{
	getEnumValues(allowedEnumValues);

	std::stringstream allowedVals;

	bool isFirst = true;
	for (auto& v : allowedEnumValues) {
		if (!isFirst) {
			allowedVals << ", ";
		}
		allowedVals << v;
		isFirst = false;
	}

	if (allowedEnumValues.size() > 0) {
		allowedValues = allowedVals.str();
	}
}

