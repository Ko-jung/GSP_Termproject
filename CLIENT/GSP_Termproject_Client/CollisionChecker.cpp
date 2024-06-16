#include "CollisionChecker.h"

bool CollisionChecker::CollisionCheck(const RECT& a, const RECT& b)
{
    if (a.left > b.right)  return false;
    if (a.top > b.bottom) return false;
    if (a.right < b.left)   return false;
    if (a.bottom < b.top)    return false;

    return true;
}
