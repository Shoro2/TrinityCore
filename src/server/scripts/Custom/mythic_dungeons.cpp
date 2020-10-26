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
#include "Log.h"
#include <iostream>
#include <string>
#include "GameTime.h"
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
                Player* myPlayer = creature->SelectNearestPlayer(999);
                Group* grp = myPlayer->GetGroup();
                std::string pName = grp->GetLeaderName();
                if (QueryResult myResult = WorldDatabase.PQuery("SELECT * FROM world.custom_speedruns_runs WHERE player = '" + pName + "'")) {
                    uint32 level;
                    do {
                        Field* myField = myResult->Fetch();
                        level = myField[6].GetUInt32();
                    } while (myResult->NextRow());
                    ApplyMythicBuffs(level, creature, 80);
                }
            }
        }
    }

    void ApplyMythicBuffs(uint32 level, Unit* creature, uint32 plvl) {
        uint32 clvl = creature->GetLevel();
        // set level first (overrides hp mod)
        if (!creature->HasAura(BUFF_INFO_NPC)) {
            uint32 stacks = plvl - clvl;
            if (stacks < 0)  stacks = 0;

            if (creature->GetLevel() < plvl) creature->SetLevel(plvl + 1);
            creature->SetAuraStack(AURA_DMG_5, creature, stacks + level * 5);
            creature->SetMaxHealth(creature->GetMaxHealth() * (level + stacks * 0.3));
            creature->AddAura(BUFF_INFO_NPC, creature);
            // apply suffixes
            if (level >= 2) creature->AddAura(SPELL_EXTRA_1, creature);
            if (level >= 4) creature->AddAura(SPELL_EXTRA_2, creature);
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
        if (killer->HasAura(500104)) {
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

        // TODO: Check if creature is dungeon end boss to end the run and add awards


    }

    void StartRun(Player* player) {
        Map* myMap = player->GetMap();
        ChatHandler myCH = ChatHandler(player->GetSession());
        // check if run is in progress
        //WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_MYTHIC_DUNGEON);
        // is this correct? i want to insert the playername into the sql statement
        //stmt->setString(0, player->GetName());
        auto const pName = player->GetName();
        //start new run
        uint32 keyCount = player->GetItemCount(100000, false);
        WorldDatabase.PQuery("INSERT INTO custom_speedruns_runs (player, dungeon, tstart, bosses_left, keylevel) VALUES ('" + pName + "','" + myMap->GetMapName() + "','" + std::to_string(GameTime::GetGameTime()) + "',0, "+ std::to_string(keyCount)+")");
        TC_LOG_DEBUG("LOG_LEVEL_DEBUG", "Mythic Dungeon: Player %s started a new a run.", pName);
        player->AddAura(BUFF_INFO_PLAYER, player);
        // TODO: Party run
        if (Group* grp = player->GetGroup()) {
            Group::MemberSlotList myList = grp->GetMemberSlots();
            for  (const auto& member : myList){
                myCH.SendSysMessage(member.name);
                Player* partyMember = ObjectAccessor::FindConnectedPlayer(member.guid);
                partyMember->SetAuraStack(BUFF_INFO_PLAYER, partyMember, keyCount);;
            }
            
            
        }
        else {
            player->SetAuraStack(BUFF_INFO_PLAYER, player, keyCount);
        }
        
        //apply buffs to all dunegon npc
        Map::CreatureBySpawnIdContainer container = myMap->GetCreatureBySpawnIdStore();
        myCH.SendSysMessage("Activated " + pName + "'s Key (Level "+std::to_string(keyCount)+"): Good Luck.");
        uint32 plvl = player->GetLevel();
        for (auto npc : container) {
            ApplyMythicBuffs(keyCount, npc.second, plvl);
        }





    }

    void OnSpellCast(Player* player, Spell* spell, bool skipCheck) {
        ChatHandler myCH = ChatHandler(player->GetSession());
        Position myPos = player->GetPosition();
        Map* myMap = player->GetMap();
        auto const pName = player->GetName();
        //Mythic Dungeon starting spell
        if (spell->GetSpellInfo()->Id == 500000) {
            if (myMap->IsDungeon() || myMap->IsRaid()) {
                if (player->GetLevel() >= 80 && !player->IsInCombat()) {
                    //party run
                    if (Group* grp = player->GetGroup()) {
                        //only leader can start
                        if (grp->GetLeaderName() != player->GetName()) {
                            myCH.SendSysMessage("Only the party leader can start a mythic dungeon run.");
                            return;
                        }
                    }
                    //solo run
                    else {
                        TC_LOG_DEBUG("LOG_LEVEL_DEBUG", "Mythic Dungeon: Query -> SELECT * FROM world.custom_speedruns_runs WHERE player = '%s'", pName);
                        bool found = false;
                        if (QueryResult myResult = WorldDatabase.PQuery("SELECT * FROM world.custom_speedruns_runs WHERE player = '" + pName + "'")) {
                            do {
                                Field* myField = myResult->Fetch();
                                auto const rName = myField[0].GetString();
                                if (rName == pName) found = true;
                            } while (myResult->NextRow());
                        }
                        if (found) {
                            //already in progress
                            TC_LOG_DEBUG("LOG_LEVEL_DEBUG", "Mythic Dungeon: Player %s is already in a run.", pName);
                            myCH.SendSysMessage("Already in progress!");
                        }
                        else {
                            uint32 keyCount = player->GetItemCount(100000, false);
                            if (keyCount > 0) {
                                StartRun(player);
                            }
                            else {
                                myCH.SendSysMessage("You need a |cffFF3D14Mythic Key|r before starting a mythic dungeon run.");
                            }
                        }
                    }
                }
            }
            else {
                myCH.SendSysMessage("You need to enter a dungeon or raid to start a mythic dungeon run.");
            }
        }
        
    }

    virtual void OnMapChanged(Player* player) {

    }

    void ApplyMythicBuffs(uint32 level, Creature* creature, uint32 plvl) {
        uint32 clvl = creature->GetLevel();
        // set level first (overrides hp mod)
        if (!creature->HasAura(BUFF_INFO_NPC)) {
            uint32 stacks = plvl - clvl;
            if (stacks < 0)  stacks = 0;

            if (creature->GetLevel() < plvl) creature->SetLevel(plvl + 1);
                creature->SetAuraStack(AURA_DMG_5, creature, stacks + level * 5);
                creature->SetMaxHealth(creature->GetMaxHealth() * (level + stacks * 0.3));

            
            creature->AddAura(BUFF_INFO_NPC, creature);
            // apply suffixes
            if (level >= 2) creature->AddAura(SPELL_EXTRA_1, creature);
            if (level >= 4) creature->AddAura(SPELL_EXTRA_2, creature);
        }
    }
};

void AddSC_mythic_dungeons()
{
    new mythic_dungeon_player();
    new mythic_dungeon_unit();
}
