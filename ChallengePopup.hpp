#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include "ChallengeManager.hpp"
#include "NoclipBridge.hpp"

using namespace geode::prelude;

class ChallengePopup : public Popup<int> {
protected:
    int m_levelID = 0;
    TextInput* m_startInput = nullptr;
    TextInput* m_endInput = nullptr;
    TextInput* m_deathCapInput = nullptr;
    TextInput* m_countInput = nullptr;
    ChallengeType m_selectedType = ChallengeType::Consistency;
    CCMenu* m_listMenu = nullptr;

    bool setup(int levelID) override {
        m_levelID = levelID;
        this->setTitle("Consistency / Noclip Challenges");

        float y = m_mainLayer->getContentHeight() - 60.f;

        auto startLabel = CCLabelBMFont::create("Start %", "bigFont.fnt");
        startLabel->setScale(0.35f);
        startLabel->setPosition({50, y});
        m_mainLayer->addChild(startLabel);

        m_startInput = TextInput::create(50, "0-100");
        m_startInput->setPosition({50, y - 20});
        m_startInput->setString("25");
        m_startInput->setFilter("0123456789.");
        m_mainLayer->addChild(m_startInput);

        auto endLabel = CCLabelBMFont::create("End %", "bigFont.fnt");
        endLabel->setScale(0.35f);
        endLabel->setPosition({150, y});
        m_mainLayer->addChild(endLabel);

        m_endInput = TextInput::create(50, "0-100");
        m_endInput->setPosition({150, y - 20});
        m_endInput->setString("35");
        m_endInput->setFilter("0123456789.");
        m_mainLayer->addChild(m_endInput);

        auto countLabel = CCLabelBMFont::create("Times", "bigFont.fnt");
        countLabel->setScale(0.35f);
        countLabel->setPosition({250, y});
        m_mainLayer->addChild(countLabel);

        m_countInput = TextInput::create(40, "count");
        m_countInput->setPosition({250, y - 20});
        m_countInput->setString("3");
        m_countInput->setFilter("0123456789");
        m_mainLayer->addChild(m_countInput);

        auto deathCapLabel = CCLabelBMFont::create("Death cap (noclip mode)", "bigFont.fnt");
        deathCapLabel->setScale(0.35f);
        deathCapLabel->setPosition({150, y - 45});
        m_mainLayer->addChild(deathCapLabel);

        m_deathCapInput = TextInput::create(50, "deaths");
        m_deathCapInput->setPosition({280, y - 45});
        m_deathCapInput->setString("40");
        m_deathCapInput->setFilter("0123456789");
        m_mainLayer->addChild(m_deathCapInput);

        auto consistencyBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Consistency (no noclip)", "goldFont.fnt", "GJ_button_01.png", 0.6f),
            this, menu_selector(ChallengePopup::onSelectConsistency)
        );
        auto noclipBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Noclip death-cap run", "goldFont.fnt", "GJ_button_01.png", 0.6f),
            this, menu_selector(ChallengePopup::onSelectNoclip)
        );
        auto typeMenu = CCMenu::create(consistencyBtn, noclipBtn);
        typeMenu->setLayout(RowLayout::create()->setGap(10.f));
        typeMenu->setPosition({m_mainLayer->getContentWidth() / 2, y - 75});
        typeMenu->updateLayout();
        m_mainLayer->addChild(typeMenu);

        auto addBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Add Challenge", "goldFont.fnt", "GJ_button_01.png", 0.7f),
            this, menu_selector(ChallengePopup::onAdd)
        );
        auto addMenu = CCMenu::create(addBtn);
        addMenu->setPosition({m_mainLayer->getContentWidth() / 2, y - 105});
        m_mainLayer->addChild(addMenu);

        auto statusLabel = CCLabelBMFont::create(
            NoclipBridge::megaHackInstalled() ? "Mega Hack detected - noclip auto-tracked" : "Mega Hack not found - noclip mode disabled",
            "chatFont.fnt"
        );
        statusLabel->setScale(0.5f);
        statusLabel->setColor(NoclipBridge::megaHackInstalled() ? ccc3(0, 255, 0) : ccc3(255, 100, 100));
        statusLabel->setPosition({m_mainLayer->getContentWidth() / 2, y - 130});
        m_mainLayer->addChild(statusLabel);

        refreshList();
        return true;
    }

    void onSelectConsistency(CCObject*) { m_selectedType = ChallengeType::Consistency; }
    void onSelectNoclip(CCObject*) { m_selectedType = ChallengeType::NoclipDeathCap; }

    void onAdd(CCObject*) {
        Challenge c;
        c.levelID = m_levelID;
        c.startPercent = numFromCCString(m_startInput->getString(), 25.f);
        c.endPercent = numFromCCString(m_endInput->getString(), 100.f);
        c.type = m_selectedType;
        c.deathCap = static_cast<int>(numFromCCString(m_deathCapInput->getString(), 40.f));
        c.targetCount = static_cast<int>(numFromCCString(m_countInput->getString(), 1.f));

        if (c.endPercent <= c.startPercent) {
            FLAlertLayer::create("Error", "End % must be greater than start %.", "OK")->show();
            return;
        }

        ChallengeManager::get()->addChallenge(c);
        refreshList();
    }

    static float numFromCCString(std::string const& s, float fallback) {
        if (s.empty()) return fallback;
        try { return std::stof(s); } catch (...) { return fallback; }
    }

    void refreshList() {
        if (m_listMenu) {
            m_listMenu->removeFromParent();
            m_listMenu = nullptr;
        }

        auto challenges = ChallengeManager::get()->challengesForLevel(m_levelID);
        m_listMenu = CCMenu::create();
        m_listMenu->setPosition({20, 150});
        m_listMenu->setAnchorPoint({0, 1});
        m_listMenu->setContentSize({m_mainLayer->getContentWidth() - 40, 140});
        m_listMenu->setLayout(
            ColumnLayout::create()->setAxisAlignment(AxisAlignment::End)->setGap(6.f)
        );

        for (auto* c : challenges) {
            std::string typeStr = c->type == ChallengeType::Consistency
                ? "Consistency"
                : fmt::format("Noclip (<{} deaths)", c->deathCap);

            std::string status = c->completed ? "DONE" : fmt::format("{}/{}", c->currentCount, c->targetCount);

            std::string accStr;
            if (c->type == ChallengeType::NoclipDeathCap && c->lastAccuracy >= 0.f) {
                accStr = fmt::format(" | acc {:.1f}%", c->lastAccuracy);
            }

            auto label = CCLabelBMFont::create(
                fmt::format("{:.0f}-{:.0f}% | {} | {}{}", c->startPercent, c->endPercent, typeStr, status, accStr).c_str(),
                "chatFont.fnt"
            );
            label->setScale(0.6f);
            m_listMenu->addChild(label);
        }

        m_listMenu->updateLayout();
        m_mainLayer->addChild(m_listMenu);
    }

public:
    static ChallengePopup* create(int levelID) {
        auto ret = new ChallengePopup();
        if (ret->initAnchored(420.f, 280.f, levelID, "GJ_square01.png")) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
