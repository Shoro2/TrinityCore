#include "Define.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "Unit.h"
#include "InstanceScript.h"
#include "mythic_dungeons.h"
#include "WorldDatabase.h"
#include "DatabaseEnv.h"
#include "Player.h"
#include <Chat.h>


class mythic_dungeon_zone : public ZoneScript
{
public:
    mythic_dungeon_zone() : ZoneScript() {}

    virtual void OnCreatureUpdate(Creature* creature) {
        Map* myMap = creature->GetMap();
        if (myMap->IsDungeon()) {
            int playerCount = myMap->GetPlayersCountExceptGMs();
            if (playerCount > 0) {
                Map::PlayerList const& players = myMap->GetPlayers();
                Player* player = players.getFirst()->GetSource();
                if (player->HasItemCount(100000, 5, false)) ApplyMythicBuffs(5, creature);
                else if (player->HasItemCount(100000, 4, false)) ApplyMythicBuffs(4, creature);
                else if (player->HasItemCount(100000, 3, false)) ApplyMythicBuffs(3, creature);
                else if (player->HasItemCount(100000, 2, false)) ApplyMythicBuffs(2, creature);
                else if (player->HasItemCount(100000, 1, false)) ApplyMythicBuffs(1, creature);
            }
        }
    }
    void ApplyMythicBuffs(uint32 level, Creature* creature) {
        switch (level) {
        case 1:
            // 5% stats
            if (!creature->HasAura(AURA_5)) creature->AddAura(AURA_5, creature);
            break;
        case 2:
            // 10% stats + 1st suffix
            if (!creature->HasAura(AURA_10)) creature->AddAura(AURA_10, creature);
            if (!creature->HasAura(SPELL_EXTRA_1)) creature->AddAura(SPELL_EXTRA_1, creature);
            break;
        case 3:
            // 15% stats + 1st suffix
            if (!creature->HasAura(AURA_15)) creature->AddAura(AURA_15, creature);
            if (!creature->HasAura(SPELL_EXTRA_1)) creature->AddAura(SPELL_EXTRA_1, creature);
            break;
        case 4:
            // 20% stats + 2 suffixes
            if (!creature->HasAura(AURA_20)) creature->AddAura(AURA_20, creature);
            if (!creature->HasAura(SPELL_EXTRA_1)) creature->AddAura(SPELL_EXTRA_1, creature);
            if (!creature->HasAura(SPELL_EXTRA_2)) creature->AddAura(SPELL_EXTRA_2, creature);
            break;
        case 5:
            // 25% stats + 3 suffixes
            if (!creature->HasAura(AURA_25)) creature->AddAura(AURA_25, creature);
            if (!creature->HasAura(SPELL_EXTRA_1)) creature->AddAura(SPELL_EXTRA_1, creature);
            if (!creature->HasAura(SPELL_EXTRA_2)) creature->AddAura(SPELL_EXTRA_2, creature);
            if (!creature->IsDungeonBoss()) creature->ModifyHealth(creature->GetMaxHealth() * 2);
            else if (creature->IsDungeonBoss()) creature->ModifyHealth(creature->GetMaxHealth() * 1.5);
            break;
        }

    }
};

class mythic_dungeon_player : public PlayerScript {
public:
    mythic_dungeon_player() : PlayerScript("mythic_dungeon") {
    }


    virtual void OnLogin(Player* player, bool firstLogin)
    {
        ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Mythic Dungeon |rmodule.");
    }

    virtual void OnMapChanged(Player* player) {
        Map* newMap = player->GetMap();
        int32 keyCount = player->GetItemCount(100000, false);
        if (newMap->IsDungeon() && keyCount >= 1) {

            switch (keyCount) {
            case 1:
                ChatHandler(player->GetSession()).SendSysMessage("You have entered a |cff4CFF00Mythic Dungeon|r. All mobs stats are boosted by 5%");
                for (auto x : player->GetMap()->GetCreatureBySpawnIdStore()) {
                    ApplyMythicBuffs(1, x.second);
                }
                break;
            case 2:
                ChatHandler(player->GetSession()).SendSysMessage("You have entered a |cff4CFF00Mythic Dungeon|r. All mobs stats are boosted by 10%");
                for (auto x : player->GetMap()->GetCreatureBySpawnIdStore()) {
                    ApplyMythicBuffs(2, x.second);
                }
                break;
            case 3:
                ChatHandler(player->GetSession()).SendSysMessage("You have entered a |cff4CFF00Mythic Dungeon|r. All mobs stats are boosted by 15%");
                for (auto x : player->GetMap()->GetCreatureBySpawnIdStore()) {
                    ApplyMythicBuffs(3, x.second);
                }
                break;
            case 4:
                ChatHandler(player->GetSession()).SendSysMessage("You have entered a |cff4CFF00Mythic Dungeon|r. All mobs stats are boosted by 20%");
                for (auto x : player->GetMap()->GetCreatureBySpawnIdStore()) {
                    ApplyMythicBuffs(4, x.second);
                }
                break;
            case 5:
                ChatHandler(player->GetSession()).SendSysMessage("You have entered a |cff4CFF00Mythic Dungeon|r. All mobs stats are boosted by 25%");
                for (auto x : player->GetMap()->GetCreatureBySpawnIdStore()) {
                    ApplyMythicBuffs(5, x.second);
                }
                break;
            default:
                ChatHandler(player->GetSession()).SendSysMessage("You have entered a |cff4CFF00Mythic Dungeon |r.");
                break;
            }
            
        }
    }
    void ApplyMythicBuffs(uint32 level, Creature* creature) {
        switch (level) {
        case 1:
            // 5% stats
            if (!creature->HasAura(AURA_5)) creature->AddAura(AURA_5, creature);
            break;
        case 2:
            // 10% stats + 1st suffix
            if (!creature->HasAura(AURA_10)) creature->AddAura(AURA_10, creature);
            if (!creature->HasAura(SPELL_EXTRA_1)) creature->AddAura(SPELL_EXTRA_1, creature);
            break;
        case 3:
            // 15% stats + 1st suffix
            if (!creature->HasAura(AURA_15)) creature->AddAura(AURA_15, creature);
            if (!creature->HasAura(SPELL_EXTRA_1)) creature->AddAura(SPELL_EXTRA_1, creature);
            break;
        case 4:
            // 20% stats + 2 suffixes
            if (!creature->HasAura(AURA_20)) creature->AddAura(AURA_20, creature);
            if (!creature->HasAura(SPELL_EXTRA_1)) creature->AddAura(SPELL_EXTRA_1, creature);
            if (!creature->HasAura(SPELL_EXTRA_2)) creature->AddAura(SPELL_EXTRA_2, creature);
            break;
        case 5:
            // 25% stats + 3 suffixes
            if (!creature->HasAura(AURA_25)) creature->AddAura(AURA_25, creature);
            if (!creature->HasAura(SPELL_EXTRA_1)) creature->AddAura(SPELL_EXTRA_1, creature);
            if (!creature->HasAura(SPELL_EXTRA_2)) creature->AddAura(SPELL_EXTRA_2, creature);
            if (!creature->IsDungeonBoss()) creature->ModifyHealth(creature->GetMaxHealth() * 2);
            else if (creature->IsDungeonBoss()) creature->ModifyHealth(creature->GetMaxHealth() * 1.5);
            break;
        }

    }
};



void AddSC_mythic_dungeons()
{
    new mythic_dungeon_zone();
    new mythic_dungeon_player();
}
