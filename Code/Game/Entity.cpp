#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineConfig.hpp"

Entity::Entity()
	: m_currentMap(nullptr)
	, m_currentTile(nullptr)
	, m_entityInventory()
	, m_fillColor(Rgba(0, 0, 0, 0))
	, m_glyph(' ')
	, m_glyphColor(Rgba(0, 0, 0, 0))
	, m_name("")
{

}

Entity::~Entity()
{

}

void Entity::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void Entity::AdvanceTurn()
{

}

void Entity::Render() const
{

}


