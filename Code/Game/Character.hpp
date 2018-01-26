#pragma once
#include "Game/Entity.hpp"
#include "Game/Stats.hpp"
#include "Game/Behavior.hpp"
#include <set>
#include "Engine/Gameplay/Tags.hpp"


struct Equipment
{
	Item* m_equippedItems[NUM_EQUIP_SLOTS];

	Equipment() 
	{
		for (int itemIndex = 0; itemIndex < NUM_EQUIP_SLOTS; itemIndex++)
		{
			m_equippedItems[itemIndex] = nullptr;
		}
	}

	Stats CalculateCombinedStatModifiers() const
	{
		Stats combinedStatModifiers;
		for (Item* equippedItem : m_equippedItems)
		{
			if(equippedItem)
				combinedStatModifiers += equippedItem->m_stats;
		}

		return combinedStatModifiers;
	}

	bool IsItemEquipped(const Item* itemToCheck) const
	{
		if (itemToCheck == nullptr)
			return false;

		for (Item* equippedItem : m_equippedItems)
		{
			if (equippedItem == itemToCheck)
				return true;
		}

		return false;
	}

};


class Character : public Entity
{
public:
	Character();
	~Character();

	virtual void Update(float deltaSeconds);
	virtual void AdvanceTurn();
	virtual void Act();

	virtual std::vector<Message> GetTooltipInfo() const override;
	float GetGCostBias(std::string tileType) const;

	void Rest();
	void MoveNorth();
	void MoveSouth();
	void MoveEast();
	void MoveWest();

	void ApplyDamage(int damageToDeal, const Tags& damageTypesString);
	void Attack(Character* attackedCharacter);

	void PickupItemsInCurrentTile();

	std::set<Tile*> GetVisibleTiles();
	void UpdateVisibleActors();

	int m_turnsUntilAction;

	Equipment m_equipment;

	std::string m_faction;
	int m_currentHP;
	Stats m_stats;
	std::vector<Behavior*> m_behaviors;
	Behavior* m_currentBehavior;
	std::map<std::string, float> m_gCostBiases;
	Tags m_tags;
	std::vector<std::string> m_damageTypeWeaknesses;
	std::vector<std::string> m_damageTypeResistances;
	std::vector<std::string> m_damageTypeImmunities;

	std::set<Character*> m_visibleCharacters;
	Character* m_target = nullptr;
};
