/*
Author:             GvR Mr Mister(https://github.com/Shoro2)

*/
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

    virtual void OnEvadeEnter(Creature* creature) {
        Map* myMap = creature->GetMap();
        uint32 myId = creature->GetInstanceId();
        if (QueryResult myResult = WorldDatabase.PQuery("SELECT * FROM world.custom_speedruns_runs WHERE instanceid = " + std::to_string(myId) + "")) {
            Field* myField = myResult->Fetch();
            uint32 runLevel = myField[6].GetUInt32();
            uint32 dungeonLevel = myField[8].GetUInt32();
            RefreshMythicBuffs(runLevel, creature, dungeonLevel);
        }
    }

    void RefreshMythicBuffs(uint32 level, Creature* creature, uint32 dungeonLevel) {
        // set level first (overrides hp mod)
        if (!creature->HasAura(BUFF_INFO_NPC)) {
            creature->AddAura(BUFF_INFO_NPC, creature);
            creature->SetLevel(1);
            creature->SetLevel(83);
            uint32 stacks = 80 - dungeonLevel;
            if (stacks < 0)  stacks = 0;
            //mods for world bosses only
            if (creature->isWorldBoss()) {
                creature->SetAuraStack(AURA_DMG_5, creature, level * 5);
                creature->SetMaxHealth(creature->GetMaxHealth() * level);
            }
            //mods for dungeon bosses only
            else if (creature->IsDungeonBoss()) {
                creature->SetAuraStack(AURA_DMG_5, creature, stacks + level * 5);
                creature->SetMaxHealth(creature->GetMaxHealth() * level + stacks * 0.3);
            }
            //mods for non bosses only
            else {
                creature->SetAuraStack(AURA_DMG_5, creature, stacks + level * 5);
                creature->SetMaxHealth(creature->GetMaxHealth() * (level + stacks * 0.3));
            }
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

    virtual void OnLogout(Player* player) {
        auto const pName = player->GetName();
        if (QueryResult myResult = WorldDatabase.PQuery("SELECT * FROM world.custom_speedruns_runs WHERE player = " + pName + "")) {
            WorldDatabase.PQuery("DELETE FROM world.custom_speedruns_runs WHERE player = '" + pName + "'");
        }
    }

    virtual void OnPlayerUpdateZone(Player* player, uint32 newZone, uint32 newArea) {

    }

    virtual void OnPlayerKilledByCreature(Creature* killer, Player* killed) {
        //todo: time lost
        std::string pName;
        Group* grp = killed->GetGroup();
        if (grp) pName = grp->GetLeaderName();
        else pName = killed->GetName();
        if (QueryResult myResult = WorldDatabase.PQuery("SELECT * FROM world.custom_speedruns_runs WHERE player = " + pName + "")) {
            WorldDatabase.PQuery("UPDATE world.custom_speedruns_runs SET score = score - 10 WHERE player = '%s'", pName);
        }
        
    }

    virtual void OnPlayerBindToInstance(Player* player, Difficulty difficulty, uint32 mapid, bool permanent, uint8 extendState) {
        //end run if player is saved to another id
        auto const pName = player->GetName();
        if (QueryResult myResult = WorldDatabase.PQuery("SELECT * FROM world.custom_speedruns_runs WHERE player = " + pName + "")) {
            Field* myField = myResult->Fetch();
            uint32 instanceId = myField[7].GetUInt32();
            if (instanceId != player->GetInstanceId()) {
                WorldDatabase.PQuery("DELETE FROM world.custom_speedruns_runs WHERE player = '" + pName + "'");
            }
        }
    }

    virtual void OnCreatureKill(Player* killer, Creature* killed) {
        // loot/score
        std::string pName;
        if (killer->HasAura(BUFF_INFO_PLAYER)) {
            Map* newMap = killer->GetMap();
            if (newMap->IsDungeon() && killed->GetLevel() >= 80 && !killed->IsCritter()) {
                //roll for it
                Group* grp = killer->GetGroup();
                // add dungeon mark if killed has the mythic dungeon buff
                if (grp) {
                    uint32 grpMemberCount = grp->GetMemberSlots().max_size();
                    Loot* const loot(&killed->loot);
                    pName = grp->GetLeaderName();
                    if (killed->IsDungeonBoss() || killed->isWorldBoss()) loot->AddItem(LootStoreItem(ITEM_MARK, 0, 100, false, LOOT_MODE_DEFAULT, grp->GetGUID(), grpMemberCount, grpMemberCount));
                    else loot->AddItem(LootStoreItem(ITEM_MARK, 0, 100, false, LOOT_MODE_DEFAULT, grp->GetGUID(), 1, 1));
                }
                //add it to inventory
                else {
                    pName = killer->GetName();
                    if (killed->IsDungeonBoss() || killed->isWorldBoss()) killer->AddItem(400000, 5);
                    else killer->AddItem(ITEM_MARK, 1);
                }
                WorldDatabase.PQuery("UPDATE world.custom_speedruns_runs SET score = score + 1 WHERE player = '%s'", pName);
                // dungeonboss down?
                if (killed->IsDungeonBoss() || killed->isWorldBoss()) {
                    auto const mapName = newMap->GetMapName();
                    if (QueryResult myResult = WorldDatabase.PQuery("SELECT * FROM world.custom_speedruns_dungeons WHERE name = '%s'", mapName)) {
                        Field* myField_dungeons = myResult->Fetch();
                        uint32 maxBosses = myField_dungeons[1].GetUInt32();
                        uint32 myEntry = killed->GetEntry();
                        for (uint32 i = 0; i < 20; i++) {
                            //mythic dungeon boss?
                            uint32 bossEntry = myField_dungeons[2 + i].GetUInt32();
                            //todo check for hc mode (additional bosses?!)
                            if (myEntry == bossEntry) {
                                ChatHandler myCH = ChatHandler(killer->GetSession());
                                //check for active run
                                if (QueryResult myResult = WorldDatabase.PQuery("SELECT * FROM world.custom_speedruns_runs WHERE player = '%s'", pName)) {
                                    Field* myField_runs = myResult->Fetch();
                                    uint32 bossesLeft = myField_runs[4].GetUInt32();
                                    if (newMap->IsHeroic()) {
                                        uint32 extraBosses = myField_dungeons[24].GetUInt32();
                                        bossesLeft = bossesLeft + extraBosses;
                                    }
                                    uint32 score = myField_runs[5].GetUInt32();
                                    if (bossesLeft >= 2) {
                                        //update run, boss  down
                                        WorldDatabase.PQuery("UPDATE world.custom_speedruns_runs SET bosses_left = bosses_left - 1 WHERE player = '%s'", pName);
                                        myCH.SendSysMessage("Downed Boss " + killed->GetName() + ", " + std::to_string(bossesLeft - 1) + " bosses left to finish the run.");
                                    }
                                    else if (bossesLeft = 1) {
                                        //end run
                                        WorldDatabase.PQuery("DELETE FROM world.custom_speedruns_runs WHERE player = '%s'", pName);
                                        uint32 timeStart = myField_runs[2].GetUInt32();
                                        uint32 timeNow = GameTime::GetGameTime();
                                        uint32 timeNeeded = timeNow - timeStart;
                                        uint32 timeMax = myField_dungeons[23].GetUInt32();
                                        // successfull run
                                        if (timeNeeded < timeMax) {
                                            //stats
                                            uint32 finalScore = score + (timeMax - timeNeeded);
                                            WorldDatabase.PQuery("INSERT into world.custom_speedruns_results (dungeon, player, time, score) VALUES ('%s', '" + pName + "', " + std::to_string(timeNeeded) + ", " + std::to_string(finalScore) + ")", mapName);
                                            myCH.SendSysMessage("Finished the mythic dungeon run for %s:", mapName);
                                            if (grp)  myCH.SendSysMessage("Leader: " + pName);
                                            else myCH.SendSysMessage("Player: " + pName);
                                            myCH.SendSysMessage("Seconds Needed: " + std::to_string(timeNeeded));
                                            myCH.SendSysMessage("Score: " + std::to_string(finalScore));
                                            //rewards
                                            if (grp) {
                                                Group::MemberSlotList myList = grp->GetMemberSlots();
                                                for (const auto& member : myList) {
                                                    myCH.SendSysMessage(member.name);
                                                    Player* partyMember = ObjectAccessor::FindConnectedPlayer(member.guid);
                                                    ChatHandler partyCH = ChatHandler(partyMember->GetSession());
                                                    uint32 rewardMoney = finalScore * 100;
                                                    partyMember->ModifyMoney(partyMember->GetMoney() + rewardMoney);
                                                    partyMember->AddItem(ITEM_MARK, finalScore);
                                                    partyCH.SendSysMessage("Added " + std::to_string(rewardMoney) + " Copper and " + std::to_string(finalScore) + " mythic dungeon marks as reward.");
                                                }
                                            }
                                            else {

                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void StartRun(Player* player) {
        Map* myMap = player->GetMap();
        ChatHandler myCH = ChatHandler(player->GetSession());
        auto const pName = player->GetName();
        //start new run
        uint32 keyCount = player->GetItemCount(ITEM_KEY, false);
        uint32 myId = player->GetInstanceId();
        auto mapName = myMap->GetMapName();
        if (QueryResult myResult = WorldDatabase.PQuery("SELECT * FROM world.custom_speedruns_dungeons WHERE name='%s'", mapName)) {
            Field* myField = myResult->Fetch();
            uint32 dungeonLevel = myField[22].GetUInt32();
            //hc version of dungeon?
            if (dungeonLevel < 70 && dungeonLevel >= 60 && myMap->IsHeroic()) dungeonLevel = 70;
            else if (dungeonLevel < 80 && dungeonLevel >= 70 && myMap->IsHeroic()) dungeonLevel = 80;
            uint32 maxBosses = myField[1].GetUInt32();
            WorldDatabase.PQuery("INSERT INTO custom_speedruns_runs (player, dungeon, tstart, bosses_left, keylevel, instanceid, dungeonlevel) VALUES ('" + pName + "','" + myMap->GetMapName() + "','" + std::to_string(GameTime::GetGameTime()) + "'," + std::to_string(maxBosses) + ", " + std::to_string(keyCount) + "," + std::to_string(myId) + "," + std::to_string(dungeonLevel) + ")");
            TC_LOG_DEBUG("LOG_LEVEL_DEBUG", "Mythic Dungeon: Player %s started a new a run.", pName);
            player->AddAura(BUFF_INFO_PLAYER, player);
            // Is this working?!
            if (Group* grp = player->GetGroup()) {
                Group::MemberSlotList myList = grp->GetMemberSlots();
                for (const auto& member : myList) {
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
            myCH.SendSysMessage("Activated " + pName + "'s Key (Level " + std::to_string(keyCount) + "): Good Luck.");
            for (auto npc : container) {
                ApplyMythicBuffs(keyCount, npc.second, dungeonLevel);
            }
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
                            uint32 keyCount = player->GetItemCount(ITEM_KEY, false);
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

    void ApplyMythicBuffs(uint32 level, Creature* creature, uint32 dungeonLevel) {
        if (!creature->HasAura(BUFF_INFO_NPC)) {
            creature->AddAura(BUFF_INFO_NPC, creature);
            creature->SetLevel(1);
            creature->SetLevel(83);
            uint32 stacks = 80 - dungeonLevel;
            if (stacks < 0)  stacks = 0;
            //mods for world bosses only
            if (creature->isWorldBoss()) {
                creature->SetAuraStack(AURA_DMG_5, creature, level * 5);
                creature->SetMaxHealth(creature->GetMaxHealth() * level);
            }
            //mods for dungeon bosses only
            else if (creature->IsDungeonBoss()) {
                creature->SetAuraStack(AURA_DMG_5, creature, stacks + level * 5);
                creature->SetMaxHealth(creature->GetMaxHealth() * level + stacks * 0.3);
            }
            //mods for non bosses only
            else {
                creature->SetAuraStack(AURA_DMG_5, creature, stacks + level * 5);
                creature->SetMaxHealth(creature->GetMaxHealth() * (level + stacks * 0.3));
            }
            // apply suffixes
            if (level >= 2) creature->AddAura(SPELL_EXTRA_1, creature);
            if (level >= 4) creature->AddAura(SPELL_EXTRA_2, creature);

            creature->SetFullHealth();
        }
    }
};

void AddSC_mythic_dungeons()
{
    new mythic_dungeon_player();
    new mythic_dungeon_unit();
}
