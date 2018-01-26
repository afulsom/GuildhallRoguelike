#pragma once
#include "Game/TileDefinition.hpp"
#include "Game/Inventory.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Game/Character.hpp"
#include "Game/Feature.hpp"
#include "Game/Message.hpp"
#include <string>




class Tile
{
public:
	Tile();
	Tile(std::string tileTypeName);
	~Tile();

	void Update(float deltaSeconds);
	void Render() const;

	void ChangeType(std::string tileTypeName);

	std::vector<Message> GetTooltipInfo() const;

	Tile* GetNorthNeighbor() const;
	Tile* GetSouthNeighbor() const;
	Tile* GetEastNeighbor() const;
	Tile* GetWestNeighbor() const;
	Tile* GetNorthEastNeighbor() const;
	Tile* GetNorthWestNeighbor() const;
	Tile* GetSouthEastNeighbor() const;
	Tile* GetSouthWestNeighbor() const;

	bool IsSolidToTags(const Tags& tagsToCheck) const;
	float GetGCost() const;
public:
	TileDefinition* m_tileDefinition;
	char m_glyph;
	Rgba m_glyphColor;
	Rgba m_fillColor;

	Map* m_containingMap;
	IntVector2 m_tileCoords;
	Character* m_occupyingCharacter;
	Feature* m_occupyingFeature;
	Inventory m_tileInventory;

	Tags m_tags;

	bool m_isVisibleToPlayer = false;
	bool m_hasBeenSeenByPlayer = false;

	int m_isClosedInPathID = 0;
	int m_isOpenInPathID = 0;
	float m_permanence;
};

