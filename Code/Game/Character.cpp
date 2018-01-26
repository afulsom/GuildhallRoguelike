#include "Game/Character.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineConfig.hpp"
#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Character::Character()
	: Entity()
	, m_turnsUntilAction(1)
	, m_currentBehavior(nullptr)
	, m_behaviors()
	, m_currentHP(0)
	, m_faction()
	, m_equipment()
	, m_gCostBiases()
	, m_visibleCharacters()
	, m_tags()
{

}

Character::~Character()
{

}

void Character::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (m_turnsUntilAction <= 0)
		Act();
}

void Character::AdvanceTurn()
{
	--m_turnsUntilAction;
}

void Character::Act()
{
	float maxUtility = -1.f;
	for (size_t behaviorIndex = 0; behaviorIndex < m_behaviors.size(); behaviorIndex++)
	{
		float behaviorUtility = m_behaviors[behaviorIndex]->CalcUtility(this);
		if (behaviorUtility > maxUtility)
		{
			maxUtility = behaviorUtility;
			m_currentBehavior = m_behaviors[behaviorIndex];
		}
	}

	m_currentBehavior->Act(this);
}

void Character::Rest()
{
	
}

void Character::MoveNorth()
{
	m_currentMap->TryToMoveCharacterToTile(this, m_currentTile->GetNorthNeighbor());
}

void Character::MoveSouth()
{
	m_currentMap->TryToMoveCharacterToTile(this, m_currentTile->GetSouthNeighbor());
}

void Character::MoveEast()
{
	m_currentMap->TryToMoveCharacterToTile(this, m_currentTile->GetEastNeighbor());
}

void Character::MoveWest()
{
	m_currentMap->TryToMoveCharacterToTile(this, m_currentTile->GetWestNeighbor());
}

void Character::Attack(Character* attackedCharacter)
{
	if (m_faction == attackedCharacter->m_faction)
		return;

	Stats modifiedAttackerStats = m_stats + m_equipment.CalculateCombinedStatModifiers();
	Stats modifiedDefenderStats = m_stats + m_equipment.CalculateCombinedStatModifiers();

	//Determine if hit
	float chanceToHit = BASE_CHANCE_TO_HIT + ((modifiedAttackerStats[STAT_AGILITY] - modifiedDefenderStats[STAT_AGILITY]) * CHANCE_TO_HIT_PER_AGILITY);
	if (GetRandomFloatZeroToOne() > chanceToHit)
		return;					//Miss, no damage done

	//Calculate base damage dealt
	int damageToDeal = (2 * modifiedAttackerStats[STAT_STRENGTH]) - modifiedDefenderStats[STAT_ENDURANCE];

	//Determine critical
	float criticalChance = BASE_CRITICAL_CHANCE + ((modifiedAttackerStats[STAT_LUCK] - modifiedDefenderStats[STAT_LUCK]) * CRITICAL_CHANCE_PER_LUCK);
	if (GetRandomFloatZeroToOne() <= criticalChance)
		damageToDeal = (int)floor(CRITICAL_MULTIPLIER * damageToDeal);

	Tags damageTypes;
	if (m_equipment.m_equippedItems[EQUIP_SLOT_PRIMARY_WEAPON])
		damageTypes = m_equipment.m_equippedItems[EQUIP_SLOT_PRIMARY_WEAPON]->m_damageTypes;

	attackedCharacter->ApplyDamage(damageToDeal, damageTypes);
}

void Character::PickupItemsInCurrentTile()
{
	if (!m_currentTile->m_tileInventory.IsEmpty())
	{
		for (Item* item : m_currentTile->m_tileInventory.m_items)
		{
			m_entityInventory.AddItem(item);
			if (item->m_definition->m_slot != EQUIP_SLOT_NONE)
			{
				if (m_equipment.m_equippedItems[item->m_definition->m_slot] == nullptr)
					m_equipment.m_equippedItems[item->m_definition->m_slot] = item;
				else if (m_equipment.m_equippedItems[item->m_definition->m_slot]->CalculateTotalStatModifier() < item->CalculateTotalStatModifier())
					m_equipment.m_equippedItems[item->m_definition->m_slot] = item;
			}
		}
	}

	m_currentTile->m_tileInventory.m_items.clear();
}

std::set<Tile*> Character::GetVisibleTiles()
{
	std::set<Tile*> visibleTiles;
	int numRaycasts = 256;
	for (int raycastIndex = 0; raycastIndex < numRaycasts; raycastIndex++)
	{
		float theta = 360.f / numRaycasts;
		Vector2 directionVector;
		directionVector.SetUnitLengthAndHeadingDegrees(theta * raycastIndex);
		RaycastResult result = m_currentMap->RaycastForOpaque((Vector2)m_currentTile->m_tileCoords + Vector2(0.5f, 0.5f), directionVector, 15.f);
		for (Tile* tile : result.m_impactedTiles)
		{
			visibleTiles.insert(tile);
		}
	}
	return visibleTiles;
}

void Character::UpdateVisibleActors()
{
	m_visibleCharacters.clear();

	std::vector<Character*> potentialCharacters = m_currentMap->FindAllCharacters();
	for (Character* otherCharacter : potentialCharacters)
	{
		if(otherCharacter != this)
		{
			Vector2 displacementToPotentialTarget = otherCharacter->m_currentTile->m_tileCoords - m_currentTile->m_tileCoords;
			RaycastResult result = m_currentMap->RaycastForOpaque(Vector2(m_currentTile->m_tileCoords) + Vector2(0.5f, 0.5f), displacementToPotentialTarget.GetNormalized(), displacementToPotentialTarget.CalcLength());
			if (!result.m_didImpact)
			{
				m_visibleCharacters.insert(otherCharacter);
				otherCharacter->m_visibleCharacters.insert(this);
			}
		}
	}
}

std::vector<Message> Character::GetTooltipInfo() const
{
	std::vector<Message> outputInfo;
	outputInfo.push_back(Message(m_name, m_glyphColor, 1.f));

	if(!m_tags.GetTagsAsString().empty())
		outputInfo.push_back(Message(" " + m_tags.GetTagsAsString(), Rgba::WHITE, 0.6f));

	if (!m_damageTypeWeaknesses.empty())
	{
		std::string weaknessString;
		for (std::string weakness : m_damageTypeWeaknesses)
		{
			weaknessString += weakness;
			weaknessString += ", ";
		}
		weaknessString.erase(weaknessString.end() - 2, weaknessString.end() - 1);
		outputInfo.push_back(Message("  Weak: " + weaknessString, Rgba::WHITE, 0.5f));
	}

	if(!m_damageTypeResistances.empty())
	{
		std::string resistanceString;
		for (std::string resistance : m_damageTypeResistances)
		{
			resistanceString += resistance;
			resistanceString += ", ";
		}
		resistanceString.erase(resistanceString.end() - 2, resistanceString.end() - 1);
		outputInfo.push_back(Message("  Res: " + resistanceString, Rgba::WHITE, 0.5f));
	}

	if(!m_damageTypeImmunities.empty())
	{
		std::string immunityString;
		for (std::string immunity : m_damageTypeImmunities)
		{
			immunityString += immunity;
			immunityString += ", ";
		}
		immunityString.erase(immunityString.end() - 2, immunityString.end() - 1);
		outputInfo.push_back(Message("  Null: " + immunityString, Rgba::WHITE, 0.5f));
	}

	if(m_currentBehavior)
		outputInfo.push_back(Message(" Behavior: " + m_currentBehavior->GetName(), Rgba::WHITE, 0.5f));

	if(!m_entityInventory.IsEmpty())
	{
		outputInfo.push_back(Message(" Items:", Rgba::WHITE, 0.5f));
		for (Item* item : m_entityInventory.m_items)
		{
			outputInfo.push_back(Message("  " + item->m_definition->m_name, Rgba::WHITE, 0.35f));
			if(!item->m_damageTypes.GetTagsAsString().empty())
				outputInfo.push_back(Message("   Elements: " + item->m_damageTypes.GetTagsAsString(), Rgba::WHITE, 0.3f));
		}
	}

	return outputInfo;
}

float Character::GetGCostBias(std::string tileType) const
{
	std::map<std::string, float>::const_iterator found = m_gCostBiases.find(tileType);
	if (found == m_gCostBiases.end())
		return 0.f;

	return found->second;
}

void Character::ApplyDamage(int damageToDeal, const Tags& damageTypes)
{
	float damageModifier = 1.f;
	for (std::string weaknessTag : m_damageTypeWeaknesses)
	{
		if (damageTypes.MatchTags(weaknessTag))
			damageModifier *= 2.f;
	}

	for (std::string resistanceTag : m_damageTypeResistances)
	{
		if (damageTypes.MatchTags(resistanceTag))
			damageModifier *= 0.5f;
	}

	for (std::string immunityTag : m_damageTypeImmunities)
	{
		if (damageTypes.MatchTags(immunityTag))
		{
			damageModifier *= 0.f;
			break;
		}
	}

	damageToDeal = (int)floor((float)damageToDeal * damageModifier);

	m_currentHP -= damageToDeal;
	m_currentMap->m_damageNumbers.push_back(DamageNumber(std::to_string(damageToDeal), Vector2(m_currentTile->m_tileCoords) + Vector2(0.5f, 0.75f), Rgba::RED, ClampFloat(damageModifier * 0.5f, 0.4f, 1.25f)));
	if (m_currentHP <= 0)
	{
		if (g_theApp->m_game->m_theWorld->m_thePlayer == this)
			g_theApp->m_game->m_theWorld->m_thePlayer = nullptr;

		m_currentMap->DestroyCharacter(this);
	}
}
