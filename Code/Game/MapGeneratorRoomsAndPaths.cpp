#include "Game/MapGeneratorRoomsAndPaths.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"

MapGeneratorRoomsAndPaths::MapGeneratorRoomsAndPaths(XMLNode element)
	: MapGenerator(element)
	, m_roomMaxs()
	, m_roomMins()
	, m_roomCenters()
{
	m_numRooms = ParseXMLAttributeInt(element, "numRooms", 0);
	m_minRoomDimensions = ParseXMLAttributeIntVector2(element, "minRoomDimensions", IntVector2(-1, -1));
	m_maxRoomDimensions = ParseXMLAttributeIntVector2(element, "maxRoomDimensions", IntVector2(-1, -1));
	ASSERT_OR_DIE(m_minRoomDimensions.x != -1 || m_maxRoomDimensions.x != -1, "Missing or invalid room dimensions.");

	m_chanceForPathToContinueStraight = ParseXMLAttributeFloat(element, "pathStraightness", 0.f);

	m_roomFloorTile = ParseXMLAttributeString(element, "roomFloorTile", "INVALID_TILE");
	ASSERT_OR_DIE(m_roomFloorTile != "INVALID_TILE", "Missing or invalid tile name for room floor tile.");

	m_roomWallTile = ParseXMLAttributeString(element, "roomWallTile", "INVALID_TILE");
	ASSERT_OR_DIE(m_roomWallTile != "INVALID_TILE", "Missing or invalid tile name for room wall tile.");

	m_pathTile = ParseXMLAttributeString(element, "pathTile", "INVALID_TILE");
	ASSERT_OR_DIE(m_pathTile != "INVALID_TILE", "Missing or invalid tile name for path tile.");

	m_roomFloorPermanence = ParseXMLAttributeFloat(element, "roomFloorPermanence", 0.5f);
	m_roomWallPermanence = ParseXMLAttributeFloat(element, "roomWallPermanence", 0.2f);
	m_pathPermanence = ParseXMLAttributeFloat(element, "pathPermanence", 0.3f);

	m_pathStraightness = ParseXMLAttributeFloat(element, "pathStraightness", 0.f);
	m_possibleOverlaps = ParseXMLAttributeInt(element, "possibleOverlaps", 0);
}

void MapGeneratorRoomsAndPaths::GenerateMap(Map*& outMapToGenerate)
{
	//Find room positions
	int maxAttempts = 10;
	int maxRoomAttempts = 100;
	for (int attemptIndex = 0; attemptIndex < maxAttempts; attemptIndex++)
	{
		for (int roomIndex = 0; roomIndex < m_numRooms; roomIndex++)
		{
			for (int roomAttemptIndex = 0; roomAttemptIndex < maxRoomAttempts; roomAttemptIndex++)
			{
				IntVector2 newRoomCenter;
				IntVector2 newRoomMins;
				IntVector2 newRoomMaxs;
				bool roomPlacedSuccessfully = AttemptToPlaceRoom(outMapToGenerate, newRoomCenter, newRoomMins, newRoomMaxs);
				if (roomPlacedSuccessfully)
				{
					m_roomCenters.push_back(newRoomCenter);
					m_roomMins.push_back(newRoomMins);
					m_roomMaxs.push_back(newRoomMaxs);
					break;
				}
			}
		}
		if (m_roomCenters.size() == (size_t)m_numRooms)
			break;

		if (attemptIndex == (maxAttempts - 1))
			break;

		m_roomCenters.clear();
		m_roomMins.clear();
		m_roomMaxs.clear();
	}

	//Place room tiles
	for (size_t roomIndex = 0; roomIndex < m_roomMins.size(); roomIndex++)
	{
		PlaceRoom(outMapToGenerate, m_roomMins[roomIndex], m_roomMaxs[roomIndex]);
	}

	//Corridors
	for (size_t roomCenterIndex = 1; roomCenterIndex < m_roomCenters.size(); roomCenterIndex++)
	{
		PlaceCorridor(outMapToGenerate, m_roomCenters[roomCenterIndex - 1], m_roomCenters[roomCenterIndex]);
	}
}


void MapGeneratorRoomsAndPaths::PlaceCorridor(Map*& mapToPlaceCorridorIn, const IntVector2& startCoords, const IntVector2& endCoords)
{
	IntVector2 distanceDebts = endCoords - startCoords;
	IntVector2 currentCoords = startCoords;

	PlaceTileIfPossible(mapToPlaceCorridorIn->GetTileAtTileCoords(currentCoords), m_pathTile, m_pathPermanence);

	IntVector2 previousDirection(0, 0);
	while (distanceDebts.x != 0 || distanceDebts.y != 0)
	{
		IntVector2 nextDirection;
		IntVector2 absoluteDistanceDebts(abs(distanceDebts.x), abs(distanceDebts.y));
		IntVector2 absolutePreviousDirection(abs(previousDirection.x), abs((previousDirection.y)));
		if	((previousDirection != IntVector2(0, 0))
			&& ((absoluteDistanceDebts - absolutePreviousDirection).x >= 0)
			&& ((absoluteDistanceDebts - absolutePreviousDirection).y >= 0)
			&& (GetRandomFloatZeroToOne() <= m_pathStraightness))
		{
			nextDirection = previousDirection;
		}
		else
		{
			std::vector<IntVector2> possibleDirections;

			if (distanceDebts.x > 0)
				possibleDirections.push_back(IntVector2(1, 0));
			else if (distanceDebts.x < 0)
				possibleDirections.push_back(IntVector2(-1, 0));

			if (distanceDebts.y > 0)
				possibleDirections.push_back(IntVector2(0, 1));
			else if (distanceDebts.y < 0)
				possibleDirections.push_back(IntVector2(0, -1));

			nextDirection = possibleDirections[GetRandomIntLessThan(possibleDirections.size())];
		}
		
		distanceDebts = distanceDebts - nextDirection;
		currentCoords = currentCoords + nextDirection;
		previousDirection = nextDirection;

		PlaceTileIfPossible(mapToPlaceCorridorIn->GetTileAtTileCoords(currentCoords), m_pathTile, m_pathPermanence);
	}
}

void MapGeneratorRoomsAndPaths::PlaceRoom(Map*& mapToPlaceRoomIn, const IntVector2& roomMins, const IntVector2& roomMaxs)
{
	IntVector2 startingPoint = roomMins;
	IntVector2 newRoomDimensions = roomMaxs - roomMins;

	//Place room floors
	for (int yIndex = 0; yIndex < newRoomDimensions.y; yIndex++)
	{
		for (int xIndex = 0; xIndex < newRoomDimensions.x; xIndex++)
		{
			IntVector2 currentTileCoords(startingPoint + IntVector2(xIndex, yIndex));
			PlaceTileIfPossible(mapToPlaceRoomIn->GetTileAtTileCoords(currentTileCoords), m_roomFloorTile, m_roomFloorPermanence);
		}
	}

	//Place room walls
	for (int xIndex = -1; xIndex <= newRoomDimensions.x; xIndex++)
	{
		IntVector2 topTileCoords(startingPoint + IntVector2(xIndex, newRoomDimensions.y));
		IntVector2 bottomTileCoords(startingPoint + IntVector2(xIndex, -1));

		PlaceTileIfPossible(mapToPlaceRoomIn->GetTileAtTileCoords(topTileCoords), m_roomWallTile, m_roomWallPermanence);
		PlaceTileIfPossible(mapToPlaceRoomIn->GetTileAtTileCoords(bottomTileCoords), m_roomWallTile, m_roomWallPermanence);
	}

	for (int yIndex = -1; yIndex <= newRoomDimensions.y; yIndex++)
	{
		IntVector2 leftTileCoords(startingPoint + IntVector2(-1, yIndex));
		IntVector2 rightTileCoords(startingPoint + IntVector2(newRoomDimensions.x, yIndex));

		PlaceTileIfPossible(mapToPlaceRoomIn->GetTileAtTileCoords(leftTileCoords), m_roomWallTile, m_roomWallPermanence);
		PlaceTileIfPossible(mapToPlaceRoomIn->GetTileAtTileCoords(rightTileCoords), m_roomWallTile, m_roomWallPermanence);
	}
}

bool MapGeneratorRoomsAndPaths::DoRoomsOverlap(const IntVector2& roomAMins, const IntVector2& roomAMaxs, const IntVector2& roomBMins, const IntVector2& roomBMaxs)
{
	if (roomAMins.x > roomBMins.x && roomAMins.x < roomBMaxs.x)
		return true;

	if (roomAMins.y > roomBMins.y && roomAMins.y < roomBMaxs.y)
		return true;

	if (roomAMaxs.x > roomBMins.x && roomAMaxs.x < roomBMaxs.x)
		return true;

	if (roomAMaxs.y > roomBMins.y && roomAMaxs.y < roomBMaxs.y)
		return true;

	return false;
}

bool MapGeneratorRoomsAndPaths::AttemptToPlaceRoom(Map*& mapToPlaceRoomIn, IntVector2& outRoomCenter, IntVector2& outRoomMins, IntVector2& outRoomMaxs)
{
	IntVector2 newRoomDimensions(GetRandomIntInRange(m_minRoomDimensions.x, m_maxRoomDimensions.x), GetRandomIntInRange(m_minRoomDimensions.y, m_maxRoomDimensions.y));

	IntVector2 minimumStartingPoint(2, 2);
	IntVector2 maximumStartingPoint((mapToPlaceRoomIn->m_definition->m_dimensions - newRoomDimensions) - IntVector2(2, 2));

	IntVector2 startingPoint(GetRandomIntInRange(minimumStartingPoint.x, maximumStartingPoint.x), GetRandomIntInRange(minimumStartingPoint.y, maximumStartingPoint.y));

	IntVector2 roomRelativeCenter = ((startingPoint + newRoomDimensions) - startingPoint);
	roomRelativeCenter = IntVector2((int)floor(roomRelativeCenter.x / 2), (int)floor(roomRelativeCenter.y / 2));
	IntVector2 newRoomCenter = startingPoint + roomRelativeCenter;
	IntVector2 newRoomMins = startingPoint;
	IntVector2 newRoomMaxs = startingPoint + newRoomDimensions;


	bool hasOverlapped = false;

	for (size_t roomIndex = 0; roomIndex < m_roomMins.size(); roomIndex++)
	{
		if (DoRoomsOverlap(newRoomMins, newRoomMaxs, m_roomMins[roomIndex], m_roomMaxs[roomIndex]))
		{
			if (m_possibleOverlaps > 0)
				hasOverlapped = true;
			else
				return false;
		}
	}

	//Determine if chosen room is allowed
// 	for (int yIndex = -1; yIndex <= newRoomDimensions.y; yIndex++)
// 	{
// 		for (int xIndex = -1; xIndex <= newRoomDimensions.x; xIndex++)
// 		{
// 			IntVector2 currentTileCoords(startingPoint + IntVector2(xIndex, yIndex));
// 			if (mapToPlaceRoomIn->GetTileAtTileCoords(currentTileCoords)->m_tileDefinition->m_name == m_roomFloorTile || mapToPlaceRoomIn->GetTileAtTileCoords(currentTileCoords)->m_tileDefinition->m_name == m_roomWallTile)
// 			{
// 				if (m_possibleOverlaps > 0)
// 					hasOverlapped = true;
// 				else
// 					return false;
// 			}
// 		}
// 	}


	if (hasOverlapped)
		m_possibleOverlaps--;

	outRoomCenter = newRoomCenter;
	outRoomMaxs = newRoomMaxs;
	outRoomMins = newRoomMins;

	return true;
}
