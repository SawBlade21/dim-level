#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <Geode/loader/SettingEvent.hpp>

using namespace geode::prelude;

void startRGB(PlayLayer* pl) {
    if (auto overlay = pl->getChildByID("dimOverlay"_spr)) {
        float speed = abs(Mod::get()->getSettingValue<double>("rgb-speed") - 3.f);

        if (speed == 0.f)
            speed = 0.01f;

        auto red = CCTintTo::create(speed, 255, 0, 0);
        auto green = CCTintTo::create(speed, 0, 255, 0);
        auto blue = CCTintTo::create(speed, 0, 0, 255);

        auto colorSequence = CCSequence::create(red, green, blue, nullptr);
        auto repeatSequence = CCRepeatForever::create(colorSequence);

        overlay->runAction(repeatSequence);
    }
}

void stopRGB(PlayLayer* pl) {
    if (auto overlay = pl->getChildByID("dimOverlay"_spr)) {
        overlay->stopAllActions();
        static_cast<CCLayerColor*>(overlay)->setColor(Mod::get()->getSettingValue<cocos2d::ccColor3B>("overlay-color"));
    }
}

class $modify (PlayLayer) {

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        cocos2d::CCSize screenSize = CCDirector::sharedDirector()->getWinSize();
        auto opacity = (Mod::get()->getSettingValue<int64_t>("set-opacity") * 2.55);
        auto color = Mod::get()->getSettingValue<ccColor3B>("overlay-color");
        cocos2d::CCLayerColor* overlay = CCLayerColor::create(ccc4(color.r, color.g, color.b, opacity), screenSize.width, screenSize.height);
        overlay->setID("dimOverlay"_spr);

        if (!Mod::get()->getSettingValue<bool>("enable-overlay"))
            overlay->setVisible(false);

        addChild(overlay, 1000);

        if (Mod::get()->getSettingValue<bool>("overlay-rgb"))
            startRGB(this);
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
            if (Mod::get()->getSettingValue<bool>("enable-overlay")) {
            auto overlay = static_cast<CCLayerColor*>(pl->getChildByID("dimOverlay"_spr));
            overlay->setOpacity(value * 2.55);
            }
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
            if (auto overlay = static_cast<CCLayerColor*>(pl->getChildByID("dimOverlay"_spr)))
                overlay->setVisible(value);
        }
   });
   geode::listenForSettingChanges("overlay-rgb", +[](bool  value) {
        if (auto pl = PlayLayer::get())
            value ? startRGB(pl) : stopRGB(pl);
   });
   geode::listenForSettingChanges("rgb-speed", +[](double  value) {
        if (auto pl = PlayLayer::get()) {
            stopRGB(pl);
            startRGB(pl);
        }
   });
};
