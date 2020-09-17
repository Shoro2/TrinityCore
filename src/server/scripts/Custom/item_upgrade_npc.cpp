// Database
#include <DatabaseEnv.h>

// Scripting
#include <ScriptMgr.h>

// Creature
#include <ScriptedCreature.h>

// Misc
#include <Player.h>
#include <Item.h>
#include <Chat.h>

// Utilities
#include <ScriptedGossip.h>
#include <Log.h>
#include <SharedDefines.h>
#include <Random.h>

// Standard
#include <vector>
#include <unordered_set>
#include <iomanip>
#include <sstream>

// CONSTANTS & ENUMS

static const std::string PREFIX = "|cff006080UPGRADE:|r ";

static const std::vector<uint8> WEAPONRY_SLOTS = std::vector<uint8>{
	EQUIPMENT_SLOT_MAINHAND,
	EQUIPMENT_SLOT_OFFHAND,
	EQUIPMENT_SLOT_RANGED
};

static const std::vector<uint8> ARMORY_SLOTS = std::vector<uint8>{
	EQUIPMENT_SLOT_HEAD,
	EQUIPMENT_SLOT_SHOULDERS,
	EQUIPMENT_SLOT_BACK,
	EQUIPMENT_SLOT_CHEST,
	EQUIPMENT_SLOT_BODY,
	EQUIPMENT_SLOT_TABARD,
	EQUIPMENT_SLOT_WRISTS,
	EQUIPMENT_SLOT_HANDS,
	EQUIPMENT_SLOT_WAIST,
	EQUIPMENT_SLOT_LEGS,
	EQUIPMENT_SLOT_FEET
};

static const std::vector<uint8> JEWELRY_SLOTS = std::vector<uint8>{
	EQUIPMENT_SLOT_NECK,
	EQUIPMENT_SLOT_FINGER1,
	EQUIPMENT_SLOT_FINGER2,
	EQUIPMENT_SLOT_TRINKET1,
	EQUIPMENT_SLOT_TRINKET2,
};

enum ACTION {
	MAIN_MENU = 1000,
	RELOAD = 1001,
	NEVERMIND = 1002,
	BACK = 1003,
	UPGRADE = 1004,
	MAIN_MENU_MENU = 1100,
	SLOTS = 1200,
	RECIPES = 2000,
};

// ITEM UPGRADE AND DATABASE

struct Instance {
	uint32 itemEntry;
	uint8 amount;
};

struct Upgrade {
	Upgrade() { }

	uint32 reqGold, reqHonor, reqArena;
	float success;
	std::list<Instance> requirements;
};

static std::unordered_map<uint32, std::vector<std::pair<uint32, Upgrade>>> depository = { };

static const void LoadUpgradableItemsFromDatabase() {
	QueryResult upgradeQuery = WorldDatabase.PQuery("SELECT item_entry, upgrade_entry, gold, honor, arena, success"
		" FROM item_upgrade_system;");

	if (upgradeQuery) {
		depository.clear();

		do {
			Field* upgradeFields = upgradeQuery->Fetch();

			uint32 itemEntry = upgradeFields[0].GetUInt32();
			uint32 upgradeEntry = upgradeFields[1].GetUInt32();

			Upgrade upgrade;
			upgrade.reqGold = upgradeFields[2].GetUInt32();
			upgrade.reqHonor = upgradeFields[3].GetUInt32();
			upgrade.reqArena = upgradeFields[4].GetUInt32();
			upgrade.success = upgradeFields[5].GetFloat();

			QueryResult requireQuery = WorldDatabase.PQuery("SELECT req_item, amount"
				" FROM item_upgrade_system_requirements"
				" WHERE item_entry = " + std::to_string(itemEntry) +
				" AND upgrade_entry = " + std::to_string(upgradeEntry) + ";");

			if (requireQuery) {
				do {
					Field* requireFields = requireQuery->Fetch();

					Instance instance;
					instance.itemEntry = requireFields[0].GetUInt32();
					instance.amount = requireFields[1].GetUInt8();

					upgrade.requirements.push_back(instance);
				} while (requireQuery->NextRow());
			}

			depository[itemEntry].push_back({ upgradeEntry, upgrade });
		} while (upgradeQuery->NextRow());
	}
}

// HELPER FUNCTIONS

static const std::string DecimalToHex(uint32 decimal) {
	std::stringstream ss;
	ss << std::hex << decimal;
	return ss.str();
}

static const std::string GetIconName(uint8 slot, bool usingRelic, bool usingShield) {
	std::string icon;

	switch (slot) {
	case EQUIPMENT_SLOT_HEAD: 		icon = "Head"; break;
	case EQUIPMENT_SLOT_NECK: 		icon = "Neck"; break;
	case EQUIPMENT_SLOT_SHOULDERS: 	icon = "Shoulder"; break;
	case EQUIPMENT_SLOT_BODY: 		icon = "Shirt"; break;
	case EQUIPMENT_SLOT_CHEST:
	case EQUIPMENT_SLOT_BACK: 		icon = "Chest"; break;
	case EQUIPMENT_SLOT_WAIST: 		icon = "Waist"; break;
	case EQUIPMENT_SLOT_LEGS: 		icon = "Legs"; break;
	case EQUIPMENT_SLOT_FEET: 		icon = "Feet"; break;
	case EQUIPMENT_SLOT_WRISTS: 	icon = "Wrists"; break;
	case EQUIPMENT_SLOT_HANDS: 		icon = "Hands"; break;
	case EQUIPMENT_SLOT_FINGER1:
	case EQUIPMENT_SLOT_FINGER2: 	icon = "Finger"; break;
	case EQUIPMENT_SLOT_TRINKET1:
	case EQUIPMENT_SLOT_TRINKET2: 	icon = "Trinket"; break;
	case EQUIPMENT_SLOT_MAINHAND: 	icon = "MainHand"; break;
	case EQUIPMENT_SLOT_OFFHAND: 	icon = usingShield ? "SecondaryHand" : "MainHand"; break;
	case EQUIPMENT_SLOT_RANGED: 	icon = usingRelic ? "Relic" : "Ranged"; break;
	case EQUIPMENT_SLOT_TABARD: 	icon = "Tabard"; break;
	default: icon = "EmptySlot";
	}

	return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-" + icon + ":32:32:-25:1|t";
};

static const std::string GetItemDisplay(uint32 entry) {
	QueryResult itemQuery = WorldDatabase.PQuery("SELECT name, quality FROM item_template WHERE entry = " + std::to_string(entry) + ";");

	std::string itemDisplay = "";
	if (itemQuery) {
		Field* fields = itemQuery->Fetch();

		itemDisplay = "|c" + DecimalToHex(ItemQualityColors[fields[1].GetUInt8()]) + fields[0].GetString() + "|r";
	}

	return itemDisplay;
}

// global data to save information through gossip selection

struct PlayerData {
	ObjectGuid selectedItem;
	uint32 selectedRecipe;
	std::stack<uint32> breadcrumbs;
};

// ObjectGuid is Player's GUID
static std::unordered_map<ObjectGuid, PlayerData> data = { };

// GOSSIP

class item_upgrade_npc : public CreatureScript {
public:
	item_upgrade_npc() : CreatureScript("item_upgrade_npc") {
		LoadUpgradableItemsFromDatabase();
	}

	struct item_upgrade_npcAI : public ScriptedAI {
		item_upgrade_npcAI(Creature* creature) : ScriptedAI(creature) { }

		// Called when a player opens a gossip dialog with the creature.
		bool GossipHello(Player* player) {
			data[player->GetGUID()].breadcrumbs = {};

			AddGossipItemFor(player, GOSSIP_ICON_TABARD,
				"|TInterface\\ICONS\\INV_Sword_2H_AshbringerCorrupt:32:32:-25:1|t Weaponry",
				GOSSIP_SENDER_MAIN, ACTION::MAIN_MENU_MENU);
			AddGossipItemFor(player, GOSSIP_ICON_TABARD,
				"|TInterface\\ICONS\\INV_HELMET_96:32:32:-25:1|t Armory",
				GOSSIP_SENDER_MAIN, ACTION::MAIN_MENU_MENU + 1);
			AddGossipItemFor(player, GOSSIP_ICON_TABARD,
				"|TInterface\\ICONS\\INV_JEWELRY_RING_80:32:32:-25:1|t Jewelry",
				GOSSIP_SENDER_MAIN, ACTION::MAIN_MENU_MENU + 2);
			// AddGossipItemFor(player, GOSSIP_ICON_TABARD, 
				// "|TInterface\\BUTTONS\\Button-Backpack-Up:32:32:-25:1|t Backpack", 
				// GOSSIP_SENDER_MAIN, ACTION::MAIN_MENU_MENU + 3);

			if (player->IsGameMaster()) {
				AddGossipItemFor(player, GOSSIP_ICON_TABARD,
					"|TInterface\\PaperDollInfoFrame\\UI-GearManager-Undo:32:32:-25:1|t Reload Tables",
					GOSSIP_SENDER_MAIN, ACTION::RELOAD);
			}

			AddGossipItemFor(player, GOSSIP_ICON_TABARD,
				"|TInterface\\ICONS\\Ability_Hunter_MarkedForDeath:32:32:-25:1|t Nevermind",
				GOSSIP_SENDER_MAIN, ACTION::NEVERMIND);

			player->TalkedToCreature(me->GetEntry(), me->GetGUID());
			SendGossipMenuFor(player, 1, me->GetGUID());

			return true;
		}

		// Called when a player selects a gossip item in the creature's gossip menu.
		bool GossipSelect(Player* player, uint32 menuId, uint32 gossipListId) {
			uint32 action = GetGossipActionFor(player, gossipListId);

			ClearGossipMenuFor(player);

			std::stack<uint32>& breadcrumbs = data[player->GetGUID()].breadcrumbs;

			// Back functionality
			// Changes the action of the previous click
			if (action == ACTION::BACK && !breadcrumbs.empty()) {
				if (breadcrumbs.size() == 1) {
					GossipHello(player);
				}
				else {
					breadcrumbs.pop();
					action = breadcrumbs.top();
				}
			}

			if (action != ACTION::BACK && action != ACTION::RELOAD)
				AddToBreadcrumbs(breadcrumbs, player, action);

			if (action == ACTION::MAIN_MENU) {
				GossipHello(player);

			}
			else if (action == ACTION::RELOAD) {
				LoadUpgradableItemsFromDatabase();
				ChatHandler(player->GetSession()).PSendSysMessage("%s Tables have been reloaded.", PREFIX);
				GossipHello(player);

			}
			else if (action == ACTION::NEVERMIND) {
				CloseGossipMenuFor(player);

			}
			else if (action == ACTION::UPGRADE) {
				handleUpgrade(player, action);

			}
			else if (action >= ACTION::MAIN_MENU_MENU && action <= ACTION::MAIN_MENU_MENU + 3) {
				// an item in main menu

				switch (action) {
				case ACTION::MAIN_MENU_MENU: { // weaponry
					ShowSlots(player, WEAPONRY_SLOTS, action);
					break;
				}
				case ACTION::MAIN_MENU_MENU + 1: { // armory
					ShowSlots(player, ARMORY_SLOTS, action);
					break;
				}
				case ACTION::MAIN_MENU_MENU + 2: { // jewelry
					ShowSlots(player, JEWELRY_SLOTS, action);
					break;
				}
											   // case ACTION::MAIN_MENU_MENU + 3: { // backpack
											   // 	break;
											   // }
				}

				AddBackItem(player);
				SendGossipMenuFor(player, 1, me->GetGUID());
			}
			else if (action >= ACTION::SLOTS && action < ACTION::SLOTS + 500) {
				// an upgradable slot has been selected
				handleSelection(player, action);

				AddBackItem(player);
				SendGossipMenuFor(player, 1, me->GetGUID());
			}
			else if (action >= ACTION::RECIPES) {
				handleRecipeSelection(player, action);

				AddBackItem(player);
				SendGossipMenuFor(player, 1, me->GetGUID());
			}

			return true;
		}

		void handleUpgrade(Player* player, uint32 action) {
			PlayerData& playerData = data[player->GetGUID()];
			Item* item = player->GetItemByGuid(playerData.selectedItem);

			uint32 itemEntry = item->GetTemplate()->ItemId;
			uint32 upgradeEntry = depository[itemEntry][playerData.selectedRecipe].first;
			Upgrade& upgrade = depository[itemEntry][playerData.selectedRecipe].second;

			bool hasGold = player->HasEnoughMoney(upgrade.reqGold * 10000);
			bool hasHonorPoints = player->GetHonorPoints() >= upgrade.reqHonor;
			bool hasArenaPoints = player->GetArenaPoints() >= upgrade.reqArena;
			bool hasRequirements = true;

			for (Instance& req : upgrade.requirements) {
				if (!player->HasItemCount(req.itemEntry, req.amount, true)) {
					hasRequirements = false;
					break;
				}
			}

			bool hasEverything = hasGold && hasHonorPoints && hasArenaPoints && hasRequirements;

			if (!hasEverything) {
				if (!hasGold) {
					uint32 difference = upgrade.reqGold - player->GetMoney() / 10000;
					AddGossipItemFor(player, GOSSIP_ICON_TABARD,
						"|TInterface\\ICONS\\INV_Misc_Coin_02:32:32:-25:1|t"
						"You still need " + std::to_string(difference) + " Gold.",
						GOSSIP_SENDER_MAIN, action);
				}

				if (!hasHonorPoints) {
					std::string honorIcon = "|TInterface\\ICONS\\Achievement_PVP_";
					honorIcon.append(player->GetTeamId() == TEAM_ALLIANCE ? "A" : "H");
					honorIcon.append("_16:32:32:-25:1|t");

					uint32 difference = upgrade.reqHonor - player->GetHonorPoints();
					AddGossipItemFor(player, GOSSIP_ICON_TABARD,
						honorIcon + "You still need " + std::to_string(difference) + " Honor Points.",
						GOSSIP_SENDER_MAIN, action);
				}

				if (!hasArenaPoints) {
					uint32 difference = upgrade.reqArena - player->GetArenaPoints();
					AddGossipItemFor(player, GOSSIP_ICON_TABARD,
						"|TInterface\\ICONS\\PVP-ArenaPoints-Icon:32:32:-25:1|t"
						"You still need " + std::to_string(difference) + " Arena Points.",
						GOSSIP_SENDER_MAIN, action);
				}

				if (!hasRequirements) {
					for (Instance& req : upgrade.requirements) {
						uint32 count = player->GetItemCount(req.itemEntry, true);

						if (req.amount > count) {
							uint32 difference = req.amount - count;
							AddGossipItemFor(player, GOSSIP_ICON_TABARD,
								"|TInterface\\ICONS\\INV_Misc_QuestionMark:32:32:-25:1|t"
								"You still need " + std::to_string(difference) + " of [" + GetItemDisplay(req.itemEntry) + "].",
								GOSSIP_SENDER_MAIN, action);
						}
					}
				}

				AddBackItem(player);
				SendGossipMenuFor(player, 1, me->GetGUID());
			}
			else {
				player->ModifyMoney(upgrade.reqGold * 10000);
				player->SetHonorPoints(player->GetHonorPoints() - upgrade.reqHonor);
				player->SetArenaPoints(player->GetArenaPoints() - upgrade.reqArena);

				for (Instance& req : upgrade.requirements) {
					player->DestroyItemCount(req.itemEntry, req.amount, true);
				}

				if (roll_chance_f(upgrade.success)) {
					// TODO: Create item before adding it to make unique condition check
					player->AddItem(upgradeEntry, 1);
					Item* newItem = player->GetItemByEntry(upgradeEntry);

					ChatHandler(player->GetSession()).PSendSysMessage("%s [%s] -> [%s]", PREFIX,
						item->GetTemplate()->Name1, newItem->GetTemplate()->Name1);

					player->DestroyItemCount(itemEntry, 1, true);
				}
				else {
					ChatHandler(player->GetSession()).PSendSysMessage("%s The upgrade process was unsuccessful.", PREFIX);
				}

				CloseGossipMenuFor(player);
			}
		}

		void handleSelection(Player* player, uint32 action) {
			Item* selectedItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, action - ACTION::SLOTS);

			PlayerData& playerData = data.at(player->GetGUID());

			playerData.selectedItem = selectedItem->GetGUID();

			// should always return an item
			auto it = depository.find(selectedItem->GetTemplate()->ItemId);

			if (it->second.size() == 1) {
				// only one recipe exists
				playerData.selectedRecipe = 0;
				ShowUpgradeInformation(player, selectedItem->GetTemplate()->ItemId, 0, action);
			}
			else {
				ShowRecipes(player, selectedItem->GetTemplate()->ItemId);
			}
		}

		void handleRecipeSelection(Player* player, uint32 action) {
			PlayerData& playerData = data[player->GetGUID()];
			Item* item = player->GetItemByGuid(playerData.selectedItem);

			playerData.selectedRecipe = action - ACTION::RECIPES;

			ShowUpgradeInformation(player, item->GetTemplate()->ItemId, playerData.selectedRecipe, action);
		}

		void AddToBreadcrumbs(std::stack<uint32>& breadcrumbs, Player* player, uint32 action) {
			if (breadcrumbs.empty() || breadcrumbs.top() != action) {
				breadcrumbs.push(action);
			}
		}

		void AddBackItem(Player* player) {
			AddGossipItemFor(player, GOSSIP_ICON_TABARD,
				"|TInterface\\ICONS\\INV_Inscription_Scroll:32:32:-25:1|t Go Back..",
				GOSSIP_SENDER_MAIN, ACTION::BACK);
		}

		void ShowSlots(Player* player, std::vector<uint8> slots, uint32 currAction) {
			for (uint8 slot : slots) {
				Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);

				bool usingRelic = player->GetClass() == CLASS_PALADIN || player->GetClass() == CLASS_DEATH_KNIGHT || player->GetClass() == CLASS_SHAMAN;

				if (item) {
					bool usingShield = item->GetTemplate()->Class == ITEM_CLASS_ARMOR;

					uint32 itemEntry = item->GetTemplate()->ItemId;
					auto it = depository.find(itemEntry);
					if (it != depository.end()) {
						// upgradable
						AddGossipItemFor(player, GOSSIP_ICON_TABARD, GetIconName(slot, usingRelic, usingShield) + GetItemDisplay(it->first), GOSSIP_SENDER_MAIN, ACTION::SLOTS + slot);
					}
					else {
						// OLD ICON: |TInterface\\PaperDollInfoFrame\\UI-GearManager-LeaveItem-Opaque:32:32:-25:1|t
						AddGossipItemFor(player, GOSSIP_ICON_TABARD, GetIconName(slot, usingRelic, usingShield) + " Not Upgradable", GOSSIP_SENDER_MAIN, currAction);
					}
				}
				else {
					bool usingShield = player->GetClass() == CLASS_WARRIOR || player->GetClass() == CLASS_PALADIN || player->GetClass() == CLASS_SHAMAN;

					// OLD ICON: |TInterface\\PaperDoll\\UI-Backpack-EmptySlot:32:32:-25:1|t 
					AddGossipItemFor(player, GOSSIP_ICON_TABARD, GetIconName(slot, usingRelic, usingShield) + " Empty Slot", GOSSIP_SENDER_MAIN, currAction);
				}
			}
		}

		void ShowRecipes(Player* player, uint32 entry) {
			auto recipes = depository[entry];

			uint8 index = 0;
			for (auto recipe : recipes) {
				AddGossipItemFor(player, GOSSIP_ICON_TABARD,
					"|TInterface\\ICONS\\INV_Scroll_03:32:32:-25:1|t " + GetItemDisplay(recipe.first),
					GOSSIP_SENDER_MAIN, ACTION::RECIPES + index);
				index++;
			}
		};

		void ShowUpgradeInformation(Player* player, uint32 entry, uint32 recipeIndex, uint32 currAction) {
			// pair with upgrade_index and Upgrade class.
			auto recipe = depository[entry][recipeIndex];

			Upgrade& upgrade = recipe.second;

			AddGossipItemFor(player, GOSSIP_ICON_TABARD,
				"|TInterface\\ICONS\\Achievement_BG_trueAVshutout:32:32:-25:1|t " + GetItemDisplay(recipe.first),
				GOSSIP_SENDER_MAIN, currAction);

			if (upgrade.reqGold > 0)
				AddGossipItemFor(player, GOSSIP_ICON_TABARD,
					"|TInterface\\ICONS\\INV_Misc_Coin_02:32:32:-25:1|t Gold: " + std::to_string(upgrade.reqGold),
					GOSSIP_SENDER_MAIN, currAction);

			if (upgrade.reqHonor > 0) {
				std::string honorPoints = "|TInterface\\ICONS\\Achievement_PVP_";
				honorPoints.append(player->GetTeamId() == TEAM_ALLIANCE ? "A" : "H");
				honorPoints.append("_16:32:32:-25:1|t Honor Points: " + std::to_string(upgrade.reqHonor));

				AddGossipItemFor(player, GOSSIP_ICON_TABARD, honorPoints, GOSSIP_SENDER_MAIN, currAction);
			}

			if (upgrade.reqArena > 0)
				AddGossipItemFor(player, GOSSIP_ICON_TABARD,
					"|TInterface\\PVPFrame\\PVP-ArenaPoints-Icon:32:32:-25:1|t Arena Points: " + std::to_string(upgrade.reqArena),
					GOSSIP_SENDER_MAIN, currAction);

			for (auto requirement : upgrade.requirements)
				AddGossipItemFor(player, GOSSIP_ICON_TABARD,
					"|TInterface\\ICONS\\INV_Misc_Gear_01:32:32:-25:1|t [" +
					GetItemDisplay(requirement.itemEntry) + "] x" + std::to_string(requirement.amount),
					GOSSIP_SENDER_MAIN, currAction);

			if (upgrade.success < 100) {
				std::stringstream ss;
				ss << std::fixed << std::setprecision(2) << upgrade.success;

				AddGossipItemFor(player, GOSSIP_ICON_TABARD,
					"|TInterface\\ICONS\\INV_Misc_Dice_01:32:32:-25:1|t Success: " + ss.str() + "%",
					GOSSIP_SENDER_MAIN, currAction);
			}

			AddGossipItemFor(player, GOSSIP_ICON_TABARD,
				"|TInterface\\ICONS\\Ability_Paladin_SanctifiedWrath:32:32:-25:1|t Upgrade!",
				GOSSIP_SENDER_MAIN, ACTION::UPGRADE);
		}
	};

	CreatureAI* GetAI(Creature* creature) const override {
		return new item_upgrade_npcAI(creature);
	}
};

void AddSC_item_upgrade_npc() {
	new item_upgrade_npc();
}
