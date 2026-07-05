#pragma once
#include "Challenge.hpp"
#include <vector>
#include <algorithm>

using namespace geode::prelude;

class ChallengeManager {
protected:
    std::vector<Challenge> m_challenges;

    ChallengeManager() {
        load();
    }

public:
    static ChallengeManager* get() {
        static ChallengeManager instance;
        return &instance;
    }

    void load() {
        m_challenges.clear();
        auto saved = Mod::get()->getSavedValue<matjson::Value>("challenges", matjson::Value::array());
        if (saved.isArray()) {
            for (auto& v : saved.asArray().unwrapOr(std::vector<matjson::Value>{})) {
                m_challenges.push_back(Challenge::fromJson(v));
            }
        }
    }

    void save() {
        auto arr = matjson::Value::array();
        for (auto& c : m_challenges) {
            arr.push(c.toJson());
        }
        Mod::get()->setSavedValue("challenges", arr);
    }

    std::vector<Challenge*> challengesForLevel(int levelID) {
        std::vector<Challenge*> out;
        for (auto& c : m_challenges) {
            if (c.levelID == levelID) out.push_back(&c);
        }
        return out;
    }

    Challenge& addChallenge(Challenge c) {
        c.id = std::to_string(static_cast<long long>(rand())) + "-" + std::to_string(m_challenges.size());
        m_challenges.push_back(c);
        save();
        return m_challenges.back();
    }

    void removeChallenge(std::string const& id) {
        m_challenges.erase(
            std::remove_if(m_challenges.begin(), m_challenges.end(),
                [&](Challenge const& c) { return c.id == id; }),
            m_challenges.end()
        );
        save();
    }

    // Called once per PlayLayer::resetLevel (i.e. new attempt starting)
    void onAttemptStart(int levelID) {
        for (auto* c : challengesForLevel(levelID)) {
            c->attemptDeaths = 0;
            c->attemptInSegment = false;
            c->attemptFailed = false;
            c->attemptClearedThisPass = false;
        }
    }

    // Called on every percent update. Tracks whether we're inside the
    // segment, and fires a clean-clear the moment percent crosses endPercent
    // without the attempt having failed first.
    void onPercentUpdate(int levelID, float percent, bool noclipActiveDuringAttempt, float currentAccuracy) {
        for (auto* c : challengesForLevel(levelID)) {
            if (c->completed) continue;

            bool inSegment = (percent >= c->startPercent && percent <= c->endPercent);
            c->attemptInSegment = inSegment;

            if (!c->attemptFailed && !c->attemptClearedThisPass && percent > c->endPercent) {
                c->attemptClearedThisPass = true;
                resolveClear(c, noclipActiveDuringAttempt, currentAccuracy);
            }
        }
    }

    void resolveClear(Challenge* c, bool noclipActiveDuringAttempt, float accuracyAtClear) {
        if (c->type == ChallengeType::Consistency) {
            if (noclipActiveDuringAttempt) return; // must be fully clean
            c->currentCount++;
        } else { // NoclipDeathCap
            if (!noclipActiveDuringAttempt) return;
            if (c->attemptDeaths > c->deathCap) return;
            c->currentCount++;
            c->lastAccuracy = accuracyAtClear;
        }
        if (c->currentCount >= c->targetCount) c->completed = true;
        save();
    }

    // Called whenever the player dies
    void onDeath(int levelID, float deathPercent, bool noclipActive) {
        for (auto* c : challengesForLevel(levelID)) {
            if (c->completed) continue;

            bool deathInSegment = (deathPercent >= c->startPercent && deathPercent <= c->endPercent);
            if (!deathInSegment) continue;

            if (c->type == ChallengeType::Consistency) {
                // Any death inside the segment breaks a clean consistency run,
                // regardless of noclip - restart is required next attempt.
                c->attemptFailed = true;
            } else { // NoclipDeathCap
                c->attemptDeaths++;
                if (c->attemptDeaths > c->deathCap) {
                    c->attemptFailed = true;
                }
            }
        }
    }

    // Handles the edge case of a segment ending exactly at 100% (endPercent
    // == 100), which levelComplete() reaches without ever ticking past it
    // in postUpdate. Call this from levelComplete as a safety net.
    void onLevelCompleteSafetyNet(int levelID, bool noclipActiveDuringAttempt, float currentAccuracy) {
        for (auto* c : challengesForLevel(levelID)) {
            if (c->completed || c->attemptFailed || c->attemptClearedThisPass) continue;
            if (c->endPercent < 99.5f) continue; // only relevant for segments ending at 100%
            c->attemptClearedThisPass = true;
            resolveClear(c, noclipActiveDuringAttempt, currentAccuracy);
        }
    }
};
