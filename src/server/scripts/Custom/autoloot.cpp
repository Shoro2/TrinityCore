#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "Loot.h"
#include "Group.h"
#include "WorldSession.h"

/*
https://www.emucoach.com/forums/threads/3-3-5-trinitycore-auto-loot-creature-on-kill-script.6540/
by edes
*/


class AutoLoot : public PlayerScript
{
public:
    AutoLoot() : PlayerScript("AutoLoot") {}

    void OnCreatureKill(Player* player, Creature* creature) {
        //get any loot the creature has
        Loot* loot(&creature->loot);

        //check if player is in group
        //i'm still looking into how to make this work
        if (Group* grp = player->GetGroup()) {
            // @todo: implement group auto loot
        }
        else {
            if (player->isAllowedToLoot(creature)) {
                if (!loot->isLooted() && !loot->empty()) {
                    //iterate over all available items
                    uint8 maxSlot = loot->GetMaxSlotInLootFor(player);
                    uint8 playerSlots = player->GetFreeInventorySpace();
                    if (playerSlots > maxSlot) {
                        //loot gold
                        uint32 gold = loot->gold;
                        if (gold > 0) {
                            player->ModifyMoney(gold);
                        }
                        for (int i = 0; i < maxSlot; ++i) {
                            //the method name here is a bit misleading, it just returns the item at a specific slot of the loot list
                            LootItem* item = loot->LootItemInSlot(i, player);
                            //so you have to add the item manually
                            player->AddItem(item->itemid, item->count);
                        }

                        //because we don't remove the items as they are looted, clear the loot list
                        loot->clear();
                    }

                    
                }
            }
        }
    }
};

void AddSC_AutoLoot()
{
    new AutoLoot();
};
