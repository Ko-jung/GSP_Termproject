#include "MapMgr.h"

void MapMgr::Init()
{
	WorldImageTile.Load(TEXT("Image/Map/MapImage.png"));

	CImage WorldImage;
	WorldImage.Load(TEXT("../../Common/Image/MiniWorldMapTile.png"));
	//WorldImageTile.Load(TEXT("../../Common/Image/WorldMapTile.png"));

	int Width = WorldImage.GetWidth();
	int Height = WorldImage.GetHeight();

		for (int j = 0; j < Width; j++)
	{
	for (int i = 0; i < Height; i++)
		{
			COLORREF PixelColor = WorldImage.GetPixel(i, j);
			int Red = GetRValue(PixelColor);
			int Green = GetGValue(PixelColor);
			int Blue = GetBValue(PixelColor);

			if (Red == 0 && Green == 0 && Blue == 0)
			{
				WorldMap[j][i] = (BYTE)MAP_INFO::WALLS_BLOCK;
			}
			else if (Red == 255 && Green == 0 && Blue == 0)
			{

			}
			else if (Red == 0 && Green == 255 && Blue == 0)
			{
				WorldMap[j][i] = (BYTE)MAP_INFO::GROUND_EMPTY;
			}
			else if (Red == 0 && Green == 0 && Blue == 255)
			{
				WorldMap[j][i] = (BYTE)MAP_INFO::GROUND_EMPTY;
			}
		}
	}
}

void MapMgr::Draw(HDC& memdc, const POSITION& pos)
{
	for (int y = 0; y < BOARDSIZE + 2; y++)
	{
		for (int x = 0; x < BOARDSIZE + 2; x++)
		{
			int DrawX = (int)pos.X + x - BOARDSIZE / 2 - 1;
			int DrawY = (int)pos.Y + y - BOARDSIZE / 2 - 1;

			if (DrawX < 0 || DrawX >= W_WIDTH || DrawY < 0 || DrawY >= W_WIDTH) continue;

			int BoardDrawSize = WINWIDTH / BOARDSIZE;
			int DrawWindowX = (x - (pos.X - (int)pos.X) - 1) * BoardDrawSize;
			int DrawWindowY = (y - (pos.Y - (int)pos.Y) - 1) * BoardDrawSize;
			// RECT TileDest{ DrawWindowX - BoardDrawSize / 2, DrawWindowY - BoardDrawSize / 2,
			// 	DrawWindowX + 1 + BoardDrawSize / 2, DrawWindowY + 1 + BoardDrawSize / 2 };

			RECT TileDest{ DrawWindowX , DrawWindowY,
				 	DrawWindowX + BoardDrawSize, DrawWindowY + BoardDrawSize };
			switch (WorldMap[DrawY][DrawX])
			{
			case (BYTE)MAP_INFO::WALLS_BLOCK:
			{
				// 109, 188
				// 132, 211
				RECT Tile{ 109,188,132,211 };
				WorldImageTile.Draw(memdc, TileDest, Tile);
				break;
			}
			case (BYTE)MAP_INFO::GROUND_EMPTY:
			{
				// 334, 188
				// 357, 211
				RECT Tile{ 334,188,357,211 };
				WorldImageTile.Draw(memdc, TileDest, Tile);
				break;
			}
			default:
				break;
			}
		}
	}
}
