#include "Define.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "spell.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include <iostream>
#include <string>
#include <Chat.h>

class custom_spells : public PlayerScript {
public:
    custom_spells() : PlayerScript("custom_spells_player") {
    }

    void OnCreatureKill(Player* killer, Creature* killed) {
        // Skeleton Shot
        if (killed->HasAura(500537)) {
            killer->CastSpell(killed->GetPosition(), 500539);
        }

    }

    void OnSpellCast(Player* player, Spell* spell, bool skipCheck) {

        switch (spell->GetSpellInfo()->Id) {
        case 12345:
            break;
        }

    }


};



void AddSC_mythic_dungeons()
{
    new custom_spells();
}
