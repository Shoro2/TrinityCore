#include "Define.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "Unit.h"
#include "InstanceScript.h"
#include "mythic_dungeons.h"
#include "WorldDatabase.h"
#include "DatabaseEnv.h"
#include <Chat.h>
// .. more includes


class mythic_dungeons : public ZoneScript
{
public:
    virtual void OnCreatureCreate(Creature* creature) {
        Map* myMap = creature->GetMap();
        int playerCount = myMap->GetPlayersCountExceptGMs();
        int32 myDiffi;

        if (playerCount > 0) {
            Map::PlayerList const& players = myMap->GetPlayers();
            Player* player = players.begin()->GetSource();
            
            if(player->HasItemCount(12345, 5, true)) {
                
            }
            else if (player->HasItemCount(12345, 4, true)) {

            }
            else if (player->HasItemCount(12345, 3, true)) {

            }
            else if (player->HasItemCount(12345, 2, true)) {

            }
            else if (player->HasItemCount(12345, 1, true)) {

            }
        }
        

    }

    virtual void OnPlayerEnter(Player* player) {
        //start run
        ChatHandler(player->GetSession()).SendSysMessage("Mythic Dungeon loading...");
    }


    virtual void OnPlayerLeave(Player* player) {
        //finish run

    }
};

void AddSC_mythic_dungeons()
{
    new mythic_dungeons();
}
