#include "Game/AttackBehavior.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/EngineConfig.hpp"

AttackBehavior::AttackBehavior(XMLNode element)
{
	m_utility = ParseXMLAttributeFloat(element, "utility", m_utility);
}

AttackBehavior::AttackBehavior(AttackBehavior* behaviorToCopy)
{
	m_utility = behaviorToCopy->m_utility;
}

AttackBehavior::~AttackBehavior()
{

}

void AttackBehavior::Act(Character* actingCharacter)
{
	actingCharacter->Attack(actingCharacter->m_target);

	actingCharacter->m_turnsUntilAction = 1;
}


float AttackBehavior::CalcUtility(Character* actingCharacter) const
{
	if (actingCharacter->m_target && actingCharacter->m_currentMap->CalculateManhattanDistance(*actingCharacter->m_currentTile, *actingCharacter->m_target->m_currentTile) <= 1)
		return m_utility;
	return -1.f;
}

std::string AttackBehavior::GetName() const
{
	return "Attack";
}

void AttackBehavior::DebugRender(const Character* actingCharacter) const
{
	if(actingCharacter->m_target)
		g_theRenderer->DrawLine2D((Vector2)actingCharacter->m_currentTile->m_tileCoords + Vector2(0.5f, 0.5f), (Vector2)actingCharacter->m_target->m_currentTile->m_tileCoords + Vector2(0.5f, 0.5f), 0.125f, Rgba::WHITE, Rgba::RED);
}

Behavior* AttackBehavior::Clone()
{
	AttackBehavior* outBehavior = new AttackBehavior(this);
	return outBehavior;
}

