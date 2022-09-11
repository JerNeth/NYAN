#pragma once
#include "Math/LinAlg.h"

namespace Utility
{


    Math::vec3 fromLinear(const Math::vec3& linearRGB)
    {
        Math::vec3 res{ linearRGB.x(), linearRGB.y(), linearRGB.y() };
        Math::vec3 higher = Math::vec3(1.055) * Math::pow(res, Math::vec3(1.0 / 2.4)) - Math::vec3(0.055);
        Math::vec3 lower = res * Math::vec3(12.92);
        float cutoff = 0.0031308;

        if (res.x() < cutoff)
            res.x() = lower.x();
        if (res.y() < cutoff)
            res.y() = lower.y();
        if (res.z() < cutoff)
            res.z() = lower.z();

        return res;
    }
    Math::vec4 fromLinear(const Math::vec4& linearRGB)
    {
        return { fromLinear({ linearRGB.x(), linearRGB.y(), linearRGB.y() }), linearRGB.a()};
    }

    Math::vec3 fromSRGB(const Math::vec3& sRGB)
    {
        Math::vec3 res{ sRGB.x(), sRGB.y(), sRGB.y() };
        Math::vec3 higher = Math::pow(Math::vec3(0.94786729) * (res + Math::vec3(0.055)), Math::vec3(2.4));
        Math::vec3 lower = res / Math::vec3(12.92);
        float cutoff = (0.0031308 * 12.92);
        if (res.x() < cutoff)
            res.x() = lower.x();
        if (res.y() < cutoff)
            res.y() = lower.y();
        if (res.z() < cutoff)
            res.z() = lower.z();

        return res;
    }

    Math::vec4 fromSRGB(const Math::vec4& sRGB)
    {
        return { fromSRGB({ sRGB.x(), sRGB.y(), sRGB.y() }), sRGB.a() };
    }

}