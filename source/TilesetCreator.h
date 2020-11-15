#pragma once

constexpr auto TW			= 14;	// tile width in pixels
constexpr auto TH			= 16;	// tile height in pixels
constexpr auto PIXELDEPTH	= 4;	// ARGB

constexpr auto REGIONMAPSTART		= 0xA000;	// Start of Region Map in main memory. Ends at 0xA8FF
constexpr auto REGIONMAPWIDTH		= 0x30;		// Width of the Region Map (it's also the height since it's square)
constexpr auto RMAP					= 0x6D02;	// Location of RMAP value in main memory. It determines where the player is within the Region Map
constexpr auto VISIBLEORIGINOFFSET	= 0xF8;		// Origin offset to substract from RMAP to where the visible Region Map starts (see "Region Map" in Overworld Map xls)

// For the framebuffer
constexpr auto FRAMEBUFFERWIDTH = 280 * PIXELDEPTH;
constexpr auto FRAMBUFFERHEIGHT = 192 * PIXELDEPTH;
constexpr auto ResMX = 2;	// resolution multiplier (AppleWin multiplies resolution by 2)
constexpr UINT8 FBTILESPERROW = 17;
constexpr UINT8 FBTILESPERCOL = 11;
constexpr auto LEFTMARGIN = 0 * PIXELDEPTH;
constexpr auto TOPMARGIN = 8 * PIXELDEPTH;
constexpr auto RIGHTMARGIN = FRAMEBUFFERWIDTH - LEFTMARGIN - FBTILESPERROW*TW*PIXELDEPTH;
constexpr auto BOTTOMMARGIN = FRAMBUFFERHEIGHT - TOPMARGIN - FBTILESPERCOL*TH*PIXELDEPTH;

// For the patchwork PNG
constexpr auto PNGTILESPERROW	= 16;
constexpr auto PNGTILESPERCOL	= 16;
constexpr auto PNGBYTESPERROW	= PNGTILESPERROW * TW * PIXELDEPTH;


class TilesetCreator
{
public:
	bool isActive = false;
	TilesetCreator()
	{
		pTilesetBuffer = new UINT8[TH * PNGTILESPERCOL * PNGBYTESPERROW];
		isActive = true;
	}
	~TilesetCreator()
	{
		delete pTilesetBuffer;
		pTilesetBuffer = NULL;
	}
	void reset();
	UINT parseTilesInFrameBuffer(UINT8* pFrameBuffer);
	bool insertTileInTilesetBuffer(UINT32 iTileId, UINT32 iTileNumber, UINT8* pFrameBuffer);
	bool saveTilesetPNG(std::string filepath);
private:
	UINT8* pTilesetBuffer;
	UINT8 aKnownTiles[UINT8_MAX] = {};
};

