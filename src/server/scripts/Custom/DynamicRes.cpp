/* Copyright
Author : Callmephil
Version : 3.3.5 / 4.3.4
Dynamic Resurrection is a simple script that add a "Resurrection Waypoint" near the latest boss killed in dungeon or raid. for faster Resurrection.
Source: https://gist.github.com/callmephil/6bb933fdf772119c164b6f05b6369287
Updated : 9/9/2018
*/

#include "DynamicRes.h"

bool DynamicRes::IsInDungeonOrRaid(Player* player)
{
    if (sMapStore.LookupEntry(player->GetMapId())->Instanceable())
        return true; // boolean need to return to a value
    return false;
}

bool DynamicRes::CheckForSpawnPoint(Player* player)
{
    // Find Nearest Creature And Teleport.
    if (Creature* creature = player->FindNearestCreature(CREATURE_ENTRY, DISTANCE_CHECK_RANGE))
        return true;
    return false;
}

void DynamicRes::DynamicResurrection(Player* player)
{
    // Find Nearest Creature And Teleport.
    if (Creature* creature = player->FindNearestCreature(CREATURE_ENTRY, DISTANCE_CHECK_RANGE)) {
        player->TeleportTo(player->GetMapId(), creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), 1);
    }
    // Revive Player with 70 %
    player->ResurrectPlayer(0.7f);
}
