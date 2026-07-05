#include <Geode/modify/PauseLayer.hpp>
#include "ChallengePopup.hpp"

using namespace geode::prelude;

class $modify(BCPauseLayer, PauseLayer) {
    struct Fields {
        int m_levelID = 0;
    };

    void customSetup() {
        PauseLayer::customSetup();

        auto level = PlayLayer::get() ? PlayLayer::get()->m_level : nullptr;
        int levelID = level ? level->m_levelID.value() : 0;

        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("BC", "goldFont.fnt", "GJ_button_04.png", 0.8f),
            this, menu_selector(BCPauseLayer::onOpenChallenges)
        );
        btn->setID("blitz-consistency-button"_spr);

        if (auto menu = this->getChildByID("right-button-menu")) {
            menu->addChild(btn);
            menu->updateLayout();
        } else if (auto center = this->getChildByID("center-button-menu")) {
            center->addChild(btn);
            center->updateLayout();
        }

        m_fields->m_levelID = levelID;
    }

    void onOpenChallenges(CCObject*) {
        ChallengePopup::create(m_fields->m_levelID)->show();
    }
};
