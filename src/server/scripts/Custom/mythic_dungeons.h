#include "Define.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "Unit.h"
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

enum SPELLS
{
    // Info Buff
    BUFF_INFO_NPC       =       500100,
    BUFF_INFO_PLAYER    =       500105,

    // percentage buffs
    AURA_5              =       73816,
    AURA_10             =       73818,
    AURA_15             =       73819,
    AURA_20             =       73820,
    AURA_25             =       73821,
    AURA_30             =       73822,

    AURA_DMG_5          =       500101, //5% dmg (per stack)

    // extra spells
    SPELL_EXTRA_1       =       500102, // dodge 25%
    SPELL_EXTRA_2       =       500103,  // Ice Armor
    
    SPELL_EXTRA_4       =       1,
};

