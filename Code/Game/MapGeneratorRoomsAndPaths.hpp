#pragma once
#include "ThirdParty\XMLParser\XMLParser.hpp"
#include <string>
#include "Game/MapGenerator.hpp"
#include <vector>


class MapGeneratorRoomsAndPaths : public MapGenerator
{
public:
	MapGeneratorRoomsAndPaths(XMLNode element);

	virtual void GenerateMap(Map*& outMapToGenerate) override;

	int m_numRooms;
	IntVector2 m_minRoomDimensions;
	IntVector2 m_maxRoomDimensions;
	float m_chanceForPathToContinueStraight;
	std::string m_roomFloorTile;
	std::string m_roomWallTile;
	std::string m_pathTile;

	int m_possibleOverlaps;
	float m_pathStraightness;

	float m_roomFloorPermanence;
	float m_roomWallPermanence;
	float m_pathPermanence;

	std::vector<IntVector2> m_roomMins;
	std::vector<IntVector2> m_roomMaxs;
	std::vector<IntVector2> m_roomCenters;

private:
	bool AttemptToPlaceRoom(Map*& mapToPlaceRoomIn, IntVector2& outRoomCenter, IntVector2& outRoomMins, IntVector2& outRoomMaxs);
	void PlaceCorridor(Map*& mapToPlaceCorridorIn, const IntVector2& startCoords, const IntVector2& endCoords);
	void PlaceRoom(Map*& mapToPlaceRoomIn, const IntVector2& roomMins, const IntVector2& roomMaxs);
	bool DoRoomsOverlap(const IntVector2& roomAMins, const IntVector2& roomAMaxs, const IntVector2& roomBMins, const IntVector2& roomBMaxs);
};