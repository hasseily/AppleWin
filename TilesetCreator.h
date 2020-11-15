#pragma once

constexpr auto ResMX	= 2;	// resolution multiplier (AppleWin multiplies resolution by 2)
constexpr auto TW		= 14;	// tile width in pixels
constexpr auto TH		= 16;	// tile height in pixels
constexpr auto TILESPERROW	= 16;
constexpr auto TILESPERCOL	= 16;
constexpr auto PIXELSPERROW	= TILESPERROW * TW;

class TilesetCreator
{
	TilesetCreator();
	~TilesetCreator();
	bool insertTileInTilesetBuffer(UINT8 iTileId, UINT32* pFramePixelOrigin, UINT32 iFramePixelWidth);
	bool saveTilesetPNG(std::string filepath);
private:
	UINT32* pTilesetBuffer;
	UINT8 aKnownTiles[UINT8_MAX] = {};
};

