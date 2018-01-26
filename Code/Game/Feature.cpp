#include "Game/Feature.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

std::map<std::string, Feature*> Feature::s_registry;


Feature::Feature(XMLNode element)
	: Entity()
	, m_isSolid(false)
{
	m_name = ParseXMLAttributeString(element, "name", "ERROR_INVALID_NAME");
	ASSERT_OR_DIE(m_name != "ERROR_INVALID_NAME", "No name found for TileDefinition element.");

	m_isSolid = ParseXMLAttributeBool(element, "isSolid", false);
	m_glyph = ParseXMLAttributeChar(element, "glyph", ' ');
	m_glyphColor = ParseXMLAttributeRgba(element, "glyphColor", Rgba::WHITE);
	m_fillColor = ParseXMLAttributeRgba(element, "fillColor", Rgba::WHITE);

	if (element.nChildNode("Exit") > 0)
	{
		m_isExit = true;
		m_exitData = ParseExitData(element.getChildNode("Exit"));
	}
	else
	{
		m_isExit = false;
	}

	s_registry[m_name] = this;
}

Feature::Feature(std::string typeName)
	: Entity()
	, m_isSolid(false)
{
	std::map<std::string, Feature*>::iterator found = s_registry.find(typeName);
	if (found == s_registry.end())
		ERROR_AND_DIE("Attempted to create unknown feature type.");

	Feature* foundFeature = found->second;
	m_name = foundFeature->m_name;
	m_isSolid = foundFeature->m_isSolid;
	m_glyph = foundFeature->m_glyph;
	m_glyphColor = foundFeature->m_glyphColor;
	m_fillColor = foundFeature->m_fillColor;
	m_isExit = foundFeature->m_isExit;
	m_exitData = foundFeature->m_exitData;
}

Feature::~Feature()
{

}

std::vector<Message> Feature::GetTooltipInfo() const
{
	std::vector<Message> outputInfo;
	outputInfo.push_back(Message(m_name, m_glyphColor, 1.f));

	if (m_isSolid)
		outputInfo.push_back(Message(" solid", Rgba::LIGHT_GREY, 0.5f));
	
	for (Item* item : m_entityInventory.m_items)
	{
		outputInfo.push_back(Message("  " + item->m_definition->m_name, Rgba::WHITE, 0.5f));
	}

	return outputInfo;
}

ExitData Feature::ParseExitData(XMLNode exitNode)
{
	ExitData newExitData;

	newExitData.m_destinationTile = nullptr;
	newExitData.m_destinationTileType = ParseXMLAttributeString(exitNode, "destinationTileType", "");
	std::string directionString = ParseXMLAttributeString(exitNode, "destination", "MISSING_DIRECTION");
	if (directionString == "next")
		newExitData.m_direction = DIRECTION_NEXT;
	else if (directionString == "previous")
		newExitData.m_direction = DIRECTION_PREVIOUS;
	else
		ERROR_AND_DIE("Missing or invalid direction for exit.");

	newExitData.m_isTwoWay = ParseXMLAttributeBool(exitNode, "isTwoWay", false);

	return newExitData;
}
