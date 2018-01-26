#include "Game/Map.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/CharacterBuilder.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineConfig.hpp"
#include "Game/App.hpp"


PathGenerator::PathGenerator(const IntVector2& start, const IntVector2& end, Map* map, Character* gCostReferenceCharacter)
	: m_start(start)
	, m_end(end)
	, m_map(map)
	, m_gCostReferenceCharacter(gCostReferenceCharacter)
	, m_openList()
{
	static int pathID = 0;
	pathID++;
	m_pathID = pathID;
	OpenNodeForProcessing(*m_map->GetTileAtTileCoords(m_start), nullptr);
}


Path PathGenerator::CreateFinalPath(OpenNode& endNode)
{
	OpenNode* currentNode = &endNode;

	Path outPath;
	while (currentNode->m_parent != nullptr)
	{
		outPath.push_back(currentNode->m_tile);
		currentNode = currentNode->m_parent;
	}

	m_finalPath = outPath;
	return outPath;
}

void PathGenerator::OpenNodeForProcessing(Tile& tileToOpen, OpenNode* parent)
{
	OpenNode* newOpenNode = new OpenNode;
	newOpenNode->m_tile = &tileToOpen;
	newOpenNode->m_parent = parent;
	newOpenNode->m_localGCost = newOpenNode->m_tile->GetGCost() + m_gCostReferenceCharacter->GetGCostBias(newOpenNode->m_tile->m_tileDefinition->m_name);
	newOpenNode->m_totalGCost = ((parent) ? parent->m_totalGCost : 0.f) + newOpenNode->m_localGCost;
	newOpenNode->m_estimatedDistToGoal = (float)m_map->CalculateManhattanDistance(*newOpenNode->m_tile, *m_map->GetTileAtTileCoords(m_end));
	newOpenNode->m_fScore = newOpenNode->m_estimatedDistToGoal + newOpenNode->m_totalGCost;

	m_openList.push_back(newOpenNode);
	tileToOpen.m_isOpenInPathID = m_pathID;
}

OpenNode* PathGenerator::SelectAndCloseBestOpenNode()
{
	int bestNodeIndex = -1;
	float lowestFScore = FLT_MAX;

	for (size_t nodeIndex = 0; nodeIndex < m_openList.size(); nodeIndex++)
	{
		OpenNode* node = m_openList[nodeIndex];
		if (node->m_fScore < lowestFScore)
		{
			lowestFScore = node->m_fScore;
			bestNodeIndex = nodeIndex;
		}
	}

	if (bestNodeIndex == -1)
		return nullptr;

	OpenNode* bestNode = m_openList[bestNodeIndex];
	bestNode->m_tile->m_isClosedInPathID = m_pathID;
	m_openList.erase(m_openList.begin() + bestNodeIndex);
	return bestNode;
}

void PathGenerator::OpenNodeIfValid(Tile* tileToOpen, OpenNode* parent)
{
	if (!tileToOpen)
		return;

	if (tileToOpen->IsSolidToTags(m_gCostReferenceCharacter->m_tags))
		return;

	if (tileToOpen->m_isClosedInPathID == m_pathID)
		return;

	if (tileToOpen->m_isOpenInPathID == m_pathID)
		return;

	OpenNodeForProcessing(*tileToOpen, parent);
}


const float Map::DAMAGE_NUMBER_LIFETIME = 1.f;

DamageNumber::DamageNumber(std::string number, const Vector2& position, const Rgba& color, float scale)
	: m_color(color)
	, m_number(number)
	, m_position(position)
	, m_lifetime(Map::DAMAGE_NUMBER_LIFETIME)
	, m_scale(scale)
{
		
}


Map::Map(std::string mapDefinitionName)
	: m_tiles()
	, m_entities()
	, m_name()
{
	m_definition = MapDefinition::GetDefinition(mapDefinitionName);

	m_tiles.resize(m_definition->m_dimensions.x * m_definition->m_dimensions.y);
	for (size_t tileIndex = 0; tileIndex < m_tiles.size(); tileIndex++)
	{
		m_tiles[tileIndex].m_tileCoords = CalculateTileCoordsFromTileIndex(tileIndex);
		m_tiles[tileIndex].m_containingMap = this;
		m_tiles[tileIndex].ChangeType(m_definition->m_fillTileType);
	}
}

Map::~Map()
{

}


void Map::Update(float deltaSeconds)
{
	for (size_t tileIndex = 0; tileIndex < m_tiles.size(); tileIndex++)
	{
		m_tiles[tileIndex].Update(deltaSeconds);
	}

	for (size_t entityIndex = 0; entityIndex < m_entities.size(); entityIndex++)
	{
		m_entities[entityIndex]->Update(deltaSeconds);
	}

	UpdateDamageNumbers(deltaSeconds);
}

void Map::Render() const
{
	for (size_t tileIndex = 0; tileIndex < m_tiles.size(); tileIndex++)
	{
		m_tiles[tileIndex].Render();
	}

	for (size_t entityIndex = 0; entityIndex < m_entities.size(); entityIndex++)
	{
		m_entities[entityIndex]->Render();
	}

	RenderDamageNumbers();
}

void Map::RenderDebugPathing() const
{
	if (!m_currentPath)
		return;

	for (Tile tile : m_tiles)
	{
		if (tile.m_isClosedInPathID == m_currentPath->m_pathID)
		{
			g_theRenderer->DrawCenteredText2D((Vector2)tile.m_tileCoords + Vector2(0.5f, 0.5f), g_theRenderer->m_defaultFont, "x", Rgba::RED, 0.5f);
		}
		else if (tile.m_isOpenInPathID == m_currentPath->m_pathID)
		{
			g_theRenderer->DrawCenteredText2D((Vector2)tile.m_tileCoords + Vector2(0.5f, 0.5f), g_theRenderer->m_defaultFont, "o", Rgba::GREEN, 0.5f);
		}
		else if (tile.m_tileDefinition->m_isSolid)
		{
			g_theRenderer->DrawCenteredText2D((Vector2)tile.m_tileCoords + Vector2(0.5f, 0.5f), g_theRenderer->m_defaultFont, "s", Rgba::RED, 0.5f);
		}
	}

	for (OpenNode* node : m_currentPath->m_openList)
	{
		g_theRenderer->DrawCenteredText2D((Vector2)node->m_tile->m_tileCoords + Vector2(0.25f, 0.75f), g_theRenderer->m_defaultFont, std::to_string((int)node->m_totalGCost), Rgba::GREEN, 0.5f);
		g_theRenderer->DrawCenteredText2D((Vector2)node->m_tile->m_tileCoords + Vector2(0.75f, 0.75f), g_theRenderer->m_defaultFont, std::to_string((int)node->m_localGCost), Rgba::GREEN, 0.5f);
		g_theRenderer->DrawCenteredText2D((Vector2)node->m_tile->m_tileCoords + Vector2(0.75f, 0.25f), g_theRenderer->m_defaultFont, std::to_string((int)node->m_estimatedDistToGoal), Rgba::GREEN, 0.5f);
		g_theRenderer->DrawCenteredText2D((Vector2)node->m_tile->m_tileCoords + Vector2(0.25f, 0.25f), g_theRenderer->m_defaultFont, std::to_string((int)node->m_fScore), Rgba::GREEN, 0.5f);
	}

	if (!m_currentPath->m_finalPath.empty())
	{
		for (Tile* tile : m_currentPath->m_finalPath)
		{
			g_theRenderer->DrawCenteredText2D((Vector2)tile->m_tileCoords + Vector2(0.5f, 0.5f), g_theRenderer->m_defaultFont, "x", Rgba::BLUE, 0.5f);
		}

	}
}

void Map::RenderDebugGenerating() const
{
	m_definition->DebugRender(this);
}

void Map::AdvanceTurns()
{
	for (size_t entityIndex = 0; entityIndex < m_entities.size(); entityIndex++)
	{
		m_entities[entityIndex]->AdvanceTurn();
	}
}

int Map::CalculateTileIndexFromTileCoords(const IntVector2& tileCoords) const
{
	return tileCoords.y * m_definition->m_dimensions.x + tileCoords.x;
}

IntVector2 Map::CalculateTileCoordsFromTileIndex(int tileIndex) const
{
	return IntVector2(tileIndex % m_definition->m_dimensions.x, tileIndex / m_definition->m_dimensions.x);
}

IntVector2 Map::CalculateTileCoordsFromMapCoords(const Vector2& mapCoords) const
{
	return IntVector2((int)floor(mapCoords.x), (int)floor(mapCoords.y));
}

Tile* Map::GetTileAtMapCoords(const Vector2& mapCoords)
{
	IntVector2 tileCoords = CalculateTileCoordsFromMapCoords(mapCoords);
	return GetTileAtTileCoords(tileCoords);
}

Tile* Map::GetTileAtTileCoords(const IntVector2& tileCoords)
{
	if (IsInMap(tileCoords))
	{
		int tileIndex = CalculateTileIndexFromTileCoords(tileCoords);
		return GetTileAtTileIndex(tileIndex);
	}
	else
		return nullptr;
}

Tile* Map::GetTileAtTileIndex(int tileIndex)
{
	if (tileIndex < 0 || tileIndex >= (int)m_tiles.size())
		return nullptr;
	else
		return &m_tiles[tileIndex];
}

Tile* Map::GetRandomTile()
{
	IntVector2 randomTileCoords(GetRandomIntLessThan(m_definition->m_dimensions.x), GetRandomIntLessThan(m_definition->m_dimensions.y));
	return GetTileAtTileCoords(randomTileCoords);
}

Tile* Map::GetRandomTraversableTile()
{
	Tile* randomTile = GetRandomTile();
	int counter = 0;
	int maxAttempts = 1000;
	while (randomTile->m_tileDefinition->m_isSolid || randomTile->m_occupyingCharacter != nullptr || (randomTile->m_occupyingFeature != nullptr && randomTile->m_occupyingFeature->m_isSolid))
	{
		if (counter >= maxAttempts)
			return nullptr;

		randomTile = GetRandomTile();
		counter++;
	}
	return randomTile;
}

Tile* Map::GetRandomTileOfType(std::string tileType)
{
	Tile* randomTile = GetRandomTile();
	int counter = 0;
	int maxAttempts = 1000;
	while (randomTile->m_tileDefinition->m_name != tileType || randomTile->m_occupyingCharacter != nullptr || (randomTile->m_occupyingFeature != nullptr && randomTile->m_occupyingFeature->m_isSolid))
	{
		if (counter >= maxAttempts)
			return nullptr;

		randomTile = GetRandomTile();
		counter++;
	}
	return randomTile;
}

Tile* Map::GetRandomTileWithTags(std::string tags)
{
	std::vector<Tile*> tilesWithTags;
	for (size_t tileIndex = 0; tileIndex < m_tiles.size(); tileIndex++)
	{
		if (m_tiles[tileIndex].m_tags.MatchTags(tags))
			tilesWithTags.push_back(&m_tiles[tileIndex]);
	}

	int randomTileIndex = GetRandomIntLessThan(tilesWithTags.size());
	return tilesWithTags[randomTileIndex];
}

std::vector<Message> Map::GetTooltipInfoForMapCoords(const Vector2& mapCoords)
{
	std::vector<Message> tooltipInfo;
	if (mapCoords.x > m_definition->m_dimensions.x || mapCoords.y > m_definition->m_dimensions.y)
		return tooltipInfo;

	Tile* tileAtMapCoords = GetTileAtMapCoords(mapCoords);
	if (!tileAtMapCoords)
		return tooltipInfo;

	tooltipInfo = tileAtMapCoords->GetTooltipInfo();
	return tooltipInfo;
}

bool Map::TryToMoveCharacterToTile(Character* characterToMove, Tile* destinationTile)
{
	if (!destinationTile)
		return false;

	if (destinationTile->m_occupyingCharacter)
	{
		characterToMove->Attack(destinationTile->m_occupyingCharacter);
		return false;
	}

	if (destinationTile->IsSolidToTags(characterToMove->m_tags))
		return false;

	if (destinationTile->m_occupyingFeature)
	{
		if (destinationTile->m_occupyingFeature->m_isSolid)
			return false;
		else if(destinationTile->m_occupyingFeature->m_isExit && characterToMove == g_theApp->m_game->m_theWorld->m_thePlayer)
		{
			if (destinationTile->m_occupyingFeature->m_exitData.m_destinationTile->m_containingMap != this)
			{
				int characterToMoveIndex = -1;
				for (size_t entityIndex = 0; entityIndex < m_entities.size(); entityIndex++)
				{
					if (m_entities[entityIndex] == characterToMove)
					{
						characterToMoveIndex = (int)entityIndex;
						break;
					}
				}
				if(characterToMoveIndex >= 0)
					m_entities.erase(m_entities.begin() + characterToMoveIndex);

				g_theApp->m_game->m_theWorld->m_currentMap = destinationTile->m_occupyingFeature->m_exitData.m_destinationTile->m_containingMap;
				characterToMove->m_currentTile->m_occupyingCharacter = nullptr;
				destinationTile->m_occupyingFeature->m_exitData.m_destinationTile->m_containingMap->PlaceCharacterInMap(characterToMove, destinationTile->m_occupyingFeature->m_exitData.m_destinationTile);
			}
			else
			{
				MoveCharacterToTile(characterToMove, destinationTile);
			}
		}
		else
			MoveCharacterToTile(characterToMove, destinationTile);

		return true;
	}

	MoveCharacterToTile(characterToMove, destinationTile);
	characterToMove->PickupItemsInCurrentTile();
	return true;
}

void Map::SpawnActors()
{
	Character* pixie = CharacterBuilder::BuildNewCharacter("pixie");
	PlaceCharacterInMap(pixie, GetRandomTraversableTile());

	Character* pixie2 = CharacterBuilder::BuildNewCharacter("pixie");
	PlaceCharacterInMap(pixie2, GetRandomTraversableTile());

	Character* pixie3 = CharacterBuilder::BuildNewCharacter("pixie");
	PlaceCharacterInMap(pixie3, GetRandomTraversableTile());
}

int Map::CalculateManhattanDistance(const Tile& tileA, const Tile& tileB)
{
	IntVector2 distanceVector = tileB.m_tileCoords - tileA.m_tileCoords;
	return (abs(distanceVector.x) + abs(distanceVector.y));
}

Entity* Map::FindNearestEntityOfType(const IntVector2& startingPosition, std::string type)
{
	Tile* startingTile = GetTileAtTileCoords(startingPosition);
	Entity* nearestEntity = nullptr;
	int distanceToNearestEntity = INT_MAX;
	for (Entity* entity : m_entities)
	{
		if (entity->m_name == type && entity->m_currentTile)
		{
			int distanceToEntity = CalculateManhattanDistance(*startingTile, *entity->m_currentTile);
			if (distanceToEntity < distanceToNearestEntity)
			{
				distanceToNearestEntity = distanceToEntity;
				nearestEntity = entity;
			}
		}
	}

	return nearestEntity;
}

Entity* Map::FindNearestEntityNotOfType(const IntVector2& startingPosition, std::string type)
{
	Tile* startingTile = GetTileAtTileCoords(startingPosition);
	Entity* nearestEntity = nullptr;
	int distanceToNearestEntity = INT_MAX;
	for (Entity* entity : m_entities)
	{
		if (entity->m_name != type && entity->m_currentTile)
		{
			int distanceToEntity = CalculateManhattanDistance(*startingTile, *entity->m_currentTile);
			if (distanceToEntity < distanceToNearestEntity)
			{
				distanceToNearestEntity = distanceToEntity;
				nearestEntity = entity;
			}
		}
	}

	return nearestEntity;
}

Character* Map::FindNearestCharacterOfFaction(const IntVector2& startingPosition, std::string faction)
{
	Tile* startingTile = GetTileAtTileCoords(startingPosition);
	Character* nearestCharacter = nullptr;
	int distanceToNearestCharacter = INT_MAX;
	for (Tile& tile : m_tiles)
	{
		if (tile.m_occupyingCharacter && tile.m_occupyingCharacter->m_faction == faction)
		{
			int distanceToCharacter = CalculateManhattanDistance(*startingTile, tile);
			if (distanceToCharacter < distanceToNearestCharacter)
			{
				distanceToNearestCharacter = distanceToCharacter;
				nearestCharacter = tile.m_occupyingCharacter;
			}
		}
	}

	return nearestCharacter;
}

Character* Map::FindNearestCharacterNotOfFaction(const IntVector2& startingPosition, std::string faction)
{
	Tile* startingTile = GetTileAtTileCoords(startingPosition);
	Character* nearestCharacter = nullptr;
	int distanceToNearestCharacter = INT_MAX;
	for (Tile& tile : m_tiles)
	{
		if (tile.m_occupyingCharacter && tile.m_occupyingCharacter->m_faction != faction)
		{
			int distanceToCharacter = CalculateManhattanDistance(*startingTile, tile);
			if (distanceToCharacter < distanceToNearestCharacter)
			{
				distanceToNearestCharacter = distanceToCharacter;
				nearestCharacter = tile.m_occupyingCharacter;
			}
		}
	}

	return nearestCharacter;
}

std::vector<Character*> Map::FindAllCharactersOfFaction(std::string faction)
{
	std::vector<Character*> outVector;

	for (Tile& tile : m_tiles)
	{
		if (tile.m_occupyingCharacter && tile.m_occupyingCharacter->m_faction == faction)
		{
			outVector.push_back(tile.m_occupyingCharacter);
		}
	}

	return outVector;
}

std::vector<Character*> Map::FindAllCharactersNotOfFaction(std::string faction)
{
	std::vector<Character*> outVector;

	for (Tile& tile : m_tiles)
	{
		if (tile.m_occupyingCharacter && tile.m_occupyingCharacter->m_faction != faction)
		{
			outVector.push_back(tile.m_occupyingCharacter);
		}
	}

	return outVector;
}

std::vector<Character *> Map::FindAllCharacters()
{
	std::vector<Character*> outVector;

	for (Tile& tile : m_tiles)
	{
		if (tile.m_occupyingCharacter)
		{
			outVector.push_back(tile.m_occupyingCharacter);
		}
	}

	return outVector;
}

Tile* Map::FindNearestTileOfType(const IntVector2& startingPosition, std::string type)
{
	Tile* startingTile = GetTileAtTileCoords(startingPosition);
	Tile* nearestTile = nullptr;
	int distanceToNearestTile = INT_MAX;
	for (Tile& tile : m_tiles)
	{
		if (tile.m_tileDefinition->m_name == type)
		{
			int distanceToTile = CalculateManhattanDistance(*startingTile, tile);
			if (distanceToTile < distanceToNearestTile)
			{
				distanceToNearestTile = distanceToTile;
				nearestTile = &tile;
			}
		}
	}

	return nearestTile;
}

Tile* Map::FindNearestTileNotOfType(const IntVector2& startingPosition, std::string type)
{
	Tile* startingTile = GetTileAtTileCoords(startingPosition);
	Tile* nearestTile = nullptr;
	int distanceToNearestTile = INT_MAX;
	for (Tile& tile : m_tiles)
	{
		if (tile.m_tileDefinition->m_name != type)
		{
			int distanceToTile = CalculateManhattanDistance(*startingTile, tile);
			if (distanceToTile < distanceToNearestTile)
			{
				distanceToNearestTile = distanceToTile;
				nearestTile = &tile;
			}
		}
	}

	return nearestTile;
}

void Map::SpawnFeatures()
{
	Feature* chest = new Feature("chest");
	PlaceFeatureInMap(chest, GetTileAtTileCoords(IntVector2(5, 6)));
}


void Map::DestroyCharacter(Character* characterToKill)
{
	for (Character* character : FindAllCharacters())
	{
		if (character->m_visibleCharacters.count(characterToKill) != 0)
		{
			character->m_visibleCharacters.erase(characterToKill);
		}

		if (character->m_target == characterToKill)
			character->m_target = nullptr;
	}

	Tile* tileContainingCharacterToKill = characterToKill->m_currentTile;
	tileContainingCharacterToKill->m_occupyingCharacter = nullptr;

	DestroyEntity(characterToKill);
}

void Map::DestroyFeature(Feature* featureToDestroy)
{
	Tile* tileContainingFeatureToDestroy = featureToDestroy->m_currentTile;
	tileContainingFeatureToDestroy->m_occupyingFeature = nullptr;

	DestroyEntity(featureToDestroy);
}

void Map::PlaceItemInMap(Item* itemToPlace, Tile* tileToPlaceIn)
{
	if (!tileToPlaceIn)
		return;

	tileToPlaceIn->m_tileInventory.AddItem(itemToPlace);
}

Tile* Map::FindFirstTraversableTile()
{
	for (size_t tileIndex = 0; tileIndex < m_tiles.size(); tileIndex++)
	{
		if (!GetTileAtTileIndex(tileIndex)->m_tileDefinition->m_isSolid)
			return GetTileAtTileIndex(tileIndex);
	}

	return nullptr;
}

void Map::PlaceCharacterInMap(Character* characterToPlace, Tile* destinationTile)
{
	if (!destinationTile)
		return;

	destinationTile->m_occupyingCharacter = characterToPlace;

	PlaceEntityInMap(characterToPlace, destinationTile);
}

void Map::PlaceFeatureInMap(Feature* featureToPlace, Tile* destinationTile)
{
	if (!destinationTile)
		return;

	destinationTile->m_occupyingFeature = featureToPlace;

	PlaceEntityInMap(featureToPlace, destinationTile);
}

void Map::DestroyEntity(Entity* entityToDestroy)
{
	entityToDestroy->m_entityInventory.TransferItemsToOtherInventory(entityToDestroy->m_currentTile->m_tileInventory);

	size_t entityIndex = 0;
	for (; entityIndex < m_entities.size(); entityIndex++)
	{
		if (m_entities[entityIndex] == entityToDestroy)
			break;
	}
	m_entities.erase(m_entities.begin() + entityIndex);

	delete entityToDestroy;
	entityToDestroy = nullptr;
}

void Map::PlaceEntityInMap(Entity* entityToPlace, Tile* destinationTile)
{
	entityToPlace->m_currentMap = this;
	entityToPlace->m_currentTile = destinationTile;
	m_entities.push_back(entityToPlace);
}

void Map::MoveCharacterToTile(Character* characterToMove, Tile* destinationTile)
{
	Tile* startTile = characterToMove->m_currentTile;
	startTile->m_occupyingCharacter = nullptr;
	destinationTile->m_occupyingCharacter = characterToMove;

	characterToMove->m_currentTile = destinationTile;
}

void Map::UpdateDamageNumbers(float deltaSeconds)
{
	size_t numDamageNumbers = m_damageNumbers.size();
	for (size_t damageNumberIndex = 0; damageNumberIndex < numDamageNumbers; damageNumberIndex++)
	{
		DamageNumber& number = m_damageNumbers[damageNumberIndex];
		number.m_lifetime -= deltaSeconds;
		number.m_position.y += deltaSeconds;

		if (number.m_lifetime < 0.f)
		{
			m_damageNumbers.erase(m_damageNumbers.begin() + damageNumberIndex);
			numDamageNumbers--;
			damageNumberIndex--;
		}
	}
}

void Map::RenderDamageNumbers() const
{
	for (DamageNumber number : m_damageNumbers)
	{
		Rgba fadedColor(number.m_color);
		fadedColor.a = 0;
		Rgba numberColor = Interpolate(number.m_color, fadedColor, RangeMapFloat(number.m_lifetime, DAMAGE_NUMBER_LIFETIME, 0.f, 0.f, 1.f));
		g_theRenderer->DrawCenteredText2D(number.m_position, g_theRenderer->m_defaultFont, number.m_number, numberColor, number.m_scale);
	}
}

std::vector<Tile*> Map::GetTilesInRadius(const IntVector2& tileCoords, float radius)
{
	std::vector<Tile*> outVector;

	float maxDistanceSquared = radius * radius;
	Vector2 centerTileCoords = (Vector2)tileCoords + Vector2(0.5f, 0.5f);

	for (int yIndex = tileCoords.y - (int)radius; yIndex <= tileCoords.y + (int)radius; yIndex++)
	{
		for (int xIndex = tileCoords.x - (int)radius; xIndex <= tileCoords.x + (int)radius; xIndex++)
		{
			Vector2 currentTileCenterCoords = Vector2(xIndex + 0.5f, yIndex + 0.5f);
			Tile* currentTile = GetTileAtMapCoords(currentTileCenterCoords);
			if(currentTile)
			{
				if (CalcDistanceSquared(currentTileCenterCoords, centerTileCoords) <= maxDistanceSquared)
				{
					outVector.push_back(currentTile);
				}
			}
		}
	}

	return outVector;
}

bool Map::IsInMap(const IntVector2& tileCoords) const
{
	if (tileCoords.x < 0 || tileCoords.x >= m_definition->m_dimensions.x)
		return false;

	if (tileCoords.y < 0 || tileCoords.y >= m_definition->m_dimensions.y)
		return false;

	return true;
}

RaycastResult Map::RaycastForSolid(const Vector2& startPosition, const Vector2& direction, float maxDistance)
{
	RaycastResult result;
	Vector2 endPosition = startPosition + (direction * maxDistance);
	Vector2 displacement = endPosition - startPosition;
	Vector2 singleStep = displacement * 0.01f;
	for (int stepIndex = 1; stepIndex < 100; stepIndex++)
	{
		Vector2 previousPosition = startPosition + (singleStep * (float)(stepIndex - 1));
		Vector2 currentPosition = startPosition + (singleStep * (float)stepIndex);
		Tile* currentTile = &m_tiles[CalculateTileIndexFromTileCoords(CalculateTileCoordsFromMapCoords(currentPosition))];
		result.m_impactedTiles.insert(currentTile);
		if (currentTile->m_tileDefinition->m_isSolid)
		{
			result.m_didImpact = true;
			result.m_impactedTiles.insert(currentTile);

			Vector2 currentDisplacement = currentPosition - startPosition;
			result.m_impactFraction = currentDisplacement.CalcLength() / maxDistance;

			result.m_pointBeforeImpact = previousPosition;
			result.m_impactPosition = currentPosition;

			const Tile* previousTile = &m_tiles[CalculateTileIndexFromTileCoords(CalculateTileCoordsFromMapCoords(currentPosition))];
			Vector2 tileDisplacement = previousTile->m_tileCoords - currentTile->m_tileCoords;
			result.m_impactNormal = tileDisplacement.GetNormalized();

			return result;
		}
	}
	result.m_didImpact = false;
	result.m_impactedTiles;
	result.m_impactFraction = 1.0f;
	result.m_impactPosition = startPosition + (direction * maxDistance);
	result.m_pointBeforeImpact = result.m_impactPosition;
	return result;
}

RaycastResult Map::RaycastForOpaque(const Vector2& startPosition, const Vector2& direction, float maxDistance)
{
	RaycastResult result;
	Vector2 endPosition = startPosition + (direction * maxDistance);
	Vector2 displacement = endPosition - startPosition;
	Vector2 singleStep = displacement * 0.01f;
	for (int stepIndex = 1; stepIndex < 100; stepIndex++)
	{
		Vector2 previousPosition = startPosition + (singleStep * (float)(stepIndex - 1));
		Vector2 currentPosition = startPosition + (singleStep * (float)stepIndex);
		if(IsInMap(CalculateTileCoordsFromMapCoords(currentPosition)))
		{
			Tile* currentTile = &m_tiles[CalculateTileIndexFromTileCoords(CalculateTileCoordsFromMapCoords(currentPosition))];
			result.m_impactedTiles.insert(currentTile);
			if (currentTile->m_tileDefinition->m_isOpaque)
			{
				result.m_didImpact = true;
				result.m_impactedTiles.insert(currentTile);

				Vector2 currentDisplacement = currentPosition - startPosition;
				result.m_impactFraction = currentDisplacement.CalcLength() / maxDistance;

				result.m_pointBeforeImpact = previousPosition;
				result.m_impactPosition = currentPosition;

				const Tile* previousTile = &m_tiles[CalculateTileIndexFromTileCoords(CalculateTileCoordsFromMapCoords(currentPosition))];
				Vector2 tileDisplacement = previousTile->m_tileCoords - currentTile->m_tileCoords;
				result.m_impactNormal = tileDisplacement.GetNormalized();

				return result;
			}
		}
	}
	result.m_didImpact = false;
	result.m_impactFraction = 1.0f;
	result.m_impactPosition = startPosition + (direction * maxDistance);
	result.m_pointBeforeImpact = result.m_impactPosition;
	return result;
}

Path Map::GeneratePath(const IntVector2& start, const IntVector2& end, Character* characterForPath /*= nullptr*/)
{
	StartSteppedPath(start, end, characterForPath);
	Path outPath;

	bool isCompleted = false;
	while (!isCompleted)
	{
		isCompleted = ContinueSteppedPath(outPath);
	}

	return outPath;
}

void Map::StartSteppedPath(const IntVector2& start, const IntVector2& end, Character* characterForPath /*= nullptr*/)
{
	if (m_currentPath)
		delete m_currentPath;

	m_currentPath = new PathGenerator(start, end, this, characterForPath);
}

bool Map::ContinueSteppedPath(Path& out_pathWhenComplete)
{
	//select and close best open node
	OpenNode* currentNode = m_currentPath->SelectAndCloseBestOpenNode();

	if (!currentNode)
		return true;

	//see if goal
	if (currentNode->m_tile->m_tileCoords == m_currentPath->m_end)
	{
		out_pathWhenComplete = m_currentPath->CreateFinalPath(*currentNode);
		return true;
	}

	m_currentPath->OpenNodeIfValid(currentNode->m_tile->GetNorthNeighbor(), currentNode);
	m_currentPath->OpenNodeIfValid(currentNode->m_tile->GetEastNeighbor(), currentNode);
	m_currentPath->OpenNodeIfValid(currentNode->m_tile->GetSouthNeighbor(), currentNode);
	m_currentPath->OpenNodeIfValid(currentNode->m_tile->GetWestNeighbor(), currentNode);

	return false;
}

