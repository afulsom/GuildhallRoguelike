#include "Game/Tile.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineConfig.hpp"
#include "Engine/Renderer/RHI/SimpleRenderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include <string>

Tile::Tile()
	: m_tileCoords(0, 0)
	, m_containingMap(nullptr)
	, m_occupyingFeature(nullptr)
	, m_occupyingCharacter(nullptr)
	, m_tileInventory()
	, m_tileDefinition(nullptr)
	, m_permanence(0.f)
	, m_tags()
{

}

Tile::Tile(std::string tileTypeName)
	: m_tileCoords(0, 0)
	, m_containingMap(nullptr)
	, m_occupyingFeature(nullptr)
	, m_occupyingCharacter(nullptr)
	, m_tileInventory()
	, m_permanence(0.f)
	, m_tags()
{
	TileDefinition* tileDefinition = TileDefinition::GetTileDefinition(tileTypeName);
	if (tileDefinition == nullptr)
		ERROR_AND_DIE("INVALID TILE DEFINITION USED.");

	m_tileDefinition = tileDefinition;
	m_glyph = m_tileDefinition->m_glyphs[GetRandomIntLessThan(m_tileDefinition->m_glyphs.size())];
	m_glyphColor = m_tileDefinition->m_glyphColors[GetRandomIntLessThan(m_tileDefinition->m_glyphColors.size())];
	m_fillColor = m_tileDefinition->m_fillColors[GetRandomIntLessThan(m_tileDefinition->m_fillColors.size())];
}

Tile::~Tile()
{

}

void Tile::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (m_tileDefinition == nullptr)
		ERROR_AND_DIE("TILE HAS NO TYPE.");
}

void Tile::Render() const
{
	if (m_tileDefinition == nullptr)
		ERROR_AND_DIE("TILE HAS NO TYPE.");

	if (!m_hasBeenSeenByPlayer)
	{
		g_theRenderer->SetTexture(nullptr);
		g_theRenderer->DrawQuad2D((float)m_tileCoords.x, (float)m_tileCoords.y, 1.f, 1.f, Rgba::BLACK);
		return;
	}

	float glyphScale = 0.75f;
	AABB2 glyphBounds(m_tileCoords.x + 0.125f, m_tileCoords.y + 0.125f, m_tileCoords.x + 0.875f, m_tileCoords.y + 0.875f);

	if(m_isVisibleToPlayer)
	{
		if (m_occupyingCharacter != nullptr)
		{
			g_theRenderer->SetTexture(nullptr);
			g_theRenderer->DrawQuad2D((float)m_tileCoords.x, (float)m_tileCoords.y, 1.f, 1.f, m_fillColor);
			g_theRenderer->DrawTextInAABB2(glyphBounds, g_theRenderer->m_defaultFont, std::string(1, m_occupyingCharacter->m_glyph), m_occupyingCharacter->m_glyphColor, glyphScale);
		}
		else if (m_occupyingFeature != nullptr)
		{
			g_theRenderer->SetTexture(nullptr);
			g_theRenderer->DrawQuad2D((float)m_tileCoords.x, (float)m_tileCoords.y, 1.f, 1.f, m_fillColor);
			g_theRenderer->DrawTextInAABB2(glyphBounds, g_theRenderer->m_defaultFont, std::string(1, m_occupyingFeature->m_glyph), m_occupyingFeature->m_glyphColor, glyphScale);
		}
		else if (!m_tileInventory.IsEmpty())
		{
			g_theRenderer->SetTexture(nullptr);
			g_theRenderer->DrawQuad2D((float)m_tileCoords.x, (float)m_tileCoords.y, 1.f, 1.f, m_fillColor);
			g_theRenderer->DrawTextInAABB2(glyphBounds, g_theRenderer->m_defaultFont, std::string(1, m_tileInventory.m_items[0]->m_definition->m_glyph), m_tileInventory.m_items[0]->m_definition->m_glyphColor, glyphScale);
		}
		else
		{
			g_theRenderer->SetTexture(nullptr);
			g_theRenderer->DrawQuad2D((float)m_tileCoords.x, (float)m_tileCoords.y, 1.f, 1.f, m_fillColor);
			g_theRenderer->DrawTextInAABB2(glyphBounds, g_theRenderer->m_defaultFont, std::string(1, m_glyph), m_glyphColor, glyphScale);
		}
	}
	else
	{
		g_theRenderer->SetTexture(nullptr);
		g_theRenderer->DrawQuad2D((float)m_tileCoords.x, (float)m_tileCoords.y, 1.f, 1.f, m_fillColor - Rgba(45, 45, 45, 0));
		g_theRenderer->DrawTextInAABB2(glyphBounds, g_theRenderer->m_defaultFont, std::string(1, m_glyph), m_glyphColor - Rgba(45, 45, 45, 0), glyphScale);
	}
}

void Tile::ChangeType(std::string tileTypeName)
{
	TileDefinition* tileDefinition = TileDefinition::GetTileDefinition(tileTypeName);
	if (tileDefinition == nullptr)
		ERROR_AND_DIE("INVALID TILE DEFINITION USED.");

	m_tileDefinition = tileDefinition;
	m_glyph = m_tileDefinition->m_glyphs[GetRandomIntLessThan(m_tileDefinition->m_glyphs.size())];
	m_glyphColor = m_tileDefinition->m_glyphColors[GetRandomIntLessThan(m_tileDefinition->m_glyphColors.size())];
	m_fillColor = m_tileDefinition->m_fillColors[GetRandomIntLessThan(m_tileDefinition->m_fillColors.size())];
}

Tile* Tile::GetNorthNeighbor() const
{
	return m_containingMap->GetTileAtTileCoords(m_tileCoords + IntVector2(0, 1));
}

Tile* Tile::GetSouthNeighbor() const
{
	return m_containingMap->GetTileAtTileCoords(m_tileCoords + IntVector2(0, -1));
}

Tile* Tile::GetEastNeighbor() const
{
	return m_containingMap->GetTileAtTileCoords(m_tileCoords + IntVector2(1, 0));
}

Tile* Tile::GetWestNeighbor() const
{
	return m_containingMap->GetTileAtTileCoords(m_tileCoords + IntVector2(-1, 0));
}

Tile* Tile::GetNorthEastNeighbor() const
{
	return m_containingMap->GetTileAtTileCoords(m_tileCoords + IntVector2(1, 1));
}

Tile* Tile::GetNorthWestNeighbor() const
{
	return m_containingMap->GetTileAtTileCoords(m_tileCoords + IntVector2(-1, 1));
}

Tile* Tile::GetSouthEastNeighbor() const
{
	return m_containingMap->GetTileAtTileCoords(m_tileCoords + IntVector2(1, -1));
}

Tile* Tile::GetSouthWestNeighbor() const
{
	return m_containingMap->GetTileAtTileCoords(m_tileCoords + IntVector2(-1, -1));
}

bool Tile::IsSolidToTags(const Tags& tagsToCheck) const
{
	if (m_tileDefinition->m_solidExceptions.empty())
		return m_tileDefinition->m_isSolid;

	if (tagsToCheck.MatchTags(m_tileDefinition->m_solidExceptions))
		return !m_tileDefinition->m_isSolid;
	else
		return m_tileDefinition->m_isSolid;
}

float Tile::GetGCost() const
{
	return 1.f;
}

std::vector<Message> Tile::GetTooltipInfo() const
{
	std::vector<Message> outputInfo;

	if (m_isVisibleToPlayer && m_occupyingCharacter)
	{
		std::vector<Message> characterInfo = m_occupyingCharacter->GetTooltipInfo();
		for (size_t characterInfoIndex = 0; characterInfoIndex < characterInfo.size(); characterInfoIndex++)
		{
			outputInfo.push_back(characterInfo[characterInfoIndex]);
		}
	}

	if (m_isVisibleToPlayer && m_occupyingFeature)
	{
		std::vector<Message> featureInfo = m_occupyingFeature->GetTooltipInfo();
		for (size_t featureInfoIndex = 0; featureInfoIndex < featureInfo.size(); featureInfoIndex++)
		{
			outputInfo.push_back(featureInfo[featureInfoIndex]);
		}
	}

	outputInfo.push_back(Message(m_tileDefinition->m_name, m_glyphColor, 1.f));
	std::string solidOpaqueString;
	if(m_tileDefinition->m_isSolid)
		outputInfo.push_back(Message(" solid", Rgba::LIGHT_GREY, 0.5f));

	if (!m_tags.GetTagsAsString().empty())
		outputInfo.push_back(Message("  " + m_tags.GetTagsAsString(), Rgba::WHITE, 0.5f));


	if(m_isVisibleToPlayer)
	{
		for (Item* item : m_tileInventory.m_items)
		{
			outputInfo.push_back(Message("  " + item->m_definition->m_name, Rgba::WHITE, 0.5f));
		}
	}

	return outputInfo;
}
