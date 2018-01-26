#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Disc2D.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Game/Inventory.hpp"
#include "Game/Message.hpp"

class Map;
class Tile;

class Entity
{
public:
	Entity();
	virtual ~Entity();
	
	virtual void Update(float deltaSeconds);
	virtual void AdvanceTurn();
	virtual void Render() const;

	virtual std::vector<Message> GetTooltipInfo() const = 0;

public:
	std::string m_name;
	Inventory m_entityInventory;
	Map* m_currentMap;
	Tile* m_currentTile;

	char m_glyph;
	Rgba m_glyphColor;
	Rgba m_fillColor;	
};