/* Copyright
Author : Callmephil
Version : 3.3.5 / 4.3.4
Dynamic Resurrection is a simple script that add a "Resurrection Waypoint" near the latest boss killed in dungeon or raid. for faster Resurrection.
Update : 9/9/2018
*/

#include "DynamicRes.h"

class Dynamic_Resurrections : public PlayerScript
{
public:
    Dynamic_Resurrections() : PlayerScript("Dynamic_Resurrections") {}

    void OnCreatureKill(Player* player, Creature* boss) override
    {
        if (sDynRes->IsInDungeonOrRaid(player) && (boss->isWorldBoss() || boss->IsDungeonBoss()))
            player->SummonCreature(CREATURE_ENTRY, boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN);
    }
};

void AddSC_Dynamic_Resurrections()
{
    new Dynamic_Resurrections();
}
