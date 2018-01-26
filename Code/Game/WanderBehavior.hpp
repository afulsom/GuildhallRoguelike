#pragma once
#include "Game/Behavior.hpp"
#include "Game/Map.hpp"

class Tile;

class WanderBehavior : public Behavior
{
public:
	WanderBehavior(XMLNode element);
	virtual ~WanderBehavior();

	Tile* m_wanderTarget;
	Path m_wanderPath;
	float m_baseUtility = 0.3f;

	virtual void Act(Character* actingCharacter) override;
	virtual float CalcUtility(Character* actingCharacter) const override;
	virtual std::string GetName() const override;
	virtual void DebugRender(const Character* actingCharacter) const override;

	virtual Behavior* Clone() override;
};