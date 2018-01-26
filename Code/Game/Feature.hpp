#pragma once
#include "Game/Entity.hpp"

enum ExitDirection
{
	DIRECTION_NEXT,
	DIRECTION_PREVIOUS
};

struct ExitData
{
	std::string m_destinationTileType;
	ExitDirection m_direction;
	bool m_isTwoWay;
	Tile* m_destinationTile;
};

class Feature : public Entity
{
public:
	Feature(XMLNode element);
	Feature(std::string typeName);
	~Feature();

	virtual std::vector<Message> GetTooltipInfo() const override;

	bool m_isSolid;
	bool m_isExit;
	ExitData m_exitData;


	static std::map<std::string, Feature*> s_registry;
private:
	ExitData ParseExitData(XMLNode exitNode);
};
