#include "Game/CharacterBuilder.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "LootTable.hpp"
#include "Engine/Core/StringUtils.hpp"

std::map<std::string, CharacterBuilder*> CharacterBuilder::s_registry;


CharacterBuilder::CharacterBuilder(XMLNode element)
	: m_damageTypeWeaknesses()
	, m_damageTypeResistances()
	, m_damageTypeImmunities()
{
	m_name = ParseXMLAttributeString(element, "name", "ERROR_INVALID_NAME");
	ASSERT_OR_DIE(m_name != "ERROR_INVALID_NAME", "No name found for Character element.");

	m_glyph = ParseXMLAttributeChar(element, "glyph", ' ');

	m_glyphColor = ParseXMLAttributeRgba(element, "glyphColor", Rgba::WHITE);

	m_fillColor = ParseXMLAttributeRgba(element, "fillColor", Rgba::WHITE);

	m_minStats.m_stats[STAT_STRENGTH] = ParseXMLAttributeInt(element, "minStrength", 0);
	m_maxStats.m_stats[STAT_STRENGTH] = ParseXMLAttributeInt(element, "maxStrength", 0);

	m_minStats.m_stats[STAT_AGILITY] = ParseXMLAttributeInt(element, "minAgility", 0);
	m_maxStats.m_stats[STAT_AGILITY] = ParseXMLAttributeInt(element, "maxAgility", 0);

	m_minStats.m_stats[STAT_MAGIC] = ParseXMLAttributeInt(element, "minMagic", 0);
	m_maxStats.m_stats[STAT_MAGIC] = ParseXMLAttributeInt(element, "maxMagic", 0);

	m_minStats.m_stats[STAT_ENDURANCE] = ParseXMLAttributeInt(element, "minEndurance", 0);
	m_maxStats.m_stats[STAT_ENDURANCE] = ParseXMLAttributeInt(element, "maxEndurance", 0);

	m_minStats.m_stats[STAT_LUCK] = ParseXMLAttributeInt(element, "minLuck", 0);
	m_maxStats.m_stats[STAT_LUCK] = ParseXMLAttributeInt(element, "maxLuck", 0);

	m_minStats.m_stats[STAT_MAX_HP] = ParseXMLAttributeInt(element, "minHP", 0);
	m_maxStats.m_stats[STAT_MAX_HP] = ParseXMLAttributeInt(element, "maxHP", 0);

	XMLNode behaviorRoot = element.getChildNode("Behaviors");
	for (int behaviorIndex = 0; behaviorIndex < behaviorRoot.nChildNode(); behaviorIndex++)
	{
		m_behaviors.push_back(Behavior::Create(behaviorRoot.getChildNode(behaviorIndex)));
	}

	m_faction = ParseXMLAttributeString(element, "faction", "ERROR_INVALID_FACTION");
	ASSERT_OR_DIE(m_faction != "ERROR_INVALID_FACTION", "No faction found for Character element.");

	for (int lootIndex = 0; lootIndex < element.nChildNode("Loot"); lootIndex++)
	{
		XMLNode lootNode = element.getChildNode("Loot", lootIndex);
		m_loot.push_back(lootNode.getAttribute("name"));
	}

	XMLNode gCostBiasesRoot = element.getChildNode("PathingCostBiases");
	for (int gCostBiasIndex = 0; gCostBiasIndex < gCostBiasesRoot.nChildNode("Bias"); gCostBiasIndex++)
	{
		XMLNode biasNode = gCostBiasesRoot.getChildNode("Bias", gCostBiasIndex);
		std::string biasTile = ParseXMLAttributeString(biasNode, "tile", "INVALID_TILE");
		ASSERT_OR_DIE(biasTile != "INVALID_TILE", "Missing tile type for pathing bias.");
		float costBias = ParseXMLAttributeFloat(biasNode, "cost", -1.f);
		ASSERT_OR_DIE(costBias > 0.f, "Invalid or missing cost for pathing bais. Costs must be greater than 0.");
		m_gCostBiases[biasTile] = costBias;
	}

	if (element.nChildNode("Tags") == 1)
	{
		XMLNode tagsNode = element.getChildNode("Tags");
		m_tagsToSet = ParseXMLAttributeString(tagsNode, "tags", "");
	}
	ASSERT_OR_DIE(element.nChildNode("Tags") <= 1, "Too many tags elements in character.");

	//Weaknesses
	if (element.nChildNode("DamageTypeWeaknesses") == 1)
	{
		XMLNode weaknessNode = element.getChildNode("DamageTypeWeaknesses");
		m_damageTypeWeaknesses = Split(ParseXMLAttributeString(weaknessNode, "tags", ""), ',');
	}
	ASSERT_OR_DIE(element.nChildNode("DamageTypeWeaknesses") <= 1, "Too many weakness elements in character.");

	//Resistances
	if (element.nChildNode("DamageTypeResistances") == 1)
	{
		XMLNode resistanceNode = element.getChildNode("DamageTypeResistances");
		m_damageTypeResistances = Split(ParseXMLAttributeString(resistanceNode, "tags", ""), ',');
	}
	ASSERT_OR_DIE(element.nChildNode("DamageTypeResistances") <= 1, "Too many resistance elements in character.");

	//Immunities
	if (element.nChildNode("DamageTypeImmunities") == 1)
	{
		XMLNode immunityNode = element.getChildNode("DamageTypeImmunities");
		m_damageTypeImmunities = Split(ParseXMLAttributeString(immunityNode, "tags", ""), ',');
	}
	ASSERT_OR_DIE(element.nChildNode("DamageTypeImmunities") <= 1, "Too many immunity elements in character.");

	s_registry[m_name] = this;
}

Character* CharacterBuilder::BuildNewCharacter(std::string characterTypeName)
{
	std::map<std::string, CharacterBuilder*>::iterator found = s_registry.find(characterTypeName);
	if (found == s_registry.end())
		ERROR_AND_DIE("Attempted to build unknown character type.");

	CharacterBuilder* foundBuilder = found->second;

	Character* newCharacter = new Character();
	newCharacter->m_name = foundBuilder->m_name;
	newCharacter->m_glyph = foundBuilder->m_glyph;
	newCharacter->m_glyphColor = foundBuilder->m_glyphColor;
	newCharacter->m_fillColor = foundBuilder->m_fillColor;

	newCharacter->m_faction = foundBuilder->m_faction;
	newCharacter->m_stats = Stats::CalculateRandomStatsInRange(foundBuilder->m_minStats, foundBuilder->m_maxStats);
	newCharacter->m_behaviors = CloneBehaviors(foundBuilder->m_behaviors);
	newCharacter->m_currentHP = newCharacter->m_stats[STAT_MAX_HP];
	newCharacter->m_gCostBiases = foundBuilder->m_gCostBiases;
	newCharacter->m_tags.SetTags(foundBuilder->m_tagsToSet);
	newCharacter->m_damageTypeWeaknesses = foundBuilder->m_damageTypeWeaknesses;
	newCharacter->m_damageTypeResistances = foundBuilder->m_damageTypeResistances;
	newCharacter->m_damageTypeImmunities = foundBuilder->m_damageTypeImmunities;

	for (size_t lootIndex = 0; lootIndex < foundBuilder->m_loot.size(); lootIndex++)
	{
		std::string itemFromTable = LootTable::GetLootTable(foundBuilder->m_loot[lootIndex])->m_possibleItems.GetWeightedRandomItem();
		Item* newItem = new Item(itemFromTable);
		newCharacter->m_entityInventory.AddItem(newItem);
		if (newItem->m_definition->m_slot != EQUIP_SLOT_NONE)
		{
			if (newCharacter->m_equipment.m_equippedItems[newItem->m_definition->m_slot] == nullptr)
				newCharacter->m_equipment.m_equippedItems[newItem->m_definition->m_slot] = newItem;
			else if (newCharacter->m_equipment.m_equippedItems[newItem->m_definition->m_slot]->CalculateTotalStatModifier() < newItem->CalculateTotalStatModifier())
				newCharacter->m_equipment.m_equippedItems[newItem->m_definition->m_slot] = newItem;
		}

	}

	return newCharacter;
}

std::vector<Behavior*> CharacterBuilder::CloneBehaviors(std::vector<Behavior*> behaviorsToClone)
{
	std::vector<Behavior*> outputBehaviors;
	for (size_t behaviorIndex = 0; behaviorIndex < behaviorsToClone.size(); behaviorIndex++)
	{
		outputBehaviors.push_back(behaviorsToClone[behaviorIndex]->Clone());
	}

	return outputBehaviors;
}