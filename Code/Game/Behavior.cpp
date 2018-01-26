#include "Game/Behavior.hpp"
#include <string>
#include "Game/WanderBehavior.hpp"
#include "Game/PursueBehavior.hpp"
#include "Game/FleeBehavior.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineConfig.hpp"
#include "Game/AttackBehavior.hpp"
#include "Game/PatrolBehavior.hpp"


Behavior::Behavior()
{

}

Behavior::~Behavior()
{

}

float Behavior::CalcUtility(Character* actingCharacter) const
{
	UNUSED(actingCharacter);
	return 0.f;
}

Behavior* Behavior::Create(XMLNode element)
{
	std::string elementName = element.getName();

	if (elementName == "Wander")
		return new WanderBehavior(element);

	if (elementName == "Pursue")
		return new PursueBehavior(element);

	if (elementName == "Flee")
		return new FleeBehavior(element);

	if (elementName == "Attack")
		return new AttackBehavior(element);

	if (elementName == "Patrol")
		return new PatrolBehavior(element);

	ERROR_AND_DIE("Invalid behavior name.");
}
