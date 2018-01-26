#include "Game/PursueBehavior.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/EngineConfig.hpp"

PursueBehavior::PursueBehavior(XMLNode element)
	: m_pursuitPath()
{
	m_utility = ParseXMLAttributeFloat(element, "utility", m_utility);
}

PursueBehavior::PursueBehavior(PursueBehavior* behaviorToCopy)
	: m_pursuitPath()
{
	m_utility = behaviorToCopy->m_utility;
}

PursueBehavior::~PursueBehavior()
{

}

void PursueBehavior::Act(Character* actingCharacter)
{
	if(actingCharacter->m_target)
	{
		if (m_pursuitPath.empty() || actingCharacter->m_target->m_currentTile->m_tileCoords != m_pursuitPath[0]->m_tileCoords)
		{
			m_pursuitPath = actingCharacter->m_currentMap->GeneratePath(actingCharacter->m_currentTile->m_tileCoords, actingCharacter->m_target->m_currentTile->m_tileCoords, actingCharacter);
		}

		Tile* nextTile = *(m_pursuitPath.end() - 1);
		bool successfullyMoved = actingCharacter->m_currentMap->TryToMoveCharacterToTile(actingCharacter, nextTile);
		if (successfullyMoved)
			m_pursuitPath.pop_back();
	}

	actingCharacter->m_turnsUntilAction = 1;
}


float PursueBehavior::CalcUtility(Character* actingCharacter) const
{
	if (actingCharacter->m_target)
		return m_utility;
	return -1.f;
}

std::string PursueBehavior::GetName() const
{
	return "Pursue";
}

void PursueBehavior::DebugRender(const Character* actingCharacter) const
{
	if(actingCharacter->m_target)
		g_theRenderer->DrawLine2D((Vector2)actingCharacter->m_currentTile->m_tileCoords + Vector2(0.5f, 0.5f), (Vector2)actingCharacter->m_target->m_currentTile->m_tileCoords + Vector2(0.5f, 0.5f), 0.125f, Rgba::WHITE, Rgba::RED);

	for (Tile* tile : m_pursuitPath)
	{
		g_theRenderer->DrawCenteredText2D((Vector2)tile->m_tileCoords + Vector2(0.5f, 0.5f), g_theRenderer->m_defaultFont, "p", Rgba::BLUE, 0.5f);
	}
}

Behavior* PursueBehavior::Clone()
{
	PursueBehavior* outBehavior = new PursueBehavior(this);
	return outBehavior;
}

