#include "Game/MapGeneratorCellularAutomata.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "CharacterBuilder.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"


MapGeneratorCellularAutomata::MapGeneratorCellularAutomata(XMLNode element)
	: MapGenerator(element)
{
	m_numIterations = ParseXMLAttributeInt(element, "iterations", 0);
	ASSERT_OR_DIE(m_numIterations > 0, "Missing or invalid number of iteration for Cellular Automata.");

	ASSERT_OR_DIE(element.nChildNode("Rule") > 0, "No rules for Cellular Automata.");
	for (int ruleIndex = 0; ruleIndex < element.nChildNode("Rule"); ruleIndex++)
	{
		ParseRule(element.getChildNode("Rule", ruleIndex));
	}

	m_permanence = ParseXMLAttributeFloat(element, "permanence", m_permanence);
}

void MapGeneratorCellularAutomata::GenerateMap(Map*& outMapToGenerate)
{
	for (int iterationIndex = 0; iterationIndex < m_numIterations; iterationIndex++)
	{
		ApplyRulesToMap(outMapToGenerate);
	}
}

void MapGeneratorCellularAutomata::ApplyRulesToMap(Map*& outMapToGenerate)
{
	std::vector<Tile> tempMap;
	for (size_t tileIndex = 0; tileIndex < outMapToGenerate->m_tiles.size(); tileIndex++)
	{
		tempMap.push_back(outMapToGenerate->m_tiles[tileIndex]);
	}

	for (size_t tileIndex = 0; tileIndex < tempMap.size(); tileIndex++)
	{
		for (CellularAutomataRule rule : m_rules)
		{
			if(outMapToGenerate->m_tiles[tileIndex].m_tileDefinition->m_name == rule.m_ifTile)
			{
				int numNeighborsOfType = GetNumberOfNeighborsOfType(outMapToGenerate->m_tiles[tileIndex], rule.m_ifNeighborTile);
				if (numNeighborsOfType > rule.m_ifGreaterThanNumber && numNeighborsOfType < rule.m_ifFewerThanNumber && GetRandomFloatZeroToOne() <= rule.m_chanceToRunPerTile)
				{
					PlaceTileIfPossible(&tempMap[tileIndex], rule.m_changeToTile, m_permanence);
					tempMap[tileIndex].m_tags.SetTags(rule.m_tagsToSet);
				}
			}
		}
	}

	outMapToGenerate->m_tiles = tempMap;
}

void MapGeneratorCellularAutomata::ParseRule(XMLNode ruleElement)
{
	CellularAutomataRule newRule;
	newRule.m_ifTile = ParseXMLAttributeString(ruleElement, "ifTile", "INVALID_IFTILE");
	ASSERT_OR_DIE(newRule.m_ifTile != "INVALID_IFTILE", "Missing ifTile for Cellular Automata.");

	newRule.m_ifNeighborTile = ParseXMLAttributeString(ruleElement, "ifNeighborTile", "INVALID_IFNEIGHBORTILE");
	ASSERT_OR_DIE(newRule.m_ifNeighborTile != "INVALID_IFNEIGHBORTILE", "Missing ifNeighborTile for Cellular Automata.");

	newRule.m_changeToTile = ParseXMLAttributeString(ruleElement, "changeToTile", "INVALID_CHANGETOTILE");
	ASSERT_OR_DIE(newRule.m_changeToTile != "INVALID_CHANGETOTILE", "Missing changeToTile for Cellular Automata.");

	newRule.m_chanceToRunPerTile = ParseXMLAttributeFloat(ruleElement, "chanceToRunPerTile", newRule.m_chanceToRunPerTile);
	newRule.m_ifGreaterThanNumber = ParseXMLAttributeInt(ruleElement, "ifGreaterThan", newRule.m_ifGreaterThanNumber);
	newRule.m_ifFewerThanNumber = ParseXMLAttributeInt(ruleElement, "ifFewerThan", newRule.m_ifFewerThanNumber);
	newRule.m_tagsToSet = ParseXMLAttributeString(ruleElement, "setTags", "");

	m_rules.push_back(newRule);
}

int MapGeneratorCellularAutomata::GetNumberOfNeighborsOfType(const Tile& tile, std::string neighborType)
{
	int neighborOfTypeCount = 0;

	if (tile.GetEastNeighbor() && tile.GetEastNeighbor()->m_tileDefinition->m_name == neighborType)
		neighborOfTypeCount++;
	if (tile.GetNorthEastNeighbor() && tile.GetNorthEastNeighbor()->m_tileDefinition->m_name == neighborType)
		neighborOfTypeCount++;
	if (tile.GetNorthNeighbor() && tile.GetNorthNeighbor()->m_tileDefinition->m_name == neighborType)
		neighborOfTypeCount++;
	if (tile.GetNorthWestNeighbor() && tile.GetNorthWestNeighbor()->m_tileDefinition->m_name == neighborType)
		neighborOfTypeCount++;
	if (tile.GetWestNeighbor() && tile.GetWestNeighbor()->m_tileDefinition->m_name == neighborType)
		neighborOfTypeCount++;
	if (tile.GetSouthWestNeighbor() && tile.GetSouthWestNeighbor()->m_tileDefinition->m_name == neighborType)
		neighborOfTypeCount++;
	if (tile.GetSouthNeighbor() && tile.GetSouthNeighbor()->m_tileDefinition->m_name == neighborType)
		neighborOfTypeCount++;
	if (tile.GetSouthEastNeighbor() && tile.GetSouthEastNeighbor()->m_tileDefinition->m_name == neighborType)
		neighborOfTypeCount++;

	return neighborOfTypeCount;
}
