#pragma once
#include <map>
#include <string>
#include "Game/Character.hpp"
#include "ThirdParty/XMLParser/XMLParser.hpp"


class CharacterBuilder
{
public:
	CharacterBuilder(XMLNode element);

	static Character* BuildNewCharacter(std::string characterTypeName);

public:
	std::string m_name;
	Stats m_minStats;
	Stats m_maxStats;
	std::string m_faction;

	char m_glyph;
	Rgba m_glyphColor;
	Rgba m_fillColor;

	std::vector<Behavior*> m_behaviors;
	std::vector<std::string> m_loot;
	std::map<std::string, float> m_gCostBiases;
	std::string m_tagsToSet;
	std::vector<std::string> m_damageTypeWeaknesses;
	std::vector<std::string> m_damageTypeResistances;
	std::vector<std::string> m_damageTypeImmunities;

	static std::map<std::string, CharacterBuilder*> s_registry;
private:
	static std::vector<Behavior*> CloneBehaviors(std::vector<Behavior*> behaviorsToClone);
};