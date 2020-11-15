#include "stdafx.h"
#include "TilesetCreator.h"
#include "Frame.h"
#include <png.h>

TilesetCreator::TilesetCreator()
{
	pTilesetBuffer = new UINT32[TW * TH * TILESPERROW * TILESPERCOL];
}


TilesetCreator::~TilesetCreator()
{
	delete pTilesetBuffer;
	pTilesetBuffer = NULL;
}


/// <summary>
/// Takes a tile from the framebuffer and inserts it into a patchwork buffer in the position of its tile ID.
/// The tileset buffer is that of a patchwork image of TILESPERROW x COLSPERROW tiles, each TW x TH pixels big
/// The ResMX constant defines the resolution multiplier from the source framebuffer. Resolution is brought back to 1 in the patchwork buffer
/// </summary>
/// <param name="iTileId">The tile id of the tile, which will determine its position in the patchwork</param>
/// <param name="pFramePixelOrigin">0,0 origin of the tile within the framebuffer in-game view of the world</param>
/// <param name="iFramePixelWidth">Width of the framebuffer in pixels</param>

bool TilesetCreator::insertTileInTilesetBuffer(UINT8 iTileId, UINT32* pFramePixelOrigin, UINT32 iFramePixelWidth)
{
	if (aKnownTiles[iTileId] > 0)
		return false;
	UINT8 iRow = iTileId >> 4;		// high 4 bits (0xC4 -> 0xC) - Row position in the patchwork
	UINT8 iCol = iTileId & 0b1111;	// low 4 bits  (0xC4 -> 0x4) - Column position in the patchwork
	
	UINT32 iOriginPixel = ((iRow - 1) * TH * PIXELSPERROW) + (iCol * TW);

	for (size_t i = 0; i < TH*ResMX; i += ResMX)
	{
		for (size_t j = 0; j < TW*ResMX; j += ResMX)
		{
			pTilesetBuffer[iOriginPixel + i * PIXELSPERROW/ResMX + j/ResMX] = pFramePixelOrigin[i * iFramePixelWidth + j];
		}
	}

	aKnownTiles[iTileId] = 0;
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
    int width = TILESPERROW * TW;
    int height = TILESPERCOL * TH;
    int bytewidth = width * sizeof(UINT32);
    png_byte color_type = 6;    // ARGB
    png_byte bit_depth = 8;

    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
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
        MessageBox(g_hFrameWindow, "[write_png_file] File %s could not be opened for writing", TEXT("AppleWin Error"), MB_OK);

    /* initialize stuff */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
        MessageBox(g_hFrameWindow, "[write_png_file] png_create_write_struct failed", TEXT("AppleWin Error"), MB_OK);

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        MessageBox(g_hFrameWindow, "[write_png_file] png_create_info_struct failed", TEXT("AppleWin Error"), MB_OK);

    if (setjmp(png_jmpbuf(png_ptr)))
        MessageBox(g_hFrameWindow, "[write_png_file] Error during init_io", TEXT("AppleWin Error"), MB_OK);

    png_init_io(png_ptr, fp);


    /* write header */
    if (setjmp(png_jmpbuf(png_ptr)))
        MessageBox(g_hFrameWindow, "[write_png_file] Error during writing header", TEXT("AppleWin Error"), MB_OK);

    png_set_IHDR(png_ptr, info_ptr, width, height,
        bit_depth, color_type, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);


    /* write bytes */
    if (setjmp(png_jmpbuf(png_ptr)))
        MessageBox(g_hFrameWindow, "[write_png_file] Error during writing bytes", TEXT("AppleWin Error"), MB_OK);

    png_write_image(png_ptr, row_pointers);


    /* end write */
    if (setjmp(png_jmpbuf(png_ptr)))
        MessageBox(g_hFrameWindow, "[write_png_file] Error during end of write", TEXT("AppleWin Error"), MB_OK);

    png_write_end(png_ptr, NULL);

    /* cleanup heap allocation */
    for (y = 0; y < height; y++)
        free(row_pointers[y]);
    free(row_pointers);

    fclose(fp);
}