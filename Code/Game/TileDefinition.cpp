#include "Game/TileDefinition.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

std::map<std::string, TileDefinition*> TileDefinition::s_tileDefinitionRegistry;

TileDefinition* TileDefinition::GetTileDefinition(std::string name)
{
	std::map<std::string, TileDefinition*>::iterator found = s_tileDefinitionRegistry.find(name);
	if (found != s_tileDefinitionRegistry.end())
		return found->second;
	else
		return nullptr;
}

TileDefinition::TileDefinition(XMLNode element)
{
	m_name = ParseXMLAttributeString(element, "name", "ERROR_INVALID_NAME");
	ASSERT_OR_DIE(m_name != "ERROR_INVALID_NAME", "No name found for TileDefinition element.");

	m_isSolid = ParseXMLAttributeBool(element, "isSolid", false);
	m_isOpaque = ParseXMLAttributeBool(element, "isOpaque", false);

	ASSERT_OR_DIE(element.nChildNode("Glyph") != 0, "No glyphs found for TileDefinition.");
	for (int fillColorIndex = 0; fillColorIndex < element.nChildNode("Glyph"); fillColorIndex++)
	{
		XMLNode glyphNode = element.getChildNode("Glyph", fillColorIndex);
		char glyph = ParseXMLAttributeChar(glyphNode, "glyph", ' ');
		m_glyphs.push_back(glyph);
	}

	ASSERT_OR_DIE(element.nChildNode("GlyphColor") != 0, "No glyph colors found for TileDefinition.");
	for (int fillColorIndex = 0; fillColorIndex < element.nChildNode("GlyphColor"); fillColorIndex++)
	{
		XMLNode glyphColorNode = element.getChildNode("GlyphColor", fillColorIndex);
		Rgba glyphColor = ParseXMLAttributeRgba(glyphColorNode, "glyphColor", Rgba::BLACK);
		m_glyphColors.push_back(glyphColor);
	}

	ASSERT_OR_DIE(element.nChildNode("FillColor") != 0, "No fill colors found for TileDefinition.");
	for (int fillColorIndex = 0; fillColorIndex < element.nChildNode("FillColor"); fillColorIndex++)
	{
		XMLNode fillColorNode = element.getChildNode("FillColor", fillColorIndex);
		Rgba fillColor = ParseXMLAttributeRgba(fillColorNode, "fillColor", Rgba::BLACK);
		m_fillColors.push_back(fillColor);
	}

	if (element.nChildNode("SolidExceptions") == 1)
	{
		XMLNode solidExceptionsNode = element.getChildNode("SolidExceptions");
		m_solidExceptions = ParseXMLAttributeString(solidExceptionsNode, "tags", "");
	}
	ASSERT_OR_DIE(element.nChildNode("SolidExceptions") <= 1, "Too many solid exception elements in tile definition.");

	s_tileDefinitionRegistry[m_name] = this;
}

TileDefinition::~TileDefinition()
{

}
