#include "Game/Adventure.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineConfig.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "MapDefinition.hpp"


std::map<std::string, Adventure*> Adventure::s_registry;


Adventure::Adventure(XMLNode element)
{
	m_name = ParseXMLAttributeString(element, "name", "ERROR_INVALID_NAME");
	ASSERT_OR_DIE(m_name != "ERROR_INVALID_NAME", "No name found for Adventure element.");

	m_title = ParseXMLAttributeString(element, "name", "ERROR_INVALID_TITLE");
	ASSERT_OR_DIE(m_title != "ERROR_INVALID_TITLE", "No title found for Adventure element.");

	ParseStartConditions(element.getChildNode("StartConditions"));
	ParseVictoryConditions(element.getChildNode("VictoryConditions"));
	ParseDefeatConditions(element.getChildNode("DefeatConditions"));
	
	ParseAdventureMaps(element);

	s_registry[m_name] = this;
}

Adventure* Adventure::GetDefinition(std::string definitionName)
{
	std::map<std::string, Adventure*>::iterator found = Adventure::s_registry.find(definitionName);
	if (found != Adventure::s_registry.end())
		return found->second;
	else
		return nullptr;
}


void Adventure::ParseStartConditions(XMLNode startConditionElement)
{
	m_startingConditions.m_startingMap = ParseXMLAttributeString(startConditionElement, "startMap", "INVALID_START_MAP");
	ASSERT_OR_DIE(m_startingConditions.m_startingMap != "INVALID_START_MAP", "Missing starting map name in Adventure.");

	m_startingConditions.m_tileTypeToStartOn = ParseXMLAttributeString(startConditionElement, "startTile", "");

	m_startingText = ParseXMLAttributeString(startConditionElement, "startText", m_title);
}

void Adventure::ParseVictoryConditions(XMLNode victoryConditionElement)
{
	std::string victoryItemsString = ParseXMLAttributeString(victoryConditionElement, "haveItems", "INVALID_VICTORY_ITEMS");
	m_victoryItems = Split(victoryItemsString, ',');

	m_victoryText = ParseXMLAttributeString(victoryConditionElement, "victoryText", "You Win!");
}

void Adventure::ParseDefeatConditions(XMLNode defeatConditionElement)
{
	m_defeatText = ParseXMLAttributeString(defeatConditionElement, "defeatText", "Game Over.");
}

void Adventure::ParseAdventureMaps(XMLNode adventureElement)
{
	for (int mapIndex = 0; mapIndex < adventureElement.nChildNode("Map"); mapIndex++)
	{
		XMLNode mapNode = adventureElement.getChildNode("Map", mapIndex);
		ParseAdventureMap(mapNode);
	}
}

void Adventure::ParseAdventureMap(XMLNode mapNode)
{
	AdventureMap newMap;
	newMap.m_name = ParseXMLAttributeString(mapNode, "name", "INVALID_MAP_NAME");
	ASSERT_OR_DIE(newMap.m_name != "INVALID_MAP_NAME", "Missing map name in Adventure.");

	newMap.m_definition = MapDefinition::GetDefinition(ParseXMLAttributeString(mapNode, "definition", ""));
	ASSERT_OR_DIE(newMap.m_definition != nullptr, "Missing definition for Adventure map.");

	ParseAdventureItemsIntoVector(newMap.m_itemsToSpawn, mapNode);
	ParseAdventureExitsIntoVector(newMap.m_exitsToSpawn, mapNode);
	ParseAdventureCharactersIntoVector(newMap.m_charactersToSpawn, mapNode);

	m_maps[newMap.m_name] = newMap;
}

void Adventure::ParseAdventureExitsIntoVector(std::vector<AdventureExit>& out_adventureExitVector, XMLNode elementContainingExits)
{
	for (int exitIndex = 0; exitIndex < elementContainingExits.nChildNode("Exit"); exitIndex++)
	{
		out_adventureExitVector.push_back(ParseAdventureExit(elementContainingExits.getChildNode("Exit", exitIndex)));
	}
}

AdventureExit Adventure::ParseAdventureExit(XMLNode exitNode)
{
	AdventureExit newAdventureExit;
	newAdventureExit.m_exitType = ParseXMLAttributeString(exitNode, "type", "INVALID_EXIT_TYPE");
	ASSERT_OR_DIE(newAdventureExit.m_exitType != "INVALID_EXIT_TYPE", "Missing or invalid exit type in Adventure.");

	newAdventureExit.m_destinationMapName = ParseXMLAttributeString(exitNode, "destinationMap", "INVALID_MAP_NAME");
	ASSERT_OR_DIE(newAdventureExit.m_destinationMapName != "INVALID_MAP_NAME", "Missing or invalid destination map name in Adventure.");

	newAdventureExit.m_destinationTileType = ParseXMLAttributeString(exitNode, "destinationTile", "");
	newAdventureExit.m_featureTypeToPlace = ParseXMLAttributeString(exitNode, "createDestinationFeature", "");
	newAdventureExit.m_tileTypeToSpawnOn = ParseXMLAttributeString(exitNode, "onTileType", "");

	return newAdventureExit;
}

void Adventure::ParseAdventureItemsIntoVector(std::vector<AdventureItem>& out_adventureItemVector, XMLNode elementContainingItems)
{
	for (int itemIndex = 0; itemIndex < elementContainingItems.nChildNode("Item"); itemIndex++)
	{
		out_adventureItemVector.push_back(ParseAdventureItem(elementContainingItems.getChildNode("Item", itemIndex)));
	}
}

AdventureItem Adventure::ParseAdventureItem(XMLNode itemNode)
{
	AdventureItem newAdventureItem;
	newAdventureItem.m_tileTypeToSpawnOn = ParseXMLAttributeString(itemNode, "onTileType", "");

	newAdventureItem.m_definition = ParseXMLAttributeString(itemNode, "type", "");
	ASSERT_OR_DIE(newAdventureItem.m_definition != "", "Missing item type in Adventure.");

	return newAdventureItem;
}

void Adventure::ParseAdventureCharactersIntoVector(std::vector<AdventureCharacter>& out_AdventureCharacterVector, XMLNode elementContainingCharacters)
{
	for (int characterIndex = 0; characterIndex < elementContainingCharacters.nChildNode("Character"); characterIndex++)
	{
		out_AdventureCharacterVector.push_back(ParseAdventureCharacter(elementContainingCharacters.getChildNode("Character", characterIndex)));
	}
}

AdventureCharacter Adventure::ParseAdventureCharacter(XMLNode characterNode)
{
	AdventureCharacter newAdventureCharacter;
	newAdventureCharacter.m_tileTypeToSpawnOn = ParseXMLAttributeString(characterNode, "onTileType", "");

	newAdventureCharacter.m_builderName = ParseXMLAttributeString(characterNode, "type", "");
	ASSERT_OR_DIE(newAdventureCharacter.m_builderName != "", "Missing character type in Adventure.");

	return newAdventureCharacter;
}