#ifndef BLACKFLYNODEVALUE_H
#define BLACKFLYNODEVALUE_H

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include <utils.h>

#include <memory>

class BlackflyNodeValue	//abstract
{
public:
	typedef Spinnaker::GenApi::CNodePtr Node_ptr;

	BlackflyNodeValue(const Node_ptr& node, const std::string& key,
		const std::string& defaultValue, const std::string& allowedValues)
		: node(node), key(key), allowedValues(allowedValues), defaultValue(defaultValue) { }
	BlackflyNodeValue(const Node_ptr& node, const std::string& key, const std::string& defaultValue)
		: BlackflyNodeValue(node, key, defaultValue, "") {}

	virtual ~BlackflyNodeValue() {}

	bool operator==(const std::string& nodeKey) { return key.compare(nodeKey) == 0; }
	bool valueIs(const std::string& input) 
	{
		std::string val;
		return getValue(val) && val.compare(input) == 0;
	}

	virtual bool setValue(const std::string& value) = 0;
	virtual bool getValue(std::string& value) = 0;

	std::string key;
	std::string defaultValue;	//Used as the inital value for the Attribute.
	std::string allowedValues;	//For initializing Attributes with a drop down list of options. Comma delimited.

	template<typename T>
	static bool checkNode(const T& node)
	{
		return checkNode(static_cast<Node_ptr>(node));
	}

	static bool checkNode(const Node_ptr& node);

	//Node value factory
	template<typename T>
	static std::shared_ptr<BlackflyNodeValue> makeNodeValue(const Node_ptr& node, const std::string& key,
		const T& defaultValue, const std::string& allowedValues = "");
	
	//template<>
	//static std::shared_ptr<BlackflyNodeValue> makeNodeValue<int>(const Node_ptr& node, const std::string& key,
	//	const int& defaultValue, const std::string& allowedValues);

	//template<>
	//static std::shared_ptr<BlackflyNodeValue> makeNodeValue<double>(const Node_ptr& node, const std::string& key,
	//	const double& defaultValue, const std::string& allowedValues);

protected:

	bool checkNode();

	Node_ptr node;
};

class BlackflyStringNodeValue : public BlackflyNodeValue
{
public:
	BlackflyStringNodeValue(const Node_ptr& device, const std::string& key,
		const std::string& defaultValue, const std::string& allowedValues)
		: BlackflyNodeValue(device, key, defaultValue, allowedValues) { setValue(defaultValue); }

	bool setValue(const std::string& value);
	bool getValue(std::string& value);
};

class BlackflyFloatNodeValue : public BlackflyNodeValue
{
public:

	BlackflyFloatNodeValue(const Node_ptr& node, const std::string& key,
		double defaultValue)
		: BlackflyFloatNodeValue(node, key, STI::Utils::valueToString(defaultValue))
	{
	}

	BlackflyFloatNodeValue(const Node_ptr& node, const std::string& key,
		const std::string& defaultValue)
		: BlackflyNodeValue(node, key, defaultValue)
	{
		setValue(defaultValue);
	}

	bool setValue(const std::string& value);
	bool getValue(std::string& value);

};

class BlackflyIntNodeValue : public BlackflyNodeValue
{
public:

	BlackflyIntNodeValue(const Node_ptr& node, const std::string& key,
		int defaultValue)
		: BlackflyIntNodeValue(node, key, STI::Utils::valueToString(defaultValue))
	{
	}

	BlackflyIntNodeValue(const Node_ptr& node, const std::string& key,
		const std::string& defaultValue)
		: BlackflyNodeValue(node, key, defaultValue)
	{ 
		setValue(defaultValue);
	}

	bool setValue(const std::string& value);
	bool getValue(std::string& value);

};

class BlackflyEnumNodeValue : public BlackflyNodeValue
{
public:

	BlackflyEnumNodeValue(const Node_ptr& device, const std::string& key,
		const std::string& defaultValue, const std::string& allowedValues)
		: BlackflyNodeValue(device, key, defaultValue, allowedValues) { setValue(defaultValue); }

	bool setValue(const std::string& value);
	bool getValue(std::string& value);

};



template<typename T>
std::shared_ptr<BlackflyNodeValue> BlackflyNodeValue::makeNodeValue(const Node_ptr& node, const std::string& key,
	const T& defaultValue, const std::string& allowedValues)
{
	using namespace Spinnaker::GenApi;
	std::shared_ptr<BlackflyNodeValue> nodePtr;

	//	if (!checkNode(node))
	//		return nodePtr;	//null

	std::string defaultValueStr = STI::Utils::valueToString(defaultValue);

	switch (node->GetPrincipalInterfaceType())
	{
	case intfIString:
		nodePtr = std::make_shared<BlackflyStringNodeValue>(node, key, defaultValueStr, allowedValues);
		break;
	case intfIEnumeration:
		nodePtr = std::make_shared<BlackflyEnumNodeValue>(node, key, defaultValueStr, allowedValues);
		break;
	case intfIInteger:
		nodePtr = std::make_shared<BlackflyIntNodeValue>(node, key, defaultValueStr);
		break;
	case intfIFloat:
		nodePtr = std::make_shared<BlackflyFloatNodeValue>(node, key, defaultValueStr);
		break;
	/*	case intfIBoolean:
	{
	return PrintBooleanNode(node, level);
	}
	case intfICommand:
	{
	return PrintCommandNode(node, level);

	}
	*/
	default:
		nodePtr = std::make_shared<BlackflyStringNodeValue>(node, key, defaultValueStr, allowedValues);
		break;
	}

	return nodePtr;
}


//
//template<>
//std::shared_ptr<BlackflyNodeValue> BlackflyNodeValue::makeNodeValue<int>(const Node_ptr& node, const std::string& key,
//	const int& defaultValue, const std::string& allowedValues)
//{
//	auto tmp = node->GetPrincipalInterfaceType();
//	auto tmp2 = Spinnaker::GenApi::intfIInteger;
//	bool test = tmp == tmp2;
//
//	if (node->GetPrincipalInterfaceType() == Spinnaker::GenApi::intfIInteger) {
//		return std::make_shared<BlackflyIntNodeValue>(node, key, defaultValue);
//	}
//
//	return makeNodeValue(node, key, STI::Utils::valueToString(defaultValue));
//}
//
//template<>
//std::shared_ptr<BlackflyNodeValue> BlackflyNodeValue::makeNodeValue<double>(const Node_ptr& node, const std::string& key,
//	const double& defaultValue, const std::string& allowedValues)
//{
//	if (node->GetPrincipalInterfaceType() == Spinnaker::GenApi::intfIFloat) {
//		return std::make_shared<BlackflyFloatNodeValue>(node, key, defaultValue);
//	}
//
//	return makeNodeValue(node, key, STI::Utils::valueToString(defaultValue));
//}
//


#endif

