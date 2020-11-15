#include "stdafx.h"
#include "TilesetCreator.h"
#include "Frame.h"
#include "Memory.h"
#include <png.h>


void TilesetCreator::reset()
{
    isActive = true;
}


/// <summary>
/// Goes through all the tiles in the framebuffer and parses them, adding them to the PNG patchwork as necessary
/// </summary>
/// <param name="pFrameBuffer">The game's framebuffer</param>
/// <returns>Number of tiles inserted</returns>
UINT TilesetCreator::parseTilesInFrameBuffer(UINT8* pFrameBuffer)
{
    // Get the tile ids from the region map slots given the RMAP player position
    UINT32 iTileIdLocation; // Tile Id location in main memory
    UINT32 iTileId;
    UINT32 iInserted = 0;
    for (UINT8 i = 0; i < FBTILESPERCOL; i++)
    {
        for (UINT8 j = 0; j < FBTILESPERROW; j++)
        {
            iTileIdLocation = REGIONMAPSTART + (RMAP - VISIBLEORIGINOFFSET) + (REGIONMAPWIDTH * i) + j;
            iTileId = *MemGetMainPtr(iTileIdLocation);
            if (aKnownTiles[iTileId] > 0)
                continue;
            if (insertTileInTilesetBuffer(iTileId, (i * FBTILESPERROW) + j, pFrameBuffer))
                iInserted++;
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

bool TilesetCreator::insertTileInTilesetBuffer(UINT32 iTileId, UINT32 iTileNumber, UINT8* pFrameBuffer)
{
    // Calculate the source 0,0 pixel to get the tile from
    UINT8 iFBRow = iTileNumber / FBTILESPERROW;
    UINT8 iFBCol = iTileNumber % FBTILESPERROW;
    UINT32 iFBOriginPixel = ((TOPMARGIN + iFBRow * TH * FRAMEBUFFERWIDTH) + (LEFTMARGIN + iFBCol * TW * PIXELDEPTH)) * ResMX;

    // Calculate the destination 0,0 pixel to draw the tile onto
    UINT8 iPNGRow = iTileId / PNGTILESPERROW;
    UINT8 iPNGCol = iTileId % PNGTILESPERCOL;
    // This is exactly the same code as for iFBOriginPixel except that margins are 0 and ResMX is 1
    UINT32 iPNGOriginPixel = (iPNGRow * TH * PNGBYTESPERROW) + (iPNGCol * TW * PIXELDEPTH);

    // Map every pixel of the source tile onto the destination tile
	for (UINT32 i = 0; i < TH; i ++)
	{
		for (UINT32 j = 0; j < (TW); j ++)
		{
            pTilesetBuffer[iPNGOriginPixel + (i* PNGBYTESPERROW) + j] = pFrameBuffer[(iFBOriginPixel + (i*FRAMEBUFFERWIDTH) + j) * ResMX];
        }
	}

	aKnownTiles[iTileId] = 1;
	return true;
}

/// <summary>
/// Saves a PNG file of the tileset.
/// </summary>
/// <param name="filepath"></param>
/// <returns></returns>
bool TilesetCreator::saveTilesetPNG(std::string filepath)
{
    int y;
    int width = PNGTILESPERROW * TW;
    int height = PNGTILESPERCOL * TH;
    int bytewidth = width * sizeof(UINT32);
    png_byte color_type = 6;    // ARGB
    png_byte bit_depth = 8;

    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);

    /* fill row pointers */
    for (y = 0; y < height; y++)
    {
        row_pointers[y] = (png_bytep)malloc(sizeof(UINT32) * width);
        memcpy(row_pointers[y], pTilesetBuffer + (bytewidth * y), bytewidth);
    }

    /* create file */
    FILE* fp = fopen(filepath.c_str(), "wb");
    if (!fp)
    {
        MessageBox(g_hFrameWindow, "[write_png_file] File %s could not be opened for writing", TEXT("AppleWin Error"), MB_OK);
        return false;
    }

    /* initialize stuff */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
    {
        MessageBox(g_hFrameWindow, "[write_png_file] png_create_write_struct failed", TEXT("AppleWin Error"), MB_OK);
        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        MessageBox(g_hFrameWindow, "[write_png_file] png_create_info_struct failed", TEXT("AppleWin Error"), MB_OK);
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        MessageBox(g_hFrameWindow, "[write_png_file] Error during init_io", TEXT("AppleWin Error"), MB_OK);
        return false;
    }

    png_init_io(png_ptr, fp);


    /* write header */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        MessageBox(g_hFrameWindow, "[write_png_file] Error during writing header", TEXT("AppleWin Error"), MB_OK);
        return false;
    }

    png_set_IHDR(png_ptr, info_ptr, width, height,
        bit_depth, color_type, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);


    /* write bytes */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        MessageBox(g_hFrameWindow, "[write_png_file] Error during writing bytes", TEXT("AppleWin Error"), MB_OK);
        return false;
    }

    png_write_image(png_ptr, row_pointers);


    /* end write */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        MessageBox(g_hFrameWindow, "[write_png_file] Error during end of write", TEXT("AppleWin Error"), MB_OK);
        return false;
    }

    png_write_end(png_ptr, NULL);

    /* cleanup heap allocation */
    for (y = 0; y < height; y++)
        free(row_pointers[y]);
    free(row_pointers);

    fclose(fp);
    isActive = false;
    return true;
}