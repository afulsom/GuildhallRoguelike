#pragma once
#include "Game/Behavior.hpp"
#include "Game/Map.hpp"

class Tile;

class PatrolBehavior : public Behavior
{
public:
	PatrolBehavior(XMLNode element);
	virtual ~PatrolBehavior();

	Tile* m_patrolTarget;
	Path m_patrolPath;
	std::string m_patrolPointTags;
	float m_baseUtility = 0.3f;

	virtual void Act(Character* actingCharacter) override;
	virtual float CalcUtility(Character* actingCharacter) const override;
	virtual std::string GetName() const override;
	virtual void DebugRender(const Character* actingCharacter) const override;

	virtual Behavior* Clone() override;
};