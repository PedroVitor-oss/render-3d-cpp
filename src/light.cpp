#include "light.h"

float GlobalLight::GetIntensity()
{
    return this->intensity;
}
Vector3 GlobalLight::GetDir() const
{
    return this->dir;
}