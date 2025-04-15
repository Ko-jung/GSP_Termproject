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

		for (int j = 0; j < Width; j++)
	{
	for (int i = 0; i < Height; i++)
		{
			COLORREF PixelColor = MapImage.GetPixel(i, j);
			int Red = GetRValue(PixelColor);
			int Green = GetGValue(PixelColor);
			int Blue = GetBValue(PixelColor);

			if (Red == 0 && Green == 0 && Blue == 0)
			{
				WorldMap[j][i] = (WORD)MAP_INFO::WALLS_BLOCK;
			}
			else if (Red == 255 && Green == 0 && Blue == 0)
			{

			}
			else if (Red == 0 && Green == 255 && Blue == 0)
			{
				WorldMap[j][i] = (WORD)MAP_INFO::GROUND_EMPTY;
				CanSpawnPos.emplace_back(std::make_pair(i, j));
			}
			else if (Red == 0 && Green == 0 && Blue == 255)
			{
				WorldMap[j][i] = (WORD)MAP_INFO::GROUND_EMPTY;
				CanSpawnPos.emplace_back(std::make_pair(i, j));
			}
		}
	}

	std::cout << "World Loading Complete" << std::endl;
}

std::pair<WORD, WORD> MapMgr::GetRandomCanSpawnPos()
{
	int Index = CanSpawnPos.size() - 1;
	return CanSpawnPos[rand() % Index];
}
