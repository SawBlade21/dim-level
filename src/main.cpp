#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <Geode/loader/SettingEvent.hpp>

using namespace geode::prelude;

class $modify (PlayLayer) {
    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        cocos2d::CCSize screenSize = CCDirector::sharedDirector()->getWinSize();
        auto opacity = (Mod::get()->getSettingValue<int64_t>("set-opacity") * 2.55);
        if (!Mod::get()->getSettingValue<bool>("enable-overlay")) opacity = 0;
        auto color = Mod::get()->getSettingValue<ccColor3B>("overlay-color");
        cocos2d::CCLayerColor* overlay = CCLayerColor::create(ccc4(color.r, color.g, color.b, opacity), screenSize.width, screenSize.height);
        overlay->setID("dimOverlay"_spr);
        this->addChild(overlay, 1000);
    }
};

class $modify (dimSettings, PauseLayer) {
    void onSettings(CCObject* sender) {
        PauseLayer::onSettings(sender);
        if (!Mod::get()->getSettingValue<bool>("show-opacity-button")) return;
        auto optionsLayer = typeinfo_cast<GameOptionsLayer*>(CCDirector::get()->getRunningScene()->getChildren()->lastObject());
        if (!optionsLayer) return;
        auto optionsMenu = getChildOfType<CCMenu>(optionsLayer->m_mainLayer, 1);
        auto settingsSprite = CircleButtonSprite::createWithSpriteFrameName(
            "geode.loader/settings.png", 
            1.f, 
             CircleBaseColor::DarkPurple, 
             CircleBaseSize::Small
        );
        settingsSprite->setScale(0.75);
        auto settingsButton = CCMenuItemSpriteExtra::create(settingsSprite, this, menu_selector(dimSettings::openSettings));
        settingsButton->setPosition({146, 119});
        auto text = CCLabelBMFont::create("Dim", "bigFont.fnt");
        text->setScale(0.5);
        text->setPosition({178, 120});
        optionsMenu->addChild(settingsButton);
        optionsMenu->addChild(text);
    }

    void openSettings(CCObject* obj) {
        geode::openSettingsPopup(Mod::get());
    }
};

$execute {
    geode::listenForSettingChanges("set-opacity", +[](int64_t  value) {
        if (auto pl = PlayLayer::get()) {
            auto overlay = static_cast<CCLayerColor*>(pl->getChildByID("dimOverlay"_spr));
            overlay->setOpacity(value * 2.55);
        }
   });
   geode::listenForSettingChanges("overlay-color", +[](ccColor3B value) {
        if (auto pl = PlayLayer::get()) {
            auto overlay = static_cast<CCLayerColor*>(pl->getChildByID("dimOverlay"_spr));
            overlay->setColor(value);
        }
   });
   geode::listenForSettingChanges("enable-overlay", +[](bool  value) {
        if (auto pl = PlayLayer::get()) {
            auto overlay = static_cast<CCLayerColor*>(pl->getChildByID("dimOverlay"_spr));
            if (value == true) 
                overlay->setOpacity(Mod::get()->getSettingValue<int64_t>("set-opacity") * 2.55);
            else
                overlay->setOpacity(0);
        }
   });

};
