#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"
#include "Game/Message.hpp"
#include <set>


typedef std::vector<Tile*> Path;

class MapDefinition;
class Map;

struct DamageNumber
{
	DamageNumber::DamageNumber(std::string number, const Vector2& position, const Rgba& color = Rgba::RED, float scale = 0.5f);

	Rgba m_color;
	std::string m_number;
	Vector2 m_position;
	float m_scale;
	float m_lifetime;
};

struct RaycastResult
{
	bool m_didImpact;
	float m_impactFraction;
	Vector2 m_pointBeforeImpact;
	Vector2 m_impactPosition;
	Vector2 m_impactNormal;
	std::set<Tile*> m_impactedTiles;
};

struct OpenNode
{
	Tile* m_tile;
	OpenNode* m_parent;
	float m_localGCost = 1.f;
	float m_totalGCost = 0.f;
	float m_estimatedDistToGoal = 0.f;
	float m_fScore = 0.f;
};

class PathGenerator
{
	friend class Map;

private:
	PathGenerator(const IntVector2& start, const IntVector2& end, Map* map, Character* gCostReferenceCharacter);

	void OpenNodeForProcessing(Tile& tileToOpen, OpenNode* parent);
	OpenNode* SelectAndCloseBestOpenNode();
	Path CreateFinalPath(OpenNode& endNode);
	void OpenNodeIfValid(Tile* tileToOpen, OpenNode* parent);

	IntVector2 m_start;
	IntVector2 m_end;
	Map* m_map = nullptr;
	Character* m_gCostReferenceCharacter = nullptr;
	std::vector<OpenNode*> m_openList;
	int m_pathID;

	Path m_finalPath;
};

class Map
{
public:
	Map(std::string mapDefinitionName);
	~Map();

	void Update(float deltaSeconds);
	void Render() const;

	void RenderDebugPathing() const;
	void RenderDebugGenerating() const;

	void AdvanceTurns();

	int CalculateTileIndexFromTileCoords(const IntVector2& tileCoords) const;
	IntVector2 CalculateTileCoordsFromTileIndex(int tileIndex) const;
	IntVector2 CalculateTileCoordsFromMapCoords(const Vector2& mapCoords) const;
	Tile* GetTileAtMapCoords(const Vector2& mapCoords);
	Tile* GetTileAtTileCoords(const IntVector2& tileCoords);
	Tile* GetTileAtTileIndex(int tileIndex);
	Tile* FindFirstTraversableTile();
	Tile* GetRandomTraversableTile();
	Tile* GetRandomTileOfType(std::string tileType);
	Tile* GetRandomTileWithTags(std::string m_patrolPointTags);
	Tile* GetRandomTile();
	bool IsInMap(const IntVector2& tileCoords) const;

	std::vector<Message> GetTooltipInfoForMapCoords(const Vector2& mapCoords);

	bool TryToMoveCharacterToTile(Character* characterToMove, Tile* destinationTile);
	void DestroyCharacter(Character* entityToKill);
	void DestroyFeature(Feature* entityToKill);

	void PlaceCharacterInMap(Character* characterToPlace, Tile* destinationTile);
	void PlaceFeatureInMap(Feature* featureToPlace, Tile* destinatonTile);
	void PlaceItemInMap(Item* itemToPlace, Tile* tileToPlaceIn);

	void SpawnActors();

	int CalculateManhattanDistance(const Tile& tileA, const Tile& tileB);
	Entity* FindNearestEntityOfType(const IntVector2& startingPosition, std::string type);
	Entity* FindNearestEntityNotOfType(const IntVector2& startingPosition, std::string type);
	Character* FindNearestCharacterOfFaction(const IntVector2& startingPosition, std::string faction);
	Character* FindNearestCharacterNotOfFaction(const IntVector2& startingPosition, std::string faction);
	std::vector<Character*> FindAllCharactersOfFaction(std::string faction);
	std::vector<Character*> FindAllCharactersNotOfFaction(std::string faction);
	Tile* FindNearestTileOfType(const IntVector2& startingPosition, std::string type);
	Tile* FindNearestTileNotOfType(const IntVector2& startingPosition, std::string type);
	std::vector<Tile*> GetTilesInRadius(const IntVector2& tileCoords, float radius);

	RaycastResult RaycastForSolid(const Vector2& startPosition, const Vector2& direction, float maxDistance);
	RaycastResult RaycastForOpaque(const Vector2& startPosition, const Vector2& direction, float maxDistance);

	Path GeneratePath(const IntVector2& start, const IntVector2& end, Character* characterForPath = nullptr);
	void StartSteppedPath(const IntVector2& start, const IntVector2& end, Character* characterForPath = nullptr);
	bool ContinueSteppedPath(Path& out_pathWhenComplete);


	std::string m_name;
	MapDefinition* m_definition;
	std::vector<Tile> m_tiles;
	std::vector<Entity*> m_entities;
	std::vector<DamageNumber> m_damageNumbers;

	PathGenerator* m_currentPath = nullptr;

	static const float DAMAGE_NUMBER_LIFETIME;
	std::vector<Character *> FindAllCharacters();
private:
	void SpawnFeatures();
	void DestroyEntity(Entity* entityToKill);
	void PlaceEntityInMap(Entity* entityToPlace, Tile* destinationTile);
	void MoveCharacterToTile(Character* characterToMove, Tile* destinationTile);
	void UpdateDamageNumbers(float deltaSeconds);
	void RenderDamageNumbers() const;

};
