#pragma once


constexpr auto PIXELDEPTH	= 4;	// ARGB

constexpr auto REGIONMAPSTART		= 0xA000;	// Start of Region Map in main memory. Ends at 0xA8FF
constexpr auto REGIONMAPWIDTH		= 0x30;		// Width of the Region Map (it's also the height since it's square)
constexpr auto RMAP					= 0x6D02;	// Location of RMAP value in main memory. It determines where the player is within the Region Map
constexpr auto VISIBLEORIGINOFFSET	= 0xF8;		// Origin offset to substract from RMAP to where the visible Region Map starts (see "Region Map" in Overworld Map xls)

// For the framebuffer
constexpr auto FBTW = 28;	// FB tile width in pixels
constexpr auto FBTH = 32;	// FB tile height in pixels
constexpr UINT8 FBTILESPERROW = 17;
constexpr UINT8 FBTILESPERCOL = 11;
constexpr auto FRAMEBUFFERWIDTH = 600;
constexpr auto FRAMEBUFFERHEIGHT = 420;
constexpr auto FRAMEBUFFERSIZE = FRAMEBUFFERWIDTH * FRAMEBUFFERHEIGHT;
constexpr auto ResMX = 2;	// resolution multiplier (AppleWin multiplies resolution by 2)
constexpr auto LEFTMARGIN = 20;
constexpr auto TOPMARGIN = 34;
constexpr auto RIGHTMARGIN = FRAMEBUFFERWIDTH - LEFTMARGIN - FBTILESPERROW*FBTW;
constexpr auto BOTTOMMARGIN = FRAMEBUFFERHEIGHT - TOPMARGIN - FBTILESPERCOL*FBTH;

// For the patchwork PNG
constexpr auto PNGTW = 14;
constexpr auto PNGTH = 16;
constexpr auto PNGTILESPERROW = 16;
constexpr auto PNGTILESPERCOL	= 16;
constexpr auto PNGBUFFERWIDTH = PNGTW * PNGTILESPERROW;
constexpr auto PNGBUFFERHEIGHT = PNGTH * PNGTILESPERCOL;
constexpr auto PNGBUFFERSIZE = PNGBUFFERWIDTH * PNGBUFFERHEIGHT;


class TilesetCreator
{
public:
	bool isActive = false;
	UINT32 iInserted = 0;
	TilesetCreator()
	{
		pTilesetBuffer = new UINT32[PNGBUFFERSIZE];
		ZeroMemory(pTilesetBuffer, PNGBUFFERSIZE*sizeof(UINT32));
	}
	~TilesetCreator()
	{
		delete pTilesetBuffer;
		pTilesetBuffer = NULL;
	}
	void start();
	void stop();
	UINT parseTilesInFrameBuffer(UINT32* pFrameBuffer);
	bool insertTileInTilesetBuffer(UINT32 iTileId, UINT32 iTileNumber, UINT32* pFrameBuffer);
	bool saveTilesetPNG(std::string filepath);
private:
	UINT32* pTilesetBuffer;
	UINT8 aKnownTiles[UINT8_MAX] = {0};
};


