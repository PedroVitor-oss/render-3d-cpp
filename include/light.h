#ifndef LIGHT_H
#define LIGHT_H
#include "raylib.h"
#include <iostream>
#include <cmath>
class GlobalLight
{
    Vector3 dir;
    Color cl;
    float intensity;

public:
    GlobalLight(Vector3 lightdir, Color cl, float intenxity)
    {
        // normalização de vetor direção
        float mag = std::sqrt(lightdir.x * lightdir.x + lightdir.y * lightdir.y + lightdir.z * lightdir.z);
        this->dir = {
            lightdir.x / mag,
            lightdir.y / mag,
            lightdir.z / mag};
        this->cl = cl;
        this->intensity = intenxity;
    }
    float GetIntensity();
    Vector3 GetDir() const;
};

#endif