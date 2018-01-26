#include "Game/LootTable.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

std::map<std::string, LootTable*> LootTable::s_registry;

LootTable::LootTable(XMLNode element)
{
	m_name = ParseXMLAttributeString(element, "name", "ERROR_INVALID_NAME");
	ASSERT_OR_DIE(m_name != "ERROR_INVALID_NAME", "No name found for ItemDefinition element.");

	for (int lootIndex = 0; lootIndex < element.nChildNode("Loot"); lootIndex++)
	{
		XMLNode lootNode = element.getChildNode("Loot", lootIndex);

		std::string itemName = ParseXMLAttributeString(lootNode, "name", "ERROR_INVALID_LOOT_NAME");
		ASSERT_OR_DIE(itemName != "ERROR_INVALID_LOOT_NAME", "No name found for loot element.");

		int lootWeight = ParseXMLAttributeInt(lootNode, "weight", 0);
		ASSERT_OR_DIE(lootWeight != 0, "No/invalid weight found for loot element.");

		m_possibleItems.AddItem(itemName, lootWeight);
	}

	s_registry[m_name] = this;
}

LootTable* LootTable::GetLootTable(std::string name)
{
	std::map<std::string, LootTable*>::iterator found = LootTable::s_registry.find(name);
	if (found != LootTable::s_registry.end())
		return found->second;
	else
		return nullptr;
}
