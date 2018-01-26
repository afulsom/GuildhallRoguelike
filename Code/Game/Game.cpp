#include "Game/Game.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/Disc2D.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Audio/Audio.hpp"
#include "Engine/Core/EngineConfig.hpp"
#include "Engine/Core/ConfigSystem.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Game/App.hpp"
#include "Game/MapDefinition.hpp"
#include "CharacterBuilder.hpp"
#include "LootTable.hpp"
#include "Adventure.hpp"


Game::Game()
	: isGamePaused(false)
	, m_theWorld(nullptr)
{
}


Game::~Game()
{

}


void Game::Initialize()
{
	LoadGameConstants();
	LoadLootTables();
	LoadTileDefinitions();
	LoadCharacterBuilders();
	LoadItemDefinitions();
	LoadFeaturePrototypes();
	LoadMapDefinitions();
	LoadAdventures();
}

void Game::Update(float deltaSeconds)
{
	switch (m_currentState)
	{
	case STATE_MAINMENU:
		UpdateMainMenu(deltaSeconds);
		break;
	case STATE_PLAYING:
		UpdatePlaying(deltaSeconds);
		break;
	case STATE_STATSSCREEN:
		UpdateStatsScreen(deltaSeconds);
		break;
	case STATE_GENERATING:
		UpdateGenerating(deltaSeconds);
		break;
	case STATE_PATHING:
		UpdatePathing(deltaSeconds);
		break;
	}
}

void Game::Render() const
{
	switch (m_currentState)
	{
	case STATE_MAINMENU:
		RenderMainMenu();
		break;
	case STATE_PLAYING:
		RenderPlaying();
		break;
	case STATE_STATSSCREEN:
		RenderStatsScreen();
		break;
	case STATE_GENERATING:
		RenderGenerating();
		break;
	case STATE_PATHING:
		RenderPathing();
		break;
	}
}

void Game::UpdateMainMenu(float deltaSeconds)
{
	if (m_theWorld && g_theInput->WasKeyJustPressed('1'))
	{
		m_currentState = STATE_PLAYING;
		UpdatePlaying(deltaSeconds);
	}
	else if (g_theInput->WasKeyJustPressed('2'))
	{
		StartAdventure("Test");
		m_currentState = STATE_PLAYING;
	}
	else if (g_theInput->WasKeyJustPressed('3'))
	{
		if (m_theWorld == nullptr)
		{
			m_theWorld = new World();
			m_theWorld->Initialize();
		}

		Map* newMap = m_theWorld->GenerateMap("Rooms");
		m_theWorld->m_currentMap = newMap;
		m_theWorld->m_maps[0] = newMap;
		m_theWorld->m_thePlayer = CharacterBuilder::BuildNewCharacter("player");
		m_theWorld->m_currentMap->PlaceCharacterInMap(m_theWorld->m_thePlayer, m_theWorld->m_currentMap->GetRandomTraversableTile());
		m_currentState = STATE_PLAYING;
	}
	else if (g_theInput->WasKeyJustPressed('4'))
	{
		if (m_theWorld == nullptr)
		{
			m_theWorld = new World();
			m_theWorld->Initialize();
		}

		m_theWorld->StartSteppedGeneration("CATest");
		m_theWorld->m_currentMap = m_theWorld->m_currentlyGeneratingMap;
		m_theWorld->m_maps[0] = m_theWorld->m_currentlyGeneratingMap;
		m_theWorld->m_thePlayer = CharacterBuilder::BuildNewCharacter("player");
		m_theWorld->m_currentMap->PlaceCharacterInMap(m_theWorld->m_thePlayer, m_theWorld->m_currentMap->GetRandomTraversableTile());
		m_currentState = STATE_GENERATING;
	}
	else if (g_theInput->WasKeyJustPressed('5'))
	{
		if (m_theWorld == nullptr)
		{
			m_theWorld = new World();
			m_theWorld->Initialize();
		}

		m_theWorld->StartSteppedGeneration("PerlinTest");
		m_theWorld->m_currentMap = m_theWorld->m_currentlyGeneratingMap;
		m_theWorld->m_maps[0] = m_theWorld->m_currentlyGeneratingMap;
		m_theWorld->m_thePlayer = CharacterBuilder::BuildNewCharacter("player");
		m_theWorld->m_currentMap->PlaceCharacterInMap(m_theWorld->m_thePlayer, m_theWorld->m_currentMap->GetRandomTraversableTile());
		m_currentState = STATE_GENERATING;
	}


	if (g_theInput->WasKeyJustPressed(KEYCODE_ESCAPE))
		g_theApp->SetIsQuitting(true);
}

void Game::UpdatePlaying(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESCAPE))
		m_currentState = STATE_MAINMENU;

	if (g_theInput->WasKeyJustPressed('C'))
		m_currentState = STATE_STATSSCREEN;

	if (g_theInput->WasKeyJustPressed('P'))
	{
		m_theWorld->m_currentMap->StartSteppedPath(m_theWorld->m_thePlayer->m_currentTile->m_tileCoords, m_theWorld->m_thePlayer->m_currentTile->m_tileCoords + IntVector2(5, 0), m_theWorld->m_thePlayer);
		m_currentState = STATE_PATHING;
	}

	m_theWorld->Update(deltaSeconds);
}

void Game::UpdateStatsScreen(float deltaSeconds)
{
	switch (m_currentPlayerInventoryManagementState)
	{
	case STATE_NONE:
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_ESCAPE))
		{
			m_currentState = STATE_PLAYING;
		}

		// equipping/unequipping
		if (g_theInput->WasKeyJustPressed('W'))
		{
			m_currentPlayerInventoryManagementState = STATE_EQUIPPING;
			
		}
		// using
		else if (g_theInput->WasKeyJustPressed('U'))
		{
			m_currentPlayerInventoryManagementState = STATE_USING;

		}
		// dropping
		else if (g_theInput->WasKeyJustPressed('D'))
		{
			m_currentPlayerInventoryManagementState = STATE_DROPPING;

		}

		break;
	}
	case STATE_EQUIPPING:
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_ESCAPE))
		{
			m_currentPlayerInventoryManagementState = STATE_NONE;
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_LMB))
		{
			Item* clickedItem = GetItemFromMouseClick(m_theWorld->m_cursorPosition);
			if (clickedItem)
			{
				if (m_theWorld->m_thePlayer->m_equipment.IsItemEquipped(clickedItem))
				{
					m_theWorld->m_thePlayer->m_equipment.m_equippedItems[clickedItem->m_definition->m_slot] = nullptr;
				}
				else
				{
					m_theWorld->m_thePlayer->m_equipment.m_equippedItems[clickedItem->m_definition->m_slot] = clickedItem;
				}
			}
		}

		break;
	}
	case STATE_USING:
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_ESCAPE))
		{
			m_currentPlayerInventoryManagementState = STATE_NONE;
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_LMB))
		{
			Item* clickedItem = GetItemFromMouseClick(m_theWorld->m_cursorPosition);
			if (clickedItem)
			{
				clickedItem->Use();
			}
		}


		break;
	}
	case STATE_DROPPING:
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_ESCAPE))
		{
			m_currentPlayerInventoryManagementState = STATE_NONE;
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_LMB))
		{
			int clickedItemIndex = GetItemIndexFromMouseClick(m_theWorld->m_cursorPosition);
			if (clickedItemIndex >= 0)
			{
				Item* clickedItem = m_theWorld->m_thePlayer->m_entityInventory.m_items[clickedItemIndex];
				if (m_theWorld->m_thePlayer->m_equipment.IsItemEquipped(clickedItem))
					m_theWorld->m_thePlayer->m_equipment.m_equippedItems[clickedItem->m_definition->m_slot] = nullptr;

				m_theWorld->m_thePlayer->m_entityInventory.TransferSingleItemToOtherInventory(clickedItemIndex, m_theWorld->m_thePlayer->m_currentTile->m_tileInventory);
			}
		}

		break;
	}
	}

	m_theWorld->Update(deltaSeconds);
}

void Game::UpdateGenerating(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		bool isCompleted = m_theWorld->StepGeneration();
		if (isCompleted)
		{
			m_theWorld->m_maps.push_back(m_theWorld->m_currentlyGeneratingMap);
			m_currentState = STATE_PLAYING;
		}
	}
	else if (g_theInput->WasKeyJustPressed(KEYCODE_ENTER))
	{
		m_theWorld->FinishGeneratingMap();
		m_theWorld->m_maps.push_back(m_theWorld->m_currentlyGeneratingMap);
		m_currentState = STATE_PLAYING;
	}

	m_theWorld->Update(deltaSeconds);
}

void Game::UpdatePathing(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		m_currentState = STATE_PLAYING;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		Path outPath;
		m_theWorld->m_currentMap->ContinueSteppedPath(outPath);
	}
	else if (g_theInput->WasKeyJustPressed(KEYCODE_ENTER))
	{
		bool isCompleted = false;
		Path outPath;
		while (!isCompleted)
		{
			isCompleted = m_theWorld->m_currentMap->ContinueSteppedPath(outPath);
		}
	}

	m_theWorld->Update(deltaSeconds);
}

void Game::RenderMainMenu() const
{
	g_theRenderer->SetTexture(nullptr);
	g_theRenderer->ClearColor(Rgba::BLACK);
	
	Rgba resumeColor;
	if (m_theWorld)
		resumeColor = Rgba::WHITE;
	else
		resumeColor = Rgba::LIGHT_GREY;

	g_theRenderer->DrawCenteredText2D(Vector2(ORTHO_X_DIMENSION * 0.5f, ORTHO_Y_DIMENSION - 3.f), g_theRenderer->m_defaultFont, "Roguelike", Rgba::WHITE, 3.f);
	g_theRenderer->DrawCenteredText2D(Vector2(ORTHO_X_DIMENSION * 0.5f, 5.f), g_theRenderer->m_defaultFont, "1. Resume Game", resumeColor, 1.f);
	g_theRenderer->DrawCenteredText2D(Vector2(ORTHO_X_DIMENSION * 0.5f, 4.f), g_theRenderer->m_defaultFont, "2. Test Adventure", Rgba::WHITE, 1.f);
	g_theRenderer->DrawCenteredText2D(Vector2(ORTHO_X_DIMENSION * 0.5f, 3.f), g_theRenderer->m_defaultFont, "3. Dungeon", Rgba::WHITE, 1.f);
	g_theRenderer->DrawCenteredText2D(Vector2(ORTHO_X_DIMENSION * 0.5f, 2.f), g_theRenderer->m_defaultFont, "4. Stepped Thick Walls Using CA", Rgba::WHITE, 1.f);
	g_theRenderer->DrawCenteredText2D(Vector2(ORTHO_X_DIMENSION * 0.5f, 1.f), g_theRenderer->m_defaultFont, "5. Stepped Perlin Noise", Rgba::WHITE, 1.f);
}

void Game::RenderPlaying() const
{
	if(m_theWorld)
	{
		m_theWorld->Render();
		m_theWorld->DrawUI();
		m_theWorld->DrawCursor();
	}
}

void Game::RenderStatsScreen() const
{
	if (m_theWorld)
		m_theWorld->Render();

	g_theRenderer->SetTexture(nullptr);
	g_theRenderer->DrawQuad2D(0.f, 0.f, ORTHO_X_DIMENSION, ORTHO_Y_DIMENSION, Rgba(0, 0, 0, 90));
	DrawStatsScreen();

	if(m_theWorld)
		m_theWorld->DrawCursor();

}

void Game::RenderGenerating() const
{
	if (m_theWorld)
	{
		m_theWorld->Render();
		m_theWorld->m_currentlyGeneratingMap->RenderDebugGenerating();
		m_theWorld->DrawUI();
		m_theWorld->DrawCursor();
	}
}

void Game::RenderPathing() const
{
	if (m_theWorld)
	{
		m_theWorld->Render();
		g_theRenderer->SetTexture(nullptr);
		g_theRenderer->DrawQuad2D(0.f, 0.f, ORTHO_X_DIMENSION, ORTHO_Y_DIMENSION, Rgba(0, 0, 0, 90));
		m_theWorld->m_currentMap->RenderDebugPathing();
		m_theWorld->DrawUI();
		m_theWorld->DrawCursor();
	}
}

void Game::LoadTileDefinitions()
{
	std::string tileDefinitionFileName = "Data/Gameplay/Tiles.xml";
	g_theConfig->GetConfigString(tileDefinitionFileName, "TileDefinitionFileName");

	XMLNode tileDefinitionsHead = XMLNode::parseFile(tileDefinitionFileName.c_str(), "TileDefinitions");
	for (int tileDefIndex = 0; tileDefIndex < tileDefinitionsHead.nChildNode("TileDefinition"); tileDefIndex++)
	{
		XMLNode tileDef = tileDefinitionsHead.getChildNode("TileDefinition", tileDefIndex);
		new TileDefinition(tileDef);
	}
}

void Game::LoadMapDefinitions()
{
	std::string mapFileName = "Data/Gameplay/Maps.xml";

	XMLNode mapsHead = XMLNode::parseFile(mapFileName.c_str(), "MapDefinitions");
	for (int mapsIndex = 0; mapsIndex < mapsHead.nChildNode("MapDefinition"); mapsIndex++)
	{
		XMLNode mapDefinition = mapsHead.getChildNode("MapDefinition", mapsIndex);
		new MapDefinition(mapDefinition);
	}
}

void Game::LoadAdventures()
{
	std::string adventuresFileName = "Data/Gameplay/Adventures.xml";
	g_theConfig->GetConfigString(adventuresFileName, "AdventuresFileName");

	XMLNode adventuresHead = XMLNode::parseFile(adventuresFileName.c_str(), "Adventures");
	for (int adventuresIndex = 0; adventuresIndex < adventuresHead.nChildNode("Adventure"); adventuresIndex++)
	{
		XMLNode adventureNode = adventuresHead.getChildNode("Adventure", adventuresIndex);
		new Adventure(adventureNode);
	}
}

void Game::LoadCharacterBuilders()
{
	std::string charactersFileName = "Data/Gameplay/Characters.xml";
	g_theConfig->GetConfigString(charactersFileName, "CharactersFileName");

	XMLNode charactersHead = XMLNode::parseFile(charactersFileName.c_str(), "Characters");
	for (int characterIndex = 0; characterIndex < charactersHead.nChildNode("Character"); characterIndex++)
	{
		XMLNode characterBuilder = charactersHead.getChildNode("Character", characterIndex);
		new CharacterBuilder(characterBuilder);
	}
}

void Game::LoadItemDefinitions()
{
	std::string itemsFileName = "Data/Gameplay/Items.xml";
	g_theConfig->GetConfigString(itemsFileName, "ItemsFileName");

	XMLNode itemsHead = XMLNode::parseFile(itemsFileName.c_str(), "ItemDefinitions");
	for (int itemIndex = 0; itemIndex < itemsHead.nChildNode("ItemDefinition"); itemIndex++)
	{
		XMLNode itemDefinition = itemsHead.getChildNode("ItemDefinition", itemIndex);
		new ItemDefinition(itemDefinition);
	}
}

void Game::LoadFeaturePrototypes()
{
	std::string featuresFileName = "Data/Gameplay/Features.xml";
	g_theConfig->GetConfigString(featuresFileName, "FeaturesFileName");

	XMLNode featuresHead = XMLNode::parseFile(featuresFileName.c_str(), "Features");
	for (int featureIndex = 0; featureIndex < featuresHead.nChildNode("Feature"); featureIndex++)
	{
		XMLNode feature = featuresHead.getChildNode("Feature", featureIndex);
		new Feature(feature);
	}
}

void Game::LoadLootTables()
{
	std::string lootFileName = "Data/Gameplay/Loot.xml";
	g_theConfig->GetConfigString(lootFileName, "LootFileName");

	XMLNode lootHead = XMLNode::parseFile(lootFileName.c_str(), "LootTables");
	for (int lootIndex = 0; lootIndex < lootHead.nChildNode("LootTable"); lootIndex++)
	{
		XMLNode loot = lootHead.getChildNode("LootTable", lootIndex);
		new LootTable(loot);
	}
}

void Game::LoadGameConstants()
{
	std::string constantsFileName = "Data/Gameplay/GameConstants.xml";
	g_theConfig->GetConfigString(constantsFileName, "ConstantsFileName");

	XMLNode constantsHead = XMLNode::parseFile(constantsFileName.c_str(), "Constants");

	XMLNode baseChanceToHitNode = constantsHead.getChildNode("BaseChanceToHit");
	BASE_CHANCE_TO_HIT = ParseXMLAttributeFloat(baseChanceToHitNode, "chance", 0.f);

	XMLNode chanceToHitPerAgilityNode = constantsHead.getChildNode("ChanceToHitPerAgility");
	CHANCE_TO_HIT_PER_AGILITY = ParseXMLAttributeFloat(chanceToHitPerAgilityNode, "chance", 0.f);

	XMLNode baseCriticalChanceNode = constantsHead.getChildNode("BaseCriticalChance");
	BASE_CRITICAL_CHANCE = ParseXMLAttributeFloat(baseCriticalChanceNode, "chance", 0.f);

	XMLNode criticalChancePerLuckNode = constantsHead.getChildNode("CriticalChancePerLuck");
	CRITICAL_CHANCE_PER_LUCK = ParseXMLAttributeFloat(criticalChancePerLuckNode, "chance", 0.f);

	XMLNode criticalMultiplierNode = constantsHead.getChildNode("CriticalMultiplier");
	CRITICAL_MULTIPLIER = ParseXMLAttributeFloat(criticalMultiplierNode, "multiplier", 1.f);
}

void Game::DrawPlayerStats() const
{
	Stats basePlayerStats = m_theWorld->m_thePlayer->m_stats;
	g_theRenderer->DrawText2D(Vector2(0.5f, ORTHO_Y_DIMENSION - 1.f), g_theRenderer->m_defaultFont, "HP: " + std::to_string(m_theWorld->m_thePlayer->m_currentHP) + "/" + std::to_string(basePlayerStats[STAT_MAX_HP]), Rgba::WHITE, 1.f);
	g_theRenderer->DrawText2D(Vector2(0.5f, ORTHO_Y_DIMENSION - 2.f), g_theRenderer->m_defaultFont, "Base Stats:", Rgba::WHITE, 1.f);
	g_theRenderer->DrawText2D(Vector2(0.5f, ORTHO_Y_DIMENSION - 3.f), g_theRenderer->m_defaultFont, "St: " + std::to_string(basePlayerStats[STAT_STRENGTH]), Rgba::WHITE, 0.9f);
	g_theRenderer->DrawText2D(Vector2(0.5f, ORTHO_Y_DIMENSION - 4.f), g_theRenderer->m_defaultFont, "Ag: " + std::to_string(basePlayerStats[STAT_AGILITY]), Rgba::WHITE, 0.9f);
	g_theRenderer->DrawText2D(Vector2(0.5f, ORTHO_Y_DIMENSION - 5.f), g_theRenderer->m_defaultFont, "Ma: " + std::to_string(basePlayerStats[STAT_MAGIC]), Rgba::WHITE, 0.9f);
	g_theRenderer->DrawText2D(Vector2(0.5f, ORTHO_Y_DIMENSION - 6.f), g_theRenderer->m_defaultFont, "En: " + std::to_string(basePlayerStats[STAT_ENDURANCE]), Rgba::WHITE, 0.9f);
	g_theRenderer->DrawText2D(Vector2(0.5f, ORTHO_Y_DIMENSION - 7.f), g_theRenderer->m_defaultFont, "Lu: " + std::to_string(basePlayerStats[STAT_LUCK]), Rgba::WHITE, 0.9f);

	Stats modifiedPlayerStats = basePlayerStats + m_theWorld->m_thePlayer->m_equipment.CalculateCombinedStatModifiers();
	g_theRenderer->DrawText2D(Vector2(0.5f, ORTHO_Y_DIMENSION - 9.f), g_theRenderer->m_defaultFont, "Modified Stats:", Rgba::WHITE, 1.f);
	g_theRenderer->DrawText2D(Vector2(0.5f, ORTHO_Y_DIMENSION - 10.f), g_theRenderer->m_defaultFont, "St: " + std::to_string(modifiedPlayerStats[STAT_STRENGTH]), Rgba::WHITE, 0.9f);
	g_theRenderer->DrawText2D(Vector2(0.5f, ORTHO_Y_DIMENSION - 11.f), g_theRenderer->m_defaultFont, "Ag: " + std::to_string(modifiedPlayerStats[STAT_AGILITY]), Rgba::WHITE, 0.9f);
	g_theRenderer->DrawText2D(Vector2(0.5f, ORTHO_Y_DIMENSION - 12.f), g_theRenderer->m_defaultFont, "Ma: " + std::to_string(modifiedPlayerStats[STAT_MAGIC]), Rgba::WHITE, 0.9f);
	g_theRenderer->DrawText2D(Vector2(0.5f, ORTHO_Y_DIMENSION - 13.f), g_theRenderer->m_defaultFont, "En: " + std::to_string(modifiedPlayerStats[STAT_ENDURANCE]), Rgba::WHITE, 0.9f);
	g_theRenderer->DrawText2D(Vector2(0.5f, ORTHO_Y_DIMENSION - 14.f), g_theRenderer->m_defaultFont, "Lu: " + std::to_string(modifiedPlayerStats[STAT_LUCK]), Rgba::WHITE, 0.9f);
}

void Game::DrawPlayerInventory() const
{
	g_theRenderer->DrawText2D(Vector2(ORTHO_X_DIMENSION * 0.5f, ORTHO_Y_DIMENSION - 1.f), g_theRenderer->m_defaultFont, "Inventory:", Rgba::WHITE, 1.f);

	Vector2 drawPosition(ORTHO_X_DIMENSION * 0.5f, ORTHO_Y_DIMENSION - 2.f);
	float lineScale = 0.75f;
	for (Item* item : m_theWorld->m_thePlayer->m_entityInventory.m_items)
	{
		if(!m_theWorld->m_thePlayer->m_equipment.IsItemEquipped(item))
		{
			g_theRenderer->DrawText2D(drawPosition, g_theRenderer->m_defaultFont, std::string(1, item->m_definition->m_glyph) + " - " + item->m_definition->m_name, Rgba::WHITE, lineScale);
			drawPosition.y -= lineScale;
			if (!item->m_damageTypes.GetTagsAsString().empty())
			{
				g_theRenderer->DrawText2D(drawPosition, g_theRenderer->m_defaultFont, "  Elements: " + item->m_damageTypes.GetTagsAsString(), Rgba::WHITE, lineScale * 0.5f);
				drawPosition.y -= lineScale * 0.5f;
			}
		}
	}
}

void Game::DrawPlayerEquipment() const
{
	g_theRenderer->DrawText2D(Vector2(ORTHO_X_DIMENSION * 0.75f, ORTHO_Y_DIMENSION - 1.f), g_theRenderer->m_defaultFont, "Equipped Items:", Rgba::WHITE, 1.f);

	Vector2 drawPosition(ORTHO_X_DIMENSION * 0.75f, ORTHO_Y_DIMENSION - 2.f);
	float lineScale = 0.75f;
	for (Item* equippedItem : m_theWorld->m_thePlayer->m_equipment.m_equippedItems)
	{
		if (equippedItem)
		{
			g_theRenderer->DrawText2D(drawPosition, g_theRenderer->m_defaultFont, ItemDefinition::GetStringFromEquipSlot(equippedItem->m_definition->m_slot) + ": " + std::string(1, equippedItem->m_definition->m_glyph) + " - " + equippedItem->m_definition->m_name, Rgba::WHITE, lineScale);
			drawPosition.y -= lineScale;
			if(!equippedItem->m_damageTypes.GetTagsAsString().empty())
			{
				g_theRenderer->DrawText2D(drawPosition, g_theRenderer->m_defaultFont, "  Elements: " + equippedItem->m_damageTypes.GetTagsAsString(), Rgba::WHITE, lineScale * 0.5f);
				drawPosition.y -= lineScale * 0.5f;
			}
		}
	}
}

Item* Game::GetItemFromMouseClick(const Vector2& cursorPosition)
{
	//unequipped items
	float lineScale = 0.75f;
	Vector2 AABB2StartingPosition(ORTHO_X_DIMENSION * 0.5f, ORTHO_Y_DIMENSION - 2.f);

	for (Item* item : m_theWorld->m_thePlayer->m_entityInventory.m_items)
	{
		if (!m_theWorld->m_thePlayer->m_equipment.IsItemEquipped(item))
		{
			AABB2StartingPosition.y -= lineScale;
			std::string itemText = std::string(1, item->m_definition->m_glyph) + " - " + item->m_definition->m_name;
			std::string elementText = "";

			if (!item->m_damageTypes.GetTagsAsString().empty())
				elementText = "  Elements: " + item->m_damageTypes.GetTagsAsString();

			float AABB2Width = max(g_theRenderer->m_defaultFont->CalculateTextWidth(itemText, lineScale), g_theRenderer->m_defaultFont->CalculateTextWidth(elementText, lineScale));
			float AABB2Height = g_theRenderer->m_defaultFont->CalculateTextHeight(itemText, lineScale) + g_theRenderer->m_defaultFont->CalculateTextHeight(elementText, lineScale * 0.5f);
			AABB2 itemBox(AABB2StartingPosition, AABB2StartingPosition + Vector2(AABB2Width, AABB2Height));
			if (itemBox.IsPointInside(cursorPosition))
				return item;
		}
	}


	//equipped items
	lineScale = 0.75f;
	AABB2StartingPosition = Vector2(ORTHO_X_DIMENSION * 0.75f, ORTHO_Y_DIMENSION - 2.f);
	for (Item* equippedItem : m_theWorld->m_thePlayer->m_equipment.m_equippedItems)
	{
		if (equippedItem)
		{
			AABB2StartingPosition.y -= lineScale;
			std::string itemText = ItemDefinition::GetStringFromEquipSlot(equippedItem->m_definition->m_slot) + ": " + std::string(1, equippedItem->m_definition->m_glyph) + " - " + equippedItem->m_definition->m_name;
			std::string elementText = "";

			if (!equippedItem->m_damageTypes.GetTagsAsString().empty())
				elementText = "  Elements: " + equippedItem->m_damageTypes.GetTagsAsString();

			float AABB2Width = max(g_theRenderer->m_defaultFont->CalculateTextWidth(itemText, lineScale), g_theRenderer->m_defaultFont->CalculateTextWidth(itemText, lineScale));
			float AABB2Height = g_theRenderer->m_defaultFont->CalculateTextHeight(itemText, lineScale) + g_theRenderer->m_defaultFont->CalculateTextHeight(elementText, lineScale * 0.5f);
			AABB2 itemBox(AABB2StartingPosition, AABB2StartingPosition + Vector2(AABB2Width, AABB2Height));
			if (itemBox.IsPointInside(cursorPosition))
				return equippedItem;
		}
	}

	return nullptr;
}

int Game::GetItemIndexFromMouseClick(const Vector2& cursorPosition)
{
	//unequipped items
	float lineScale = 0.75f;
	Vector2 AABB2StartingPosition(ORTHO_X_DIMENSION * 0.5f, ORTHO_Y_DIMENSION - 2.f);

	for (size_t itemIndex = 0; itemIndex < m_theWorld->m_thePlayer->m_entityInventory.m_items.size(); itemIndex++)
	{
		Item* item = m_theWorld->m_thePlayer->m_entityInventory.m_items[itemIndex];
		if (!m_theWorld->m_thePlayer->m_equipment.IsItemEquipped(item))
		{
			AABB2StartingPosition.y -= lineScale;
			std::string itemText = std::string(1, item->m_definition->m_glyph) + " - " + item->m_definition->m_name;
			float AABB2Width = g_theRenderer->m_defaultFont->CalculateTextWidth(itemText, lineScale);
			float AABB2Height = g_theRenderer->m_defaultFont->CalculateTextHeight(itemText, lineScale);
			AABB2 itemBox(AABB2StartingPosition, AABB2StartingPosition + Vector2(AABB2Width, AABB2Height));
			if (itemBox.IsPointInside(cursorPosition))
				return itemIndex;
		}
	}


	//equipped items
	lineScale = 0.75f;
	AABB2StartingPosition = Vector2(ORTHO_X_DIMENSION * 0.75f, ORTHO_Y_DIMENSION - 2.f);
	for (int equippedItemIndex = 0; equippedItemIndex < NUM_EQUIP_SLOTS; equippedItemIndex++)
	{
		Item* equippedItem = m_theWorld->m_thePlayer->m_equipment.m_equippedItems[equippedItemIndex];
		if (equippedItem)
		{
			AABB2StartingPosition.y -= lineScale;
			std::string itemText = ItemDefinition::GetStringFromEquipSlot(equippedItem->m_definition->m_slot) + ": " + std::string(1, equippedItem->m_definition->m_glyph) + " - " + equippedItem->m_definition->m_name;
			float AABB2Width = g_theRenderer->m_defaultFont->CalculateTextWidth(itemText, lineScale);
			float AABB2Height = g_theRenderer->m_defaultFont->CalculateTextHeight(itemText, lineScale);
			AABB2 itemBox(AABB2StartingPosition, AABB2StartingPosition + Vector2(AABB2Width, AABB2Height));
			if (itemBox.IsPointInside(cursorPosition))
				return equippedItemIndex;
		}
	}

	return -1;
}

void Game::DrawStatsScreen() const
{
	g_theRenderer->DrawText2D(Vector2(0.5f, ORTHO_Y_DIMENSION), g_theRenderer->m_defaultFont, "ESC: Close menu. W: Equip/Unequip Items. U: Use Items. D: Drop items.", Rgba::LIGHT_GREY, 0.6f);
	
	switch (m_currentPlayerInventoryManagementState)
	{
	case STATE_NONE:
		break;
	case STATE_EQUIPPING:
		g_theRenderer->DrawCenteredText2D(Vector2(ORTHO_X_DIMENSION * 0.5f, 1.f), g_theRenderer->m_defaultFont, "Equipping items. Click the item you want to equip or unequip. ESC: Stop equipping.", Rgba::WHITE, 0.75f);
		break;
	case STATE_USING:
		g_theRenderer->DrawCenteredText2D(Vector2(ORTHO_X_DIMENSION * 0.5f, 1.f), g_theRenderer->m_defaultFont, "Using items. Click the item you want to use. ESC: Stop using items.", Rgba::WHITE, 0.75f);
		break;
	case STATE_DROPPING:
		g_theRenderer->DrawCenteredText2D(Vector2(ORTHO_X_DIMENSION * 0.5f, 1.f), g_theRenderer->m_defaultFont, "Dropping items. Click the item you want to drop. ESC: Stop dropping items.", Rgba::WHITE, 0.75f);
		break;
	}

	DrawPlayerStats();
	DrawPlayerInventory();
	DrawPlayerEquipment();	
}

void Game::StartAdventure(std::string adventureName)
{
	if(m_theWorld != nullptr)
	{
		delete m_theWorld;
		m_theWorld = nullptr;
	}

	m_theWorld = new World();
	m_theWorld->GenerateAdventure(adventureName);
}
