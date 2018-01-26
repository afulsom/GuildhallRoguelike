#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/RHI/SpriteSheet2D.hpp"
#include "Engine/Audio/Audio.hpp"
#include "Game/World.hpp"

enum GameState
{
	STATE_MAINMENU,
	STATE_PLAYING,
	STATE_STATSSCREEN,
	STATE_GENERATING,
	STATE_PATHING
};

enum InventoryManagementState
{
	STATE_NONE,
	STATE_EQUIPPING,
	STATE_USING,
	STATE_DROPPING
};

class Game
{
private:
	bool isGamePaused;

	void DrawStatsScreen() const;
	void StartAdventure(std::string adventureName);
public:
	World* m_theWorld;
	GameState m_currentState = STATE_MAINMENU;

	InventoryManagementState m_currentPlayerInventoryManagementState = STATE_NONE;

	Game();
	~Game();

	void Initialize();

	void Update(float deltaSeconds);
	void Render() const;


private:
	void UpdateMainMenu(float deltaSeconds);
	void UpdatePlaying(float deltaSeconds);
	void UpdateStatsScreen(float deltaSeconds);
	void UpdateGenerating(float deltaSeconds);
	void UpdatePathing(float deltaSeconds);

	void RenderMainMenu() const;
	void RenderPlaying() const;
	void RenderStatsScreen() const;
	void RenderGenerating() const;
	void RenderPathing() const;

	void LoadTileDefinitions();
	void LoadCharacterBuilders();
	void LoadItemDefinitions();
	void LoadFeaturePrototypes();
	void LoadLootTables();
	void LoadMapDefinitions();
	void LoadAdventures();
	void LoadGameConstants();

	void DrawPlayerStats() const;
	void DrawPlayerInventory() const;
	void DrawPlayerEquipment() const;
	Item* GetItemFromMouseClick(const Vector2& cursorPosition);
	int GetItemIndexFromMouseClick(const Vector2& cursorPosition);
};