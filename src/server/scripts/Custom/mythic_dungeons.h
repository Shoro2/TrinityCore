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

enum SPELLS
{
    // percentage buffs
    AURA_5          =   73816,
    AURA_10         =   73818,
    AURA_15         =   73819,
    AURA_20         =   73820,
    AURA_25         =   73821,
    AURA_30         =   73822,

    // extra spells
    SPELL_EXTRA_1   =   4086, // dodge 25%
    SPELL_EXTRA_2   =   10220,  // Chilled
    SPELL_EXTRA_3   =   64036, //5% dmg (per stack)
    SPELL_EXTRA_4   =   1,
};

