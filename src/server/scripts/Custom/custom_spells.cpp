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

class custom_spells_player : public PlayerScript {
public:
    custom_spells_player() : PlayerScript("custom_spells_player") {
    }

    void OnCreatureKill(Player* killer, Creature* killed) {
        // Skeleton Shot
        if (killed->HasAura(500537)) {
            killer->SummonCreature(23389, killed->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 20s);
        }
        else if (killed->HasAura(500109)) {
            killed->CastSpell(killed->GetPosition(), 500110);
        }

    }

    void OnSpellCast(Player* player, Spell* spell, bool skipCheck) {

        switch (spell->GetSpellInfo()->Id) {
        case 12345:
            break;
        }

    }


};



void AddSC_custom_spells()
{
    new custom_spells_player();
}
