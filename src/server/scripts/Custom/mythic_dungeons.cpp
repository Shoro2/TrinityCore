#include "Define.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "Unit.h"
#include "mythic_dungeons.h"
#include "Player.h"
#include <Chat.h>
#include "spell.h"
#include "Group.h"
#include "item.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "LootItemStorage.h"
#include "LootMgr.h"
#include "WorldDatabase.h"
#include "DatabaseEnv.h"
/*
* todo: refresh buff on creatuture evade/reset
* */

class mythic_dungeon_unit : public UnitScript {
public:
    mythic_dungeon_unit() : UnitScript("mythic_dungeon_unit") {

    }

    virtual void OnEvadeEnter(Unit* creature) {
        Map* myMap = creature->GetMap();
        if (myMap->IsDungeon()) {
            uint32 playerCount = myMap->GetPlayersCountExceptGMs();
            if (playerCount > 0) {
                Map::PlayerList const& players = myMap->GetPlayers();
                Player* myPlayer = players.getFirst()->GetSource();
                Group* grp = myPlayer->GetGroup();
                ChatHandler(myPlayer->GetSession()).SendSysMessage("Unit OnEvadeEnter()");
            }
        }
    }
};

class mythic_dungeon_creature : public CreatureScript {
public:
    mythic_dungeon_creature() : CreatureScript("mythic_dungeon_creature") {

    }

    virtual void OnEvadeEnter(Creature* creature) {
        Map* myMap = creature->GetMap();
        if (myMap->IsDungeon()) {
            uint32 playerCount = myMap->GetPlayersCountExceptGMs();
            if (playerCount > 0) {
                Map::PlayerList const& players = myMap->GetPlayers();
                Player* myPlayer = players.getFirst()->GetSource();
                Group* grp = myPlayer->GetGroup();
                ChatHandler(myPlayer->GetSession()).SendSysMessage("Creature OnEvadeEnter()");
            }
        }
    }
};





class mythic_dungeon_player : public PlayerScript {
public:
    mythic_dungeon_player() : PlayerScript("mythic_dungeon_player") {
    }

    virtual void OnLogin(Player* player, bool firstLogin)
    {
        ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Mythic Dungeon |rmodule.");
    }

    virtual void OnPlayerUpdateZone(Player* player, uint32 newZone, uint32 newArea) {

    }

    virtual void OnCreatureKill(Player* killer, Creature* killed) {
        // add dungeon mark if killed has the mythic dungeon buff
        if (killed->HasAura(500100)) {
            Map* newMap = killer->GetMap();
            if (newMap->IsDungeon() && killed->GetLevel() >= 80 && !killed->IsCritter()) {
                //roll for it
                Group* grp = killer->GetGroup();
                uint32 grpMemberCount = grp->GetMemberSlots().max_size();
                if (grp) {
                    Loot* const loot(&killed->loot);
                    if (killed->IsDungeonBoss() || killed->isWorldBoss()) loot->AddItem(LootStoreItem(400000, 0, 100, false, LOOT_MODE_DEFAULT, grp->GetGUID(), grpMemberCount, grpMemberCount));
                    else loot->AddItem(LootStoreItem(400000, 0, 100, false, LOOT_MODE_DEFAULT, grp->GetGUID(), 1, 1));
                }
                //add it to inventory
                else {
                    if (killed->IsDungeonBoss() || killed->isWorldBoss()) killer->AddItem(400000, 5);
                    else killer->AddItem(400000, 1);
                }
            }
        }
    }

    void StartRun(Player* player) {
        Map* myMap = player->GetMap();
        if (myMap->IsDungeon() || myMap->IsRaid()) {
            // check if run is in progress
            WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_MYTHIC_DUNGEON);
            // is this correct? i want to insert the playername into the sql statement
            stmt->setString(0, player->GetName());
            QueryResult myResult = WorldDatabase.PQuery(stmt);
            if (myResult) {
                //already in progress
            }
            else {
                //start new run
                uint32 keyCount = player->GetItemCount(100000, false);
                if (keyCount > 0) {
                    ChatHandler(player->GetSession()).SendSysMessage("You have started a |cff4CFF00Mythic Dungeon|r Run. Good Luck!");
                    uint32 plvl = player->GetLevel();
                    Map::CreatureBySpawnIdContainer container = myMap->GetCreatureBySpawnIdStore();
                    //apply buffs to all dunegon npc by key count
                    switch (keyCount) {
                    case 1:
                        ChatHandler(player->GetSession()).SendSysMessage(player->GetName() + "'s Key (Level 1): Boosting all NPCs by 5%.");
                        for (auto npc : container) {
                            ApplyMythicBuffs(1, npc.second, plvl);
                        }
                        break;
                    case 2:
                        ChatHandler(player->GetSession()).SendSysMessage(player->GetName() + "'s Key (Level 2): Boosting all NPCs by 10%.");
                        for (auto npc : container) {
                            ApplyMythicBuffs(2, npc.second, plvl);
                        }
                        break;
                    case 3:
                        ChatHandler(player->GetSession()).SendSysMessage(player->GetName() + "'s Key (Level 3): Boosting all NPCs by 15% and applying 1 additional aura.");
                        for (auto npc : container) {
                            ApplyMythicBuffs(3, npc.second, plvl);
                        }
                        break;
                    case 4:
                        ChatHandler(player->GetSession()).SendSysMessage(player->GetName() + "'s Key (Level 4): Boosting all NPCs by 20% and applying 2 additional auras.");
                        for (auto npc : container) {
                            ApplyMythicBuffs(4, npc.second, plvl);
                        }
                        break;
                    case 5:
                        ChatHandler(player->GetSession()).SendSysMessage(player->GetName() + "'s Key (Level 5): Boosting all NPCs by 25% and applying 2 additional auras and extra health.");
                        for (auto npc : container) {
                            ApplyMythicBuffs(5, npc.second, plvl);
                        }
                        break;
                    default:
                        ChatHandler(player->GetSession()).SendSysMessage("You have entered a |cff4CFF00Mythic Dungeon |r.");
                        break;
                    }
                }
                else {
                    ChatHandler(player->GetSession()).SendSysMessage("You need a |cffFF3D14Mythic Key|r before starting a mythic dungeon run.");
                }
            }




        }
        else {
            ChatHandler(player->GetSession()).SendSysMessage("You need to enter a dungeon or raid to start a mythic dungeon run.");
        }
    }

    void OnSpellCast(Player* player, Spell* spell, bool skipCheck) {
        //Mythic Dungeon starting spell
        if (spell->GetSpellInfo()->Id == 500000) {
            //party run
            if (Group* grp = player->GetGroup()) {
                //only leader can start
                if (grp->GetLeaderName() == player->GetName()) {
                    //in dungeon/raid
                    StartRun(player);
                }
                else {
                    ChatHandler(player->GetSession()).SendSysMessage("Only the party leader can start a mythic dungeon run.");
                }
            }
            //solo run
            else {
                StartRun(player);
            }
        }

    }

    virtual void OnMapChanged(Player* player) {

    }

    void ApplyMythicBuffs(uint32 level, Creature* creature, uint32 plvl) {
        uint32 clvl = creature->GetLevel();
        // set level first (overrides hp mod)
        if (creature->GetLevel() != plvl) creature->SetLevel(plvl);
        //apply hp mod/dmg buff on new npcs
        if (!creature->HasAura(BUFF_INFO)) {
            uint32 stacks = plvl - clvl;
            if (stacks < 0 || creature->isWorldBoss() || creature->IsDungeonBoss()) {
                stacks = 0;
            }

            creature->SetAuraStack(SPELL_EXTRA_3, creature, stacks);
            creature->AddAura(BUFF_INFO, creature);
            creature->SetMaxHealth(creature->GetMaxHealth() * (level + stacks * 0.3));
        }
        // apply suffixes
        if (level >= 2) creature->AddAura(SPELL_EXTRA_1, creature);
        if (level >= 4) creature->AddAura(SPELL_EXTRA_2, creature);
    }
};

void AddSC_mythic_dungeons()
{
    new mythic_dungeon_player();
    new mythic_dungeon_unit();
}
