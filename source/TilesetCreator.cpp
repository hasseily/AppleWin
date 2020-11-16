#include "stdafx.h"
#include "TilesetCreator.h"
#include "Frame.h"
#include "Memory.h"
#include <fstream>

void TilesetCreator::start()
{
    isActive = true;
    iInserted = 0;
    for (UINT8 i = 0; i < UINT8_MAX; i++)
    {
        aKnownTiles[i] = 0;
    }
    MessageBox(g_hFrameWindow, "Starting Tileset Creator!", TEXT("AppleWin Tileset"), MB_OK);
}

void TilesetCreator::stop()
{
    isActive = false;
    std::fstream fsFile("Nox Tileset - Auto.data", std::ios::out | std::ios::binary);
    fsFile.write(pTilesetBuffer, PNGBUFFERSIZE);
    fsFile.close();
    std::string msg("Stopped Tileset Creator!\nPNG is at: Nox Tileset - Auto.png\nNumber of tiles loaded: ");
    msg.append(std::to_string(iInserted));
    MessageBox(g_hFrameWindow, msg.c_str(), TEXT("AppleWin Tileset"), MB_OK);
}

/// <summary>
/// Goes through all the tiles in the framebuffer and parses them, adding them to the PNG patchwork as necessary
/// This method only calculates a tile's unique ID and checks that it hasn't been processed yet.
/// </summary>
/// <param name="pFrameBuffer">The game's framebuffer</param>
/// <returns>Number of tiles inserted</returns>
UINT TilesetCreator::parseTilesInFrameBuffer(const char* pFrameBuffer)
{
    char tmpb[200] = {};
    //sprintf(tmpb, "RMAP is: %x, %x\n", *MemGetMainPtr(RMAP), *MemGetMainPtr(RMAP + 1));
    //OutputDebugString(tmpb);
    // Get the tile ids from the region map slots given the RMAP player position
    UINT32 iPlayerRegionPos = (*MemGetMainPtr(RMAP+1) << 8) + *MemGetMainPtr(RMAP);
    UINT32 iTileIdLocation; // Tile Id location in main memory
    UINT32 iTileId;
    for (UINT8 i = 0; i < FBTILESPERCOL; i++)
    {
        for (UINT8 j = 0; j < FBTILESPERROW; j++)
        {
            iTileIdLocation = (REGIONMAPSTART + iPlayerRegionPos) - VISIBLEORIGINOFFSET + (REGIONMAPWIDTH * i) + j;
            iTileId = *MemGetMainPtr(iTileIdLocation);
            if (aKnownTiles[iTileId] > 0)
                continue;
            if (insertTileInTilesetBuffer(iTileId, (i * FBTILESPERROW) + j, pFrameBuffer))
            {
                iInserted++;
//                sprintf(tmpb, "Inserted from %2d,%2d tile ID: %2x\n", j, i, iTileId);
//                OutputDebugString(tmpb);
            }
        }
    }
    return iInserted;
}

/// <summary>
/// Takes a tile from the framebuffer and inserts it into a patchwork buffer in the position of its tile ID.
/// The tileset buffer is that of a patchwork image of TILESPERROW x COLSPERROW tiles, each TW x TH pixels big
/// The ResMX constant defines the resolution multiplier from the source framebuffer. Resolution is brought back to 1 in the patchwork buffer
/// </summary>
/// <param name="iTileId">The tile id of the tile, which will determine its position in the patchwork</param>
/// <param name="iTileNumber">The tile number within the in-game view of the world. Tile 0 is the top left tile. There are 17 x 11 tiles.</param>
/// <param name="pFrameBuffer">the framebuffer in-game view of the world</param>

bool TilesetCreator::insertTileInTilesetBuffer(UINT32 iTileId, UINT32 iTileNumber, const char* pFrameBuffer)
{
    // Calculate the source 0,0 byte to get the tile from
    UINT8 iFBRow = iTileNumber / FBTILESPERROW;
    UINT8 iFBCol = iTileNumber % FBTILESPERROW;
    UINT32 iFBOriginByte = ((TOPMARGIN + iFBRow * FBTH) * FRAMEBUFFERWIDTH) + ((LEFTMARGIN + iFBCol * FBTW) * sizeof(UINT32));

    // Calculate the destination 0,0 byte to draw the tile onto
    UINT8 iPNGRow = iTileId / PNGTILESPERROW;
    UINT8 iPNGCol = iTileId % PNGTILESPERCOL;
    // Same as above, except that there are no margins
    UINT32 iPNGOriginByte = (iPNGRow * PNGTH * PNGBUFFERWIDTH) + (iPNGCol * PNGTW * sizeof(UINT32));

    // Map every pixel of the source tile onto the destination tile
    // But only parse every other line, and skip every other pixel
    UINT8 b0, b1, b2, b3;   // The individual pixel bytes, low to high
    UINT iFBCurrentByte;
    UINT iPNGCurrentByte;
	for (UINT32 i = 0; i < FBTH; i ++)
	{
        if (i % 2)
            continue;
		for (UINT32 j = 0; j < FBTW; j ++)
		{
            if (j % 2)
                continue;
            iFBCurrentByte = iFBOriginByte + (i * FRAMEBUFFERWIDTH) + (j * sizeof(UINT32));
            b0 = pFrameBuffer[iFBCurrentByte];
            b1 = pFrameBuffer[iFBCurrentByte + 1];
            b2 = pFrameBuffer[iFBCurrentByte + 2];
            b3 = pFrameBuffer[iFBCurrentByte + 3];

            // bgra -> rgba . Force alpha to be opaque because when bgra==ffffff (white), a==00.
            iPNGCurrentByte = iPNGOriginByte + ((i / 2) * PNGBUFFERWIDTH) + ((j / 2) * sizeof(UINT32));
            pTilesetBuffer[iPNGCurrentByte] = b2;
            pTilesetBuffer[iPNGCurrentByte + 1] = b1;
            pTilesetBuffer[iPNGCurrentByte + 2] = b0;
            pTilesetBuffer[iPNGCurrentByte + 3] = (char)0xFF;

            if (b0 + b1 + b2 + b3)
            {
                char tmpb[200] = {};
                sprintf(tmpb, "FB byte is: %2X %2X %2X %2X\n", b0, b1, b2, b3);
                OutputDebugString(tmpb);
            }
        }
	}

	aKnownTiles[iTileId] = 1;
	return true;
}
