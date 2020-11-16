#include "stdafx.h"
#include "TilesetCreator.h"
#include "Frame.h"
#include "Memory.h"
#include <png.h>
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
    saveTilesetPNG(std::string("Nox Tileset - Auto.png"));
    //std::fstream fsFile("Nox Tileset - Auto.data", std::ios::out | std::ios::binary);
    //fsFile.write((const char*)pTilesetBuffer, PNGBUFFERSIZE * sizeof(UINT32));
    //fsFile.close();
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
UINT TilesetCreator::parseTilesInFrameBuffer(UINT32* pFrameBuffer)
{

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

bool TilesetCreator::insertTileInTilesetBuffer(UINT32 iTileId, UINT32 iTileNumber, UINT32* pFrameBuffer)
{
    // Calculate the source 0,0 pixel to get the tile from
    UINT8 iFBRow = iTileNumber / FBTILESPERROW;
    UINT8 iFBCol = iTileNumber % FBTILESPERROW;
    UINT32 iFBOriginPixel = ((TOPMARGIN + iFBRow * FBTH) * FRAMEBUFFERWIDTH) + (LEFTMARGIN + iFBCol * FBTW);

    // Calculate the destination 0,0 pixel to draw the tile onto
    UINT8 iPNGRow = iTileId / PNGTILESPERROW;
    UINT8 iPNGCol = iTileId % PNGTILESPERCOL;
    // Same as above, except that there are no margins
    UINT32 iPNGOriginPixel = (iPNGRow * PNGTH * PNGBUFFERWIDTH) + (iPNGCol * PNGTW);

    // Map every pixel of the source tile onto the destination tile
    // But only parse every other line, and skip every other pixel
    UINT8 b0, b1, b2, b3;   // The individual pixel bytes, low to high
    UINT32 fbPixel;
	for (UINT32 i = 0; i < FBTH; i ++)
	{
        if (i % 2)
            continue;
		for (UINT32 j = 0; j < FBTW; j ++)
		{
            if (j % 2)
                continue;
            fbPixel = pFrameBuffer[iFBOriginPixel + (i * FRAMEBUFFERWIDTH) + j];
            // bgra?
            b0 = UINT8(fbPixel);
            b1 = UINT8(fbPixel >> 8);
            b2 = UINT8(fbPixel >> 16);
            b3 = UINT8(fbPixel >> 24);
            // Convert to argb
            fbPixel = b1        | ((UINT32)b2   << 8);
            fbPixel = fbPixel   | ((UINT32)b3   << 16);
            fbPixel = fbPixel   | (0xff         << 24);

            pTilesetBuffer[iPNGOriginPixel + ((i/2)* PNGBUFFERWIDTH) + (j/2)] = fbPixel;
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
    int width = PNGBUFFERWIDTH;
    int height = PNGBUFFERHEIGHT;
    int bytewidth = width * sizeof(UINT32);
    png_byte color_type = PNG_COLOR_TYPE_RGB_ALPHA;    // ARGB
    png_byte bit_depth = 8;

    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);

    /* fill row pointers */
    for (y = 0; y < height; y++)
    {
        row_pointers[y] = (png_bytep)pTilesetBuffer + (bytewidth * y);
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
    free(row_pointers);

    fclose(fp);
    isActive = false;
    return true;
}