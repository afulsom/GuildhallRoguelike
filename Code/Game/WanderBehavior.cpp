#include "Game/WanderBehavior.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/EngineConfig.hpp"

WanderBehavior::WanderBehavior(XMLNode element)
	: m_wanderTarget(nullptr)
{
	m_baseUtility = ParseXMLAttributeFloat(element, "utility", m_baseUtility);
}


WanderBehavior::~WanderBehavior()
{

}

void WanderBehavior::Act(Character* actingCharacter)
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

	if (!m_wanderTarget || actingCharacter->m_currentTile == m_wanderTarget)
	{
		//generate new target
		m_wanderTarget = actingCharacter->m_currentMap->GetRandomTraversableTile();
		m_wanderPath = actingCharacter->m_currentMap->GeneratePath(actingCharacter->m_currentTile->m_tileCoords, m_wanderTarget->m_tileCoords, actingCharacter);
	}

	if(!m_wanderPath.empty())
	{
		Tile* nextTile = *(m_wanderPath.end() - 1);
		bool successfullyMoved = actingCharacter->m_currentMap->TryToMoveCharacterToTile(actingCharacter, nextTile);
		if (successfullyMoved)
			m_wanderPath.pop_back();
	}

	actingCharacter->m_turnsUntilAction = 1;
}

float WanderBehavior::CalcUtility(Character* actingCharacter) const
{
	UNUSED(actingCharacter);
	return m_baseUtility;
}

std::string WanderBehavior::GetName() const
{
	return "Wander";
}

void WanderBehavior::DebugRender(const Character* actingCharacter) const
{
	UNUSED(actingCharacter);
	for (size_t tileIndex = 1; tileIndex < m_wanderPath.size(); tileIndex++)
	{
		Tile* tile = m_wanderPath[tileIndex];
		g_theRenderer->DrawCenteredText2D((Vector2)tile->m_tileCoords + Vector2(0.5f, 0.5f), g_theRenderer->m_defaultFont, "p", Rgba::BLUE, 0.5f);
	}
	if(m_wanderTarget)
		g_theRenderer->DrawCenteredText2D((Vector2)m_wanderTarget->m_tileCoords + Vector2(0.5f, 0.5f), g_theRenderer->m_defaultFont, "T", Rgba::RED, 0.5f);
}

Behavior* WanderBehavior::Clone()
{
	return new WanderBehavior(*this);
}

