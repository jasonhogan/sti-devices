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
	catch (Spinnaker::Exception&)
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


