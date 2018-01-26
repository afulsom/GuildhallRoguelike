#pragma once
#include "Game/Map.hpp"
#include <vector>
#include "Adventure.hpp"


class World
{
public:
	World();
	~World();

	void Initialize();
	Map* GenerateMap(std::string mapDefinitionName);
	void StartSteppedGeneration(std::string mapDefinitionName);
	bool StepGeneration();
	void FinishGeneratingMap();

	Vector2 m_cursorPosition;
	Character* m_thePlayer;

	Map* m_currentMap;
	std::vector<Map*> m_maps;
	
	Adventure* m_currentAdventure = nullptr;
	bool m_hasPlayerWon = false;
	bool m_hasPlayerLost = false;

	MapDefinition* m_currentlyGeneratingMapDefinition;
	Map* m_currentlyGeneratingMap;

	void Update(float deltaSeconds);
	void Render() const;
	void DrawCursor() const;
	void DrawUI() const;


	void GenerateAdventure(std::string adventureName);
private:
	void DrawTooltip() const;
	void PlaceCorridor(Map*& mapToPlaceCorridorIn, const IntVector2& startCoords, const IntVector2& endCoords, std::string corridorTile, std::string roomFloorTile);
	void UpdateFogOfWar();
	void UpdateVisibilities();
	void CheckForVictory();
};
