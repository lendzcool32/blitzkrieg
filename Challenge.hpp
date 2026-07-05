#pragma once
#include <Geode/Geode.hpp>
#include <matjson.hpp>

using namespace geode::prelude;

enum class ChallengeType : int {
    // Do the segment N times without noclip enabled at all during the attempt
    Consistency = 0,
    // Do the segment N times, each attempt using noclip, but staying under
    // a max-death cap within that attempt
    NoclipDeathCap = 1
};

struct Challenge {
    std::string id;              // uuid-ish, generated on creation
    int levelID = 0;
    float startPercent = 0.f;
    float endPercent = 100.f;
    ChallengeType type = ChallengeType::Consistency;

    // Only used for NoclipDeathCap
    int deathCap = 40;

    int targetCount = 1;         // successes needed to finish the challenge
    int currentCount = 0;        // successes so far
    int attemptDeaths = 0;       // deaths accumulated in the CURRENT attempt
    float lastAccuracy = -1.f;   // Mega Hack's noclip-accuracy reading from the last successful attempt
    bool completed = false;

    // Live/in-progress attempt state (not persisted, reset per level session)
    bool attemptInSegment = false;
    bool attemptFailed = false;        // set true if a disallowed noclip use is detected mid-consistency-run
    bool attemptClearedThisPass = false; // prevents double-counting once we've crossed endPercent this attempt

    matjson::Value toJson() const {
        matjson::Value v = matjson::Value::object();
        v["id"] = id;
        v["levelID"] = levelID;
        v["startPercent"] = startPercent;
        v["endPercent"] = endPercent;
        v["type"] = static_cast<int>(type);
        v["deathCap"] = deathCap;
        v["targetCount"] = targetCount;
        v["currentCount"] = currentCount;
        v["lastAccuracy"] = lastAccuracy;
        v["completed"] = completed;
        return v;
    }

    static Challenge fromJson(matjson::Value const& v) {
        Challenge c;
        c.id = v["id"].asString().unwrapOr("");
        c.levelID = v["levelID"].asInt().unwrapOr(0);
        c.startPercent = static_cast<float>(v["startPercent"].asDouble().unwrapOr(0.0));
        c.endPercent = static_cast<float>(v["endPercent"].asDouble().unwrapOr(100.0));
        c.type = static_cast<ChallengeType>(v["type"].asInt().unwrapOr(0));
        c.deathCap = v["deathCap"].asInt().unwrapOr(40);
        c.targetCount = v["targetCount"].asInt().unwrapOr(1);
        c.currentCount = v["currentCount"].asInt().unwrapOr(0);
        c.lastAccuracy = static_cast<float>(v["lastAccuracy"].asDouble().unwrapOr(-1.0));
        c.completed = v["completed"].asBool().unwrapOr(false);
        return c;
    }
};
