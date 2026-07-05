#pragma once
#include <Geode/Geode.hpp>
#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;

// Mega Hack v8 is closed-source, so there's no header to hook directly.
// Instead, this reads its live toggle state through Geode's public
// cross-mod settings API: Loader::get()->getLoadedMod(id)->getSettingValue<T>(key).
//
// CAVEAT: the setting keys below ("noclip", "noclip-accuracy") are best-guess
// based on Mega Hack's public naming conventions. If they're wrong, this will
// silently fall back to false/0 rather than crash - check the log on startup
// (search for "[BlitzConsistency]") to see what keys Mega Hack actually
// exposes, and adjust MEGAHACK_ID / the key strings below to match.
class NoclipBridge {
    static constexpr const char* MEGAHACK_ID = "absolllute.megahack";

    static Mod* megahack() {
        return Loader::get()->getLoadedMod(MEGAHACK_ID);
    }

public:
    static bool isNoclipActive() {
        auto* mh = megahack();
        if (!mh) return false;

        if (!mh->hasSetting("noclip")) {
            log::warn("[BlitzConsistency] Mega Hack loaded, but no 'noclip' setting found - update the key in NoclipBridge.hpp");
            return false;
        }
        return mh->getSettingValue<bool>("noclip");
    }

    static float noclipAccuracy() {
        auto* mh = megahack();
        if (!mh) return -1.f;

        if (!mh->hasSetting("noclip-accuracy")) {
            return -1.f;
        }
        return static_cast<float>(mh->getSettingValue<double>("noclip-accuracy"));
    }

    static bool megaHackInstalled() {
        return megahack() != nullptr;
    }
};
