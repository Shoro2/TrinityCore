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

class mythic_dungeon_player : public PlayerScript {
public:
    mythic_dungeon_player() : PlayerScript("mythic_dungeon_player") {
    }

    virtual void OnLogin(Player* player, bool firstLogin)
    {
        ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Mythic Dungeon |rmodule.");
    }

    virtual void OnCreatureKill(Player* killer, Creature* killed) {
        Map* newMap = killer->GetMap();
        if (newMap->IsDungeon() && killed->GetLevel()==80) {
            //roll for it
            if (Group* grp = killer->GetGroup()) {
                Loot* const loot(&killed->loot);
                //uint32 _itemid, uint32 _reference, float _chance, bool _needs_quest, uint16 _lootmode, uint8 _groupid, int32 _mincount, uint8 _maxcount CAN THIS WORK?!
                uint32 grpMemberCount = grp->GetMemberSlots().max_size();
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

    void OnSpellCast(Player* player, Spell* spell, bool skipCheck) {
        //Mythic Dungeon starting spell
        if (spell->GetSpellInfo()->Id == 64036) {
            //party run
            if (Group* grp = player->GetGroup()) {
                //only leader can start
                if (grp->GetLeaderName() == player->GetName()) {
                    //in dungeon/raid
                    Map* myMap = player->GetMap();
                    if (myMap->IsDungeon() || myMap->IsRaid()) {
                        uint32 keyCount = player->GetItemCount(100000, false);
                        //has key
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
                    else {
                        ChatHandler(player->GetSession()).SendSysMessage("You need to enter a dungeon or raid to start a mythic dungeon run.");
                    }
                    
                }
                else {
                    ChatHandler(player->GetSession()).SendSysMessage("Only the party leader can start a mythic dungeon run.");
                }
            }

            //solo run
            else {
                Map* myMap = player->GetMap();
                uint32 keyCount = player->GetItemCount(100000, false);
                uint32 plvl = player->GetLevel();
                //get all creatures of the dungeon
                Map::CreatureBySpawnIdContainer container = myMap->GetCreatureBySpawnIdStore();
                if (myMap->IsDungeon() || myMap->IsRaid()) {
                    if (keyCount > 0) {
                        ChatHandler(player->GetSession()).SendSysMessage("You have started a |cff4CFF00Mythic Dungeon|r Run. Good Luck!");
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
                else {
                    ChatHandler(player->GetSession()).SendSysMessage("You need to enter a dungeon or raid to start a mythic dungeon run.");
                }
            }
            
        }
        
    }

    virtual void OnMapChanged(Player* player) {
        
    }

    void RemoveMythicBuffs(uint32 level, Creature* creature) {
        switch (level) {
        case 1:
            //remove 2-5 etc
            if (creature->HasAura(AURA_10)) creature->RemoveAura(AURA_10);
            if (creature->HasAura(AURA_15)) {
                creature->RemoveAura(AURA_15);
                creature->RemoveAura(SPELL_EXTRA_1);
            }
            if (creature->HasAura(AURA_20)) {
                creature->RemoveAura(AURA_20);
                creature->RemoveAura(SPELL_EXTRA_1);
                creature->RemoveAura(SPELL_EXTRA_2);
            }
            if (creature->HasAura(AURA_25)) {
                creature->RemoveAura(AURA_25);
                creature->RemoveAura(SPELL_EXTRA_1);
                creature->RemoveAura(SPELL_EXTRA_2);
                creature->RemoveAura(SPELL_EXTRA_3);
            }
            break;
        case 2:
            if (creature->HasAura(AURA_5)) creature->RemoveAura(AURA_5);
            if (creature->HasAura(AURA_15)) {
                creature->RemoveAura(AURA_15);
                creature->RemoveAura(SPELL_EXTRA_1);
            }
            if (creature->HasAura(AURA_20)) {
                creature->RemoveAura(AURA_20);
                creature->RemoveAura(SPELL_EXTRA_1);
                creature->RemoveAura(SPELL_EXTRA_2);
            }
            if (creature->HasAura(AURA_25)) {
                creature->RemoveAura(AURA_25);
                creature->RemoveAura(SPELL_EXTRA_1);
                creature->RemoveAura(SPELL_EXTRA_2);
                creature->RemoveAura(SPELL_EXTRA_3);
            }
            break;
        case 3:
            if (creature->HasAura(AURA_5)) creature->RemoveAura(AURA_5);
            if (creature->HasAura(AURA_10)) creature->RemoveAura(AURA_10);
            if (creature->HasAura(AURA_20)) {
                creature->RemoveAura(AURA_20);
                creature->RemoveAura(SPELL_EXTRA_1);
                creature->RemoveAura(SPELL_EXTRA_2);
            }
            if (creature->HasAura(AURA_25)) {
                creature->RemoveAura(AURA_25);
                creature->RemoveAura(SPELL_EXTRA_1);
                creature->RemoveAura(SPELL_EXTRA_2);
                creature->RemoveAura(SPELL_EXTRA_3);
            }
            break;
        case 4:
            if (creature->HasAura(AURA_5)) creature->RemoveAura(AURA_5);
            if (creature->HasAura(AURA_10)) creature->RemoveAura(AURA_10);
            if (creature->HasAura(AURA_15)) {
                creature->RemoveAura(AURA_15);
                creature->RemoveAura(SPELL_EXTRA_1);
            }
            if (creature->HasAura(AURA_25)) {
                creature->RemoveAura(AURA_25);
                creature->RemoveAura(SPELL_EXTRA_1);
                creature->RemoveAura(SPELL_EXTRA_2);
                creature->RemoveAura(SPELL_EXTRA_3);
            }
            break;
        case 5:
            if (creature->HasAura(AURA_5)) creature->RemoveAura(AURA_5);
            if (creature->HasAura(AURA_10)) creature->RemoveAura(AURA_10);
            if (creature->HasAura(AURA_15)) {
                creature->RemoveAura(AURA_15);
                creature->RemoveAura(SPELL_EXTRA_1);
            }
            if (creature->HasAura(AURA_20)) {
                creature->RemoveAura(AURA_20);
                creature->RemoveAura(SPELL_EXTRA_1);
                creature->RemoveAura(SPELL_EXTRA_2);
            }
            break;
        default:
            break;
        }

    }

    void ApplyMythicBuffs(uint32 level, Creature* creature, uint32 plvl) {
        RemoveMythicBuffs(level, creature);
        uint32 clvl = creature->GetLevel();
        
        switch (level) {
        case 1:
            // 5% stats
            if (creature->GetLevel() != plvl) creature->SetLevel(plvl);
            if (!creature->HasAura(AURA_5)) creature->AddAura(AURA_5, creature);
            /* BUFF OR MOD?
            for (int32 i = STAT_STRENGTH; i < MAX_STATS; ++i) {
                creature->ApplyStatPctModifier(UnitMods(UNIT_MOD_STAT_START + i), TOTAL_PCT, float(100));
            }
            */
            break;
        case 2:
            // 10% stats + 1st suffix
            if (creature->GetLevel() != plvl) creature->SetLevel(plvl);
            if (!creature->HasAura(AURA_10)) creature->AddAura(AURA_10, creature);
            if (!creature->HasAura(SPELL_EXTRA_1)) creature->AddAura(SPELL_EXTRA_1, creature);
            break;
        case 3:
            // 15% stats + 1st suffix
            if (creature->GetLevel() != plvl) creature->SetLevel(plvl);
            if (!creature->HasAura(AURA_15)) creature->AddAura(AURA_15, creature);
            if (!creature->HasAura(SPELL_EXTRA_1)) creature->AddAura(SPELL_EXTRA_1, creature);
            break;
        case 4:
            // 20% stats + 2 suffixes
            if (creature->GetLevel() != plvl) creature->SetLevel(plvl);
            if (!creature->HasAura(AURA_20)) creature->AddAura(AURA_20, creature);
            if (!creature->HasAura(SPELL_EXTRA_1)) creature->AddAura(SPELL_EXTRA_1, creature);
            if (!creature->HasAura(SPELL_EXTRA_2)) creature->AddAura(SPELL_EXTRA_2, creature);
            break;
        case 5:
            // 25% stats + 2 suffixes + extra health
            if (!creature->HasAura(SPELL_EXTRA_1)) creature->AddAura(SPELL_EXTRA_1, creature);
            if (!creature->HasAura(SPELL_EXTRA_2)) creature->AddAura(SPELL_EXTRA_2, creature);
            if (clvl != plvl) creature->SetLevel(plvl);
            if (!creature->HasAura(AURA_25)) creature->AddAura(AURA_25, creature);
            break;
        }
        
        if (!creature->HasAura(AURA_5) || !creature->HasAura(AURA_10) || !creature->HasAura(AURA_15) || !creature->HasAura(AURA_20) || !creature->HasAura(AURA_25) || !creature->HasAura(AURA_30)) {
            //dmg health buff 
            uint32 stacks = plvl - clvl;
            if (stacks < 0) stacks = 0;
            creature->SetAuraStack(SPELL_EXTRA_3, creature, stacks);
            creature->SetMaxHealth(creature->GetMaxHealth() * (2 + stacks * 0.3));
        }
        
    }
};

void AddSC_mythic_dungeons()
{
    new mythic_dungeon_player();
}
