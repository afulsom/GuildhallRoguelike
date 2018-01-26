#include "Game/Item.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Item::Item(std::string typeName)
	: m_damageTypes()
{
	m_definition = ItemDefinition::GetItemDefinition(typeName);
	ASSERT_OR_DIE(m_definition != nullptr, "Attempted to create item of invalid item definition.");

	m_stats = Stats::CalculateRandomStatsInRange(m_definition->m_minStats, m_definition->m_maxStats);

	m_damageTypes.SetTags(m_definition->m_damageTypeString);
}

Item::~Item()
{

}

int Item::CalculateTotalStatModifier() const
{
	return m_stats[STAT_STRENGTH] + m_stats[STAT_AGILITY] + m_stats[STAT_MAGIC] + m_stats[STAT_ENDURANCE] + m_stats[STAT_LUCK] + m_stats[STAT_MAX_HP];
}

void Item::Use()
{
	
}

