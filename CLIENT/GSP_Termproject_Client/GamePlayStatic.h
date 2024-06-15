#pragma once

#include "framework.h"
#include "Define.h"
#include "../../Common/GameUtil.h"

class GamePlayStatic
{
public:
	SINGTON(GamePlayStatic);

public:
	static POSITION GetOwnActorPosition();

private:
	class GameMgr* GameManager;
};

