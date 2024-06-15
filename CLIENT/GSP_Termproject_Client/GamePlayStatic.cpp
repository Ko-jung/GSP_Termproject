#include "GamePlayStatic.h"

#include "GameMgr.h"
#include "Actor.h"

POSITION GamePlayStatic::GetOwnActorPosition()
{
	GameMgr* inst = GameMgr::Instance();
	return inst->GetOwnActor()->GetLocation();
}
