#pragma once
#include "Engine\Core\WeightedList.hpp"
#include "Game/ItemDefinition.hpp"



class LootTable
{
public:
	LootTable(XMLNode element);

	WeightedList<std::string> m_possibleItems;
	std::string m_name;

	static LootTable* GetLootTable(std::string name);
	static std::map<std::string, LootTable*> s_registry;
};