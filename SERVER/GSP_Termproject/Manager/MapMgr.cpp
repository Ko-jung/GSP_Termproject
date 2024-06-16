#include "MapMgr.h"

#include "../../../Common/EnumDef.h"
#include <iostream>

void MapMgr::Init()
{
	std::cout << "World Loading ... " << std::endl;

	CImage MapImage;
	MapImage.Load(TEXT("../../Common/Image/MiniWorldMapTile.png"));

	if (MapImage.IsNull())
	{
		std::cout << "../../Common/Image/MiniWorldMapTile.png is Cant find" << std::endl;
		return;
	}

	int Width = MapImage.GetWidth();
	int Height = MapImage.GetHeight();

	for (int i = 0; i < Height; i++)
	{
		for (int j = 0; j < Width; j++)
		{
			COLORREF PixelColor = MapImage.GetPixel(i, j);
			int Red = GetRValue(PixelColor);
			int Green = GetGValue(PixelColor);
			int Blue = GetBValue(PixelColor);

			if (Red == 0 && Green == 0 && Blue == 0)
			{
				WorldMap[i][j] = (WORD)MAP_INFO::WALLS_BLOCK;
			}
			else if (Red == 255 && Green == 0 && Blue == 0)
			{

			}
			else if (Red == 0 && Green == 255 && Blue == 0)
			{
				WorldMap[i][j] = (WORD)MAP_INFO::GROUND_EMPTY;
			}
			else if (Red == 0 && Green == 0 && Blue == 255)
			{
				WorldMap[i][j] = (WORD)MAP_INFO::GROUND_EMPTY;
			}
		}
	}

	std::cout << "World Loading Complete" << std::endl;
}
