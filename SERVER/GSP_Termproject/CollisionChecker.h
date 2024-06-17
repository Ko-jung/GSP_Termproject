#pragma once

#include "Define.h"
#include "stdafx.h"
#include "../../Common/GameUtil.h"

class CollisionChecker
{
public:
	SINGLETON(CollisionChecker);

	static bool CollisionCheck(const RECT& a, const RECT& b);
	static bool CollisionCheck(const RectF& a, const RectF& b);
};

