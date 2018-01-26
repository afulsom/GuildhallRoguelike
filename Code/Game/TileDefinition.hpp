#pragma once
#include <string>
#include <vector>
#include <map>
#include "Engine\Core\Rgba.hpp"

struct XMLNode;

class TileDefinition
{
public:
	TileDefinition(XMLNode element);
	~TileDefinition();

	std::string m_name;
	bool m_isSolid;
	bool m_isOpaque;
	std::string m_solidExceptions;
	std::vector<char> m_glyphs;
	std::vector<Rgba> m_glyphColors;
	std::vector<Rgba> m_fillColors;

	static std::map<std::string, TileDefinition*> s_tileDefinitionRegistry;
	static TileDefinition* GetTileDefinition(std::string name);
};