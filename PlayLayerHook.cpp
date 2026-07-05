#include <Geode/modify/PlayLayer.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include "ChallengeManager.hpp"
#include "NoclipBridge.hpp"

using namespace geode::prelude;

class $modify(BCPlayLayer, PlayLayer) {
    struct Fields {
        bool m_usedNoclipThisAttempt = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        auto lvlID = level->m_levelID.value();
        ChallengeManager::get()->onAttemptStart(lvlID);
        m_fields->m_usedNoclipThisAttempt = NoclipBridge::isNoclipActive();

        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        auto lvlID = m_level->m_levelID.value();
        ChallengeManager::get()->onAttemptStart(lvlID);
        m_fields->m_usedNoclipThisAttempt = NoclipBridge::isNoclipActive();
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // Track death BEFORE calling original, since noclip mods often
        // no-op the original call (which is exactly what tells us noclip
        // was active for this hazard).
        auto lvlID = m_level->m_levelID.value();
        float percent = this->getCurrentPercent();
        bool noclipNow = NoclipBridge::isNoclipActive();

        if (noclipNow) {
            m_fields->m_usedNoclipThisAttempt = true;
        }

        ChallengeManager::get()->onDeath(lvlID, percent, noclipNow);
        ChallengeManager::get()->onPercentUpdate(lvlID, percent, m_fields->m_usedNoclipThisAttempt, NoclipBridge::noclipAccuracy());

        PlayLayer::destroyPlayer(player, object);
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        // If noclip flips on mid-attempt, remember it for the whole attempt
        if (NoclipBridge::isNoclipActive()) {
            m_fields->m_usedNoclipThisAttempt = true;
        }

        auto lvlID = m_level->m_levelID.value();
        float percent = this->getCurrentPercent();
        ChallengeManager::get()->onPercentUpdate(lvlID, percent, m_fields->m_usedNoclipThisAttempt, NoclipBridge::noclipAccuracy());
    }

    void levelComplete() {
        auto lvlID = m_level->m_levelID.value();
        ChallengeManager::get()->onLevelCompleteSafetyNet(lvlID, m_fields->m_usedNoclipThisAttempt, NoclipBridge::noclipAccuracy());

        PlayLayer::levelComplete();
    }
};
