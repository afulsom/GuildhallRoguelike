#include "Game/World.hpp"
#include "Engine/Core/EngineConfig.hpp"
#include "Engine/Core/ConfigSystem.hpp"
#include "ThirdParty/XMLParser/XMLParser.hpp"
#include "Game/Character.hpp"
#include "Game/CharacterBuilder.hpp"
#include "LootTable.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/MapDefinition.hpp"
#include "Adventure.hpp"


World::World()
	: m_maps()
	, m_currentMap(nullptr)
	, m_thePlayer(nullptr)
	, m_cursorPosition(Vector2(ORTHO_X_DIMENSION * 0.5f, ORTHO_Y_DIMENSION * 0.5f))
	, m_currentlyGeneratingMapDefinition(nullptr)
	, m_currentlyGeneratingMap(nullptr)
{

}

World::~World()
{

}

void World::Initialize()
{
	m_maps.resize(1);
}

Map* World::GenerateMap(std::string mapDefinitionName)
{
	StartSteppedGeneration(mapDefinitionName);
	
	FinishGeneratingMap();

	return m_currentlyGeneratingMap;
}

void World::StartSteppedGeneration(std::string mapDefinitionName)
{
	m_currentlyGeneratingMapDefinition = nullptr;
	m_currentlyGeneratingMap = nullptr;

	m_currentlyGeneratingMapDefinition = MapDefinition::GetDefinition(mapDefinitionName);
	Map* newMap = new Map(mapDefinitionName);
	m_currentlyGeneratingMap = newMap;
}

bool World::StepGeneration()
{
	bool isCompleted = m_currentlyGeneratingMapDefinition->StepGeneration(m_currentlyGeneratingMap);

	return isCompleted;
}

void World::Update(float deltaSeconds)
{
	if (g_theApp->HasFocus())
	{
		IntVector2 mouseScreenPos = g_theInput->GetCursorScreenPos();
		IntVector2 screenCenter = g_theInput->GetScreenCenter();

		IntVector2 mouseDeltaMove = mouseScreenPos - screenCenter;
		mouseDeltaMove.y *= -1;

		g_theInput->SetCursorScreenPos(screenCenter);

		m_cursorPosition += ((Vector2)mouseDeltaMove * 0.01f);
		if (m_cursorPosition.x < 0.f)
			m_cursorPosition.x = 0.f;

		if (m_cursorPosition.y < 0.f)
			m_cursorPosition.y = 0.f;
	}

	bool didPlayerAct = false;
	if(m_thePlayer && g_theApp->m_game->m_currentState == STATE_PLAYING)
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_UP))
		{
			m_thePlayer->MoveNorth();
			m_currentMap->AdvanceTurns();
			m_thePlayer->m_turnsUntilAction = 1;
			didPlayerAct = true;
		}
		else if (g_theInput->WasKeyJustPressed(KEYCODE_DOWN))
		{
			m_thePlayer->MoveSouth();
			m_currentMap->AdvanceTurns();
			m_thePlayer->m_turnsUntilAction = 1;
			didPlayerAct = true;
		}
		else if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT))
		{
			m_thePlayer->MoveWest();
			m_currentMap->AdvanceTurns();
			m_thePlayer->m_turnsUntilAction = 1;
			didPlayerAct = true;
		}
		else if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT))
		{
			m_thePlayer->MoveEast();
			m_currentMap->AdvanceTurns();
			m_thePlayer->m_turnsUntilAction = 1;
			didPlayerAct = true;
		}
		else if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
		{
			m_thePlayer->Rest();
			m_currentMap->AdvanceTurns();
			m_thePlayer->m_turnsUntilAction = 1;
			didPlayerAct = true;
		}
	}

	if(m_thePlayer)
		UpdateFogOfWar();

	m_currentMap->Update(deltaSeconds);
	if (didPlayerAct)
	{
		if(m_currentAdventure)
		{
			if (!m_hasPlayerLost && m_thePlayer == nullptr)
			{
				m_hasPlayerLost = true;
				m_currentMap->m_damageNumbers.push_back(DamageNumber(m_currentAdventure->m_defeatText, Vector2(ORTHO_X_DIMENSION * 0.5f, ORTHO_Y_DIMENSION * 0.5f), Rgba::WHITE, 4.f));
				return;
			}

			if (m_thePlayer && !m_hasPlayerWon)
			{
				CheckForVictory();
				if (m_hasPlayerWon)
					m_currentMap->m_damageNumbers.push_back(DamageNumber(m_currentAdventure->m_victoryText, Vector2(ORTHO_X_DIMENSION * 0.5f, ORTHO_Y_DIMENSION * 0.5f), Rgba::WHITE, 4.f));
			}
		}
		UpdateVisibilities();
	}
}

void World::Render() const
{
	m_currentMap->Render();
}

void World::DrawCursor() const
{
	g_theRenderer->SetTexture(nullptr);
	g_theRenderer->DrawBorderedDisc2D(m_cursorPosition, 0.1f, 0.025f, 64, Rgba(255, 255, 255, 0));
}

void World::DrawTooltip() const
{
	std::vector<Message> tooltipInfo = m_currentMap->GetTooltipInfoForMapCoords(m_cursorPosition);
	if (tooltipInfo.empty())
		return;

	Tile* hoveredTile = m_currentMap->GetTileAtMapCoords(m_cursorPosition);
	if (!hoveredTile->m_hasBeenSeenByPlayer)
		return;

	if (hoveredTile && hoveredTile->m_occupyingCharacter && hoveredTile->m_occupyingCharacter->m_currentBehavior)
	{
		hoveredTile->m_occupyingCharacter->m_currentBehavior->DebugRender(hoveredTile->m_occupyingCharacter);
	}

	g_theRenderer->SetTexture(nullptr);

	float borderThickness = 0.25f;

	float maxTextWidth = 0;
	float boxHeight = 0;
	for (size_t tooltipInfoIndex = 0; tooltipInfoIndex < tooltipInfo.size(); tooltipInfoIndex++)
	{
		float messageTextWidth = g_theRenderer->m_defaultFont->CalculateTextWidth(tooltipInfo[tooltipInfoIndex].m_text, tooltipInfo[tooltipInfoIndex].m_size);
		if (messageTextWidth > maxTextWidth)
			maxTextWidth = messageTextWidth;

		boxHeight += g_theRenderer->m_defaultFont->CalculateTextHeight(tooltipInfo[tooltipInfoIndex].m_text, tooltipInfo[tooltipInfoIndex].m_size );
	}

	Vector2 tooltipHalfExtents((maxTextWidth * 0.5f) + (borderThickness), (boxHeight * 0.5f) + (borderThickness * 1.25f));
	Vector2 tooltipCenter;
	Vector2 lineEndPoint;
	if (m_cursorPosition.x > ORTHO_X_DIMENSION * 0.5f)
	{
		tooltipCenter = Vector2(ORTHO_X_DIMENSION * 0.25f, ORTHO_Y_DIMENSION * 0.5f);
		lineEndPoint = tooltipCenter + tooltipHalfExtents;
	}
	else
	{
		tooltipCenter = Vector2(ORTHO_X_DIMENSION * 0.75f, ORTHO_Y_DIMENSION * 0.5f);
		lineEndPoint = Vector2(tooltipCenter.x - tooltipHalfExtents.x, tooltipCenter.y + tooltipHalfExtents.y);
	}

	IntVector2 tileCoords = m_currentMap->GetTileAtMapCoords(m_cursorPosition)->m_tileCoords;
	IntVector2 lineStartPoint = tileCoords + IntVector2(1, 1);
	g_theRenderer->DrawLine2D(lineStartPoint, lineEndPoint, 0.05f, Rgba::BLACK, Rgba::BLACK);
	g_theRenderer->DrawBorderedQuad2D(AABB2(tileCoords, lineStartPoint), 0.05f, Rgba(0, 0, 0, 0), Rgba::BLACK);
	g_theRenderer->DrawBorderedQuad2D(AABB2(tooltipCenter, tooltipHalfExtents.x, tooltipHalfExtents.y), borderThickness, Rgba(0, 0, 0, 127), Rgba::BLACK);

	Vector2 linePosition = Vector2(tooltipCenter.x - tooltipHalfExtents.x + borderThickness, tooltipCenter.y + tooltipHalfExtents.y - (borderThickness * 0.25f));
	float lineSpacer = 0.05f;
	for (size_t tooltipInfoIndex = 0; tooltipInfoIndex < tooltipInfo.size(); tooltipInfoIndex++)
	{
		g_theRenderer->DrawText2D(linePosition, g_theRenderer->m_defaultFont, tooltipInfo[tooltipInfoIndex].m_text, tooltipInfo[tooltipInfoIndex].m_color, tooltipInfo[tooltipInfoIndex].m_size);
		linePosition.y -= g_theRenderer->m_defaultFont->CalculateTextHeight(tooltipInfo[tooltipInfoIndex].m_text, tooltipInfo[tooltipInfoIndex].m_size);
		linePosition.y -= lineSpacer;
	}
}

void World::DrawUI() const
{
	g_theRenderer->SetTexture(nullptr);

	float orthoWidth = ORTHO_X_DIMENSION;
	float orthoHeight = ORTHO_Y_DIMENSION;

	g_theConfig->GetConfigFloat(orthoWidth, "ORTHO_WIDTH");
	g_theConfig->GetConfigFloat(orthoHeight, "ORTHO_HEIGHT");

	g_theRenderer->SetProjectionMatrix(g_theRenderer->CreateOrthoProjectionMatrix(Vector2::ZERO, Vector2(orthoWidth, orthoHeight)));
	g_theRenderer->DrawText2D(Vector2(0.5f, ORTHO_Y_DIMENSION), g_theRenderer->m_defaultFont, "ESC: Open menu.", Rgba::LIGHT_GREY, 0.6f);

	if(m_thePlayer)
		g_theRenderer->DrawText2D(Vector2(0.5f, ORTHO_Y_DIMENSION - 1.f), g_theRenderer->m_defaultFont, "HP: " + std::to_string(m_thePlayer->m_currentHP) + "/" + std::to_string(m_thePlayer->m_stats[STAT_MAX_HP]), Rgba::WHITE, 0.75f);
	DrawTooltip();
}

void World::GenerateAdventure(std::string adventureName)
{
	m_currentAdventure = Adventure::GetDefinition(adventureName);

	//Generate maps
	for (std::map<std::string, AdventureMap>::iterator mapIter = m_currentAdventure->m_maps.begin(); mapIter != m_currentAdventure->m_maps.end(); mapIter++)
	{
		Map* tempMap = GenerateMap(mapIter->second.m_definition->m_name);
		tempMap->m_name = mapIter->second.m_name;

		std::vector<AdventureItem> itemsToSpawn = mapIter->second.m_itemsToSpawn;
		for (AdventureItem item : itemsToSpawn)
		{
			Item* newItem = new Item(item.m_definition);
			tempMap->PlaceItemInMap(newItem, tempMap->GetRandomTileOfType(item.m_tileTypeToSpawnOn));
		}

		std::vector<AdventureCharacter> charactersToSpawn = mapIter->second.m_charactersToSpawn;
		for (AdventureCharacter character : charactersToSpawn)
		{
			Character* newCharacter = CharacterBuilder::BuildNewCharacter(character.m_builderName);
			tempMap->PlaceCharacterInMap(newCharacter, tempMap->GetRandomTileOfType(character.m_tileTypeToSpawnOn));
		}

		m_maps.push_back(tempMap);
	}

	//Place exits
	for (size_t mapIndex = 0; mapIndex < m_maps.size(); mapIndex++)
	{
		AdventureMap currentAdventureMap = m_currentAdventure->m_maps[m_maps[mapIndex]->m_name];
		for (size_t exitIndex = 0; exitIndex < currentAdventureMap.m_exitsToSpawn.size(); exitIndex++)
		{
			Feature* newExit = new Feature(currentAdventureMap.m_exitsToSpawn[exitIndex].m_exitType);

			//Get destination map
			Map* destinationMap = nullptr;
			for (size_t findMapIndex = 0; findMapIndex < m_maps.size(); findMapIndex++)
			{
				if (m_maps[findMapIndex]->m_name == currentAdventureMap.m_exitsToSpawn[exitIndex].m_destinationMapName)
				{
					destinationMap = m_maps[findMapIndex];
					break;
				}
			}
			ASSERT_OR_DIE(destinationMap != nullptr, "Invalid destination map name in exit.");


			Tile* destinationTile;
			if (currentAdventureMap.m_exitsToSpawn[exitIndex].m_destinationTileType.empty())
				destinationTile = destinationMap->GetRandomTraversableTile();
			else
				destinationTile = destinationMap->GetRandomTileOfType(currentAdventureMap.m_exitsToSpawn[exitIndex].m_destinationTileType);

			ASSERT_OR_DIE(destinationTile != nullptr, "Attempted to place destination of exit in impossible place.");
			newExit->m_exitData.m_destinationTile = destinationTile;

			Tile* tileToSpawnOn;
			if (currentAdventureMap.m_exitsToSpawn[exitIndex].m_tileTypeToSpawnOn.empty())
				tileToSpawnOn = m_maps[mapIndex]->GetRandomTraversableTile();
			else
				tileToSpawnOn = m_maps[mapIndex]->GetRandomTileOfType(currentAdventureMap.m_exitsToSpawn[exitIndex].m_tileTypeToSpawnOn);
			ASSERT_OR_DIE(tileToSpawnOn != nullptr, "Attempted to spawn exit in impossible place.");

			m_maps[mapIndex]->PlaceFeatureInMap(newExit, tileToSpawnOn);

			//Place reciprocal exit
			if (!currentAdventureMap.m_exitsToSpawn[exitIndex].m_featureTypeToPlace.empty())
			{
				Feature* newFeature = new Feature(currentAdventureMap.m_exitsToSpawn[exitIndex].m_featureTypeToPlace);
				destinationMap->PlaceFeatureInMap(newFeature, destinationTile);

				if (newExit->m_exitData.m_isTwoWay)
				{
					newFeature->m_exitData.m_destinationTile = tileToSpawnOn;
				}
			}
		}
	}

	//Sets starting map
	for (size_t mapIndex = 0; mapIndex < m_maps.size(); mapIndex++)
	{
		if (m_maps[mapIndex]->m_name == m_currentAdventure->m_startingConditions.m_startingMap)
		{
			m_currentMap = m_maps[mapIndex];
			break;
		}
	}
	
	ASSERT_OR_DIE(m_currentMap, "No map in adventure matches starting map name.");

	//Places player and actors in current map
	Tile* startingTile;
	if (m_currentAdventure->m_startingConditions.m_tileTypeToStartOn.empty())
		startingTile = m_currentMap->GetRandomTraversableTile();
	else
		startingTile = m_currentMap->GetRandomTileOfType(m_currentAdventure->m_startingConditions.m_tileTypeToStartOn);

	m_thePlayer = CharacterBuilder::BuildNewCharacter("player");
	m_currentMap->PlaceCharacterInMap(m_thePlayer, startingTile);
	m_currentMap->m_damageNumbers.push_back(DamageNumber(m_currentAdventure->m_startingText, Vector2(ORTHO_X_DIMENSION * 0.5f, ORTHO_Y_DIMENSION * 0.5f), Rgba::WHITE, 3.f));
}

void World::PlaceCorridor(Map*& mapToPlaceCorridorIn, const IntVector2& startCoords, const IntVector2& endCoords, std::string corridorTile, std::string roomFloorTile)
{
	IntVector2 distanceDebts = endCoords - startCoords;
	IntVector2 currentCoords = startCoords;

	if (mapToPlaceCorridorIn->GetTileAtTileCoords(currentCoords)->m_tileDefinition->m_name != roomFloorTile)
		mapToPlaceCorridorIn->GetTileAtTileCoords(currentCoords)->ChangeType(corridorTile);

	while (distanceDebts.x != 0 || distanceDebts.y != 0)
	{
		std::vector<IntVector2> possibleDirections;

		if (distanceDebts.x > 0)
			possibleDirections.push_back(IntVector2(1, 0));
		else if(distanceDebts.x < 0)
			possibleDirections.push_back(IntVector2(-1, 0));

		if (distanceDebts.y > 0)
			possibleDirections.push_back(IntVector2(0, 1));
		else if (distanceDebts.y < 0)
			possibleDirections.push_back(IntVector2(0, -1));

		IntVector2 nextDirection = possibleDirections[GetRandomIntLessThan(possibleDirections.size())];
		distanceDebts = distanceDebts - nextDirection;
		currentCoords = currentCoords + nextDirection;

		if (mapToPlaceCorridorIn->GetTileAtTileCoords(currentCoords)->m_tileDefinition->m_name != roomFloorTile)
			mapToPlaceCorridorIn->GetTileAtTileCoords(currentCoords)->ChangeType(corridorTile);
	}
}

void World::UpdateFogOfWar()
{
	for (size_t tileIndex = 0; tileIndex < m_currentMap->m_tiles.size(); tileIndex++)
	{
		m_currentMap->m_tiles[tileIndex].m_isVisibleToPlayer = false;
	}

	for (Tile* tile : m_thePlayer->GetVisibleTiles())
	{
		tile->m_isVisibleToPlayer = true;
		tile->m_hasBeenSeenByPlayer = true;
	}
}

void World::UpdateVisibilities()
{
	for (Character* character : m_currentMap->FindAllCharacters())
	{
		character->UpdateVisibleActors();
	}
}

void World::CheckForVictory()
{
	bool isPlayerVictorious = true;
	for (std::string victoryItemType : m_currentAdventure->m_victoryItems)
	{
		if (!m_thePlayer->m_entityInventory.ContainsItemOfType(victoryItemType))
			isPlayerVictorious = false;
	}

	m_hasPlayerWon = isPlayerVictorious;
}

void World::FinishGeneratingMap()
{
	bool isCompleted = false;
	while (!isCompleted)
	{
		isCompleted = StepGeneration();
	}
}
