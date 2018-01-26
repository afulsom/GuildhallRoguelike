#pragma once
#include "Game/Behavior.hpp"
#include "Game/Map.hpp"



class PursueBehavior : public Behavior
{
public:
	PursueBehavior(XMLNode element);
	virtual ~PursueBehavior();
	PursueBehavior(PursueBehavior* behaviorToCopy);

	virtual void Act(Character* actingCharacter) override;
	virtual float CalcUtility(Character* actingCharacter) const override;
	virtual std::string GetName() const override;
	virtual void DebugRender(const Character* actingCharacter) const override;

	virtual Behavior* Clone() override;

	float m_utility = 0.5f;
	Path m_pursuitPath;
};