#include "Weapon.h"

Weapon::Weapon()
{
}

Weapon::~Weapon()
{
}

void Weapon::ApplyRecoil()
{
    Vector3 weaponPos = this->GetPhysicsObject()->BTGetPosition();

    //后坐力偏移，取决于recoilStrength
    Vector3 recoilOffset = -weaponLookDir * recoilStrength;

    //std::cout << weaponPos << std::endl;
    this->GetPhysicsObject()->BTSetPosition(weaponPos + recoilOffset);
    //std::cout << this->GetPhysicsObject()->BTGetPosition() << std::endl;

    //设置恢复时间
    recoilTimer = recoilRecoveryTime;
}

