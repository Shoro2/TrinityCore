#include "ScriptPCH.h"

enum AreaSpeed
{

    AREA_MALL = 5303, // add here your area id

};

class area_speed : public PlayerScript
{
public:
    area_speed() : PlayerScript("area_speed") {}

    void OnUpdateZone(Player* player, uint32 newZone, uint32 newArea)
    {
        switch (newArea)
        {

        case AREA_MALL:
            player->SetSpeed(MOVE_RUN, 2.0f);

            break;


        }
    }
};

void AddSC_area_speed()
{
    new area_speed();
}