#pragma once
#include "ThirdParty\XMLParser\XMLParser.hpp"
#include <string>
#include "Game/MapGenerator.hpp"

struct CellularAutomataRule
{
	std::string m_ifTile;
	std::string m_ifNeighborTile;
	std::string m_changeToTile;
	std::string m_tagsToSet;
	int m_ifGreaterThanNumber = -1;
	int m_ifFewerThanNumber = 9999;
	float m_chanceToRunPerTile = 1.f;
};

class MapGeneratorCellularAutomata : public MapGenerator
{
public:
	MapGeneratorCellularAutomata(XMLNode element);
	virtual void GenerateMap(Map*& outMapToGenerate) override;

	int m_numIterations = 1;
	std::vector<CellularAutomataRule> m_rules;
	float m_permanence = 0.5f;
private:
	void ApplyRulesToMap(Map*& outMapToGenerate);
	void ParseRule(XMLNode ruleElement);
	int GetNumberOfNeighborsOfType(const Tile& tile, std::string neighborType);
};