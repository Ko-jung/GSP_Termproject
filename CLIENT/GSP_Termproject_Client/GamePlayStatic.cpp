#include "GamePlayStatic.h"

#include "Manager/GameMgr.h"
#include "Actor.h"

POSITION GamePlayStatic::GetOwnActorPosition()
{
	GameMgr* inst = GameMgr::Instance();
	return inst->GetOwnActor()->GetLocation();
}

float GamePlayStatic::GetFPS()
{
	GameMgr* inst = GameMgr::Instance();
	return inst->GetFPS();
}
