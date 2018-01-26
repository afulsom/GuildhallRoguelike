#include "Game/PatrolBehavior.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/EngineConfig.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

PatrolBehavior::PatrolBehavior(XMLNode element)
	: m_patrolTarget(nullptr)
{
	m_baseUtility = ParseXMLAttributeFloat(element, "utility", m_baseUtility);
	m_patrolPointTags = ParseXMLAttributeString(element, "tags", "INVALID_TAGS");
	ASSERT_OR_DIE(m_patrolPointTags != "INVALID_TAGS", "Invalid or missing patrol point tags.");
}


PatrolBehavior::~PatrolBehavior()
{

}

void PatrolBehavior::Act(Character* actingCharacter)
{
	if (actingCharacter->m_target == nullptr)
	{
		std::vector<Character*> potentialTargets = actingCharacter->m_currentMap->FindAllCharactersNotOfFaction(actingCharacter->m_faction);
		for (Character* character : potentialTargets)
		{
			Vector2 displacementToPotentialTarget = character->m_currentTile->m_tileCoords - actingCharacter->m_currentTile->m_tileCoords;
			RaycastResult result = actingCharacter->m_currentMap->RaycastForOpaque(Vector2(actingCharacter->m_currentTile->m_tileCoords) + Vector2(0.5f, 0.5f), displacementToPotentialTarget.GetNormalized(), displacementToPotentialTarget.CalcLength());
			if (!result.m_didImpact)
			{
				actingCharacter->m_target = character;
				return;
			}
		}
	}

	if (!m_patrolTarget || actingCharacter->m_currentTile == m_patrolTarget)
	{
		//generate new target
		m_patrolTarget = actingCharacter->m_currentMap->GetRandomTileWithTags(m_patrolPointTags);
		m_patrolPath = actingCharacter->m_currentMap->GeneratePath(actingCharacter->m_currentTile->m_tileCoords, m_patrolTarget->m_tileCoords, actingCharacter);
	}

	if(!m_patrolPath.empty())
	{
		Tile* nextTile = *(m_patrolPath.end() - 1);
		bool successfullyMoved = actingCharacter->m_currentMap->TryToMoveCharacterToTile(actingCharacter, nextTile);
		if (successfullyMoved)
			m_patrolPath.pop_back();
		else
			m_patrolPath = actingCharacter->m_currentMap->GeneratePath(actingCharacter->m_currentTile->m_tileCoords, m_patrolTarget->m_tileCoords, actingCharacter);
	}

	actingCharacter->m_turnsUntilAction = 1;
}

float PatrolBehavior::CalcUtility(Character* actingCharacter) const
{
	UNUSED(actingCharacter);
	return m_baseUtility;
}

std::string PatrolBehavior::GetName() const
{
	return "Patrol";
}

void PatrolBehavior::DebugRender(const Character* actingCharacter) const
{
	UNUSED(actingCharacter);
	for (size_t tileIndex = 1; tileIndex < m_patrolPath.size(); tileIndex++)
	{
		Tile* tile = m_patrolPath[tileIndex];
		g_theRenderer->DrawCenteredText2D((Vector2)tile->m_tileCoords + Vector2(0.5f, 0.5f), g_theRenderer->m_defaultFont, "p", Rgba::BLUE, 0.5f);
	}
	if(m_patrolTarget)
		g_theRenderer->DrawCenteredText2D((Vector2)m_patrolTarget->m_tileCoords + Vector2(0.5f, 0.5f), g_theRenderer->m_defaultFont, "T", Rgba::RED, 0.5f);
}

Behavior* PatrolBehavior::Clone()
{
	return new PatrolBehavior(*this);
}

