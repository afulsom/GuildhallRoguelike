#pragma once
#include <string>
#include "ThirdParty\XMLParser\XMLParser.hpp"
#include <map>
#include <vector>
#include "Game/MapGenerator.hpp"



struct AdventureItem
{
	std::string m_definition;
	std::string m_tileTypeToSpawnOn;
};

struct AdventureCharacter
{
	std::string m_builderName;
	std::string m_tileTypeToSpawnOn;
};

struct AdventureExit
{
	std::string m_exitType;
	std::string m_tileTypeToSpawnOn;
	std::string m_destinationMapName;
	std::string m_destinationTileType;
	std::string m_featureTypeToPlace;
};

struct AdventureMap
{
	std::string m_name;
	MapDefinition* m_definition;
	std::vector<AdventureItem> m_itemsToSpawn;
	std::vector<AdventureCharacter> m_charactersToSpawn;
	std::vector<AdventureExit> m_exitsToSpawn;
};

struct StartingConditions
{
	std::string m_startingMap;
	std::string m_tileTypeToStartOn;
};

class Adventure
{
public:
	Adventure(XMLNode element);

	std::string m_name;
	std::string m_title;
	StartingConditions m_startingConditions;
	std::vector<std::string> m_victoryItems;
	std::map<std::string, AdventureMap> m_maps;
	std::string m_startingText;
	std::string m_victoryText;
	std::string m_defeatText;

	static Adventure* GetDefinition(std::string definitionName);
	static std::map<std::string, Adventure*> s_registry;
private:
	void ParseStartConditions(XMLNode element);
	void ParseVictoryConditions(XMLNode element);
	void ParseDefeatConditions(XMLNode element);
	void ParseAdventureMaps(XMLNode element);
	void ParseAdventureMap(XMLNode mapNode);
	void ParseAdventureItemsIntoVector(std::vector<AdventureItem>& out_adventureItemVector, XMLNode elementContainingItems);
	AdventureItem ParseAdventureItem(XMLNode itemNode);
	void ParseAdventureCharactersIntoVector(std::vector<AdventureCharacter>& out_AdventureCharacterVector, XMLNode elementContainingCharacters);
	AdventureCharacter ParseAdventureCharacter(XMLNode characterNode);
	void ParseAdventureExitsIntoVector(std::vector<AdventureExit>& out_adventureItemVector, XMLNode elementContainingExits);
	AdventureExit ParseAdventureExit(XMLNode exitNode);

};