#pragma once

#include "Define.h"
#include "stdafx.h"

class CollisionChecker
{
public:
	SINGLETON(CollisionChecker);

	static bool CollisionCheck(const RECT& a, const RECT& b);
};

