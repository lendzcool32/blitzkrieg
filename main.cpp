#include <Geode/Geode.hpp>
#include "NoclipBridge.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
    if (NoclipBridge::megaHackInstalled()) {
        log::info("[BlitzConsistency] Mega Hack detected - noclip state will be read automatically.");
    } else {
        log::info("[BlitzConsistency] Mega Hack not installed - noclip-mode challenges will always read as noclip-off.");
    }
}

