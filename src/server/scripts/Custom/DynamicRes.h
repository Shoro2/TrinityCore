/* Copyright
Author : Callmephil
Version : 3.3.5 / 4.3.4
Dynamic Resurrection is a simple script that add a "Resurrection Waypoint" near the latest boss killed in dungeon or raid. for faster Resurrection.
UPDATE : 9/9/2018
*/

#ifndef H_DYNAMIC_RES_
#define H_DYNAMIC_RES_

#include "Player.h"
#include "DBCStores.h"

class Player;

enum WAYPOINT_CREATURE
{
    CREATURE_ENTRY = 1, // change this as you wishes
    DISTANCE_CHECK_RANGE = 1000, // change this (in yards)
    SPAWN_TIMER_TWO_HOURS = 1200000, // change this (in miliseconds)
};

class TC_GAME_API DynamicRes
{
public:
    static DynamicRes* instance()
    {
        static DynamicRes instance;
        return &instance;
    }

    bool IsInDungeonOrRaid(Player* player);
    bool CheckForSpawnPoint(Player* player);
    void DynamicResurrection(Player* player);
};

#define sDynRes DynamicRes::instance()

#endif