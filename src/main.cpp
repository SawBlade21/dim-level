#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <Geode/loader/SettingEvent.hpp>

using namespace geode::prelude;

float getDimFactor() {
    float dimFactor = abs((Mod::get()->getSettingValue<int64_t>("set-opacity") / 100.f) - 1);
    dimFactor = static_cast<int>(dimFactor * 100.0f) / 100.0f;
    if (dimFactor == 0) 
        dimFactor = 0.01f;
    return dimFactor;
}

void createRGB(bool enable, float speed, CCLayerColor* overlay) {
    float dimFactor = getDimFactor();
    if (enable == true) {
        overlay->stopAllActions();
        auto redToOrange = CCTintTo::create(speed, 255 * dimFactor, 165 * dimFactor, 0); 
        auto orangeToYellow = CCTintTo::create(speed, 255 * dimFactor, 255 * dimFactor, 0); 
        auto yellowToGreen = CCTintTo::create(speed, 0, 255 * dimFactor, 0);    
        auto greenToBlue = CCTintTo::create(speed, 0, 0, 255 * dimFactor);      
        auto blueToIndigo = CCTintTo::create(speed, 75 * dimFactor, 0, 130 * dimFactor);    
        auto indigoToViolet = CCTintTo::create(speed, 238 * dimFactor, 130 * dimFactor, 238 * dimFactor);
        auto violetToRed = CCTintTo::create(speed, 255 * dimFactor, 0, 0); 
        auto rainbowSequence = CCSequence::create(
            redToOrange, 
            orangeToYellow, 
            yellowToGreen, 
            greenToBlue, 
            blueToIndigo, 
            indigoToViolet, 
            violetToRed, 
            nullptr
        );
        overlay->runAction(CCRepeatForever::create(rainbowSequence));
    }
    else {
        overlay->stopAllActions();
        auto overlayColor = Mod::get()->getSettingValue<ccColor3B>("overlay-color");
        overlayColor.r *= dimFactor;
        overlayColor.g *= dimFactor;
        overlayColor.b *= dimFactor;
        overlay->setColor(overlayColor);
    }
}

class $modify (PlayLayer) {
    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        cocos2d::CCSize screenSize = CCDirector::sharedDirector()->getWinSize();
        auto opacity = (Mod::get()->getSettingValue<int64_t>("set-opacity") * 2.55);
        if (!Mod::get()->getSettingValue<bool>("enable-overlay")) opacity = 0;
        auto color = Mod::get()->getSettingValue<ccColor3B>("overlay-color");
        float dimFactor = getDimFactor();
        cocos2d::CCLayerColor* overlay = CCLayerColor::create(ccc4(color.r * dimFactor, color.g * dimFactor, color.b * dimFactor, opacity), screenSize.width, screenSize.height);
        overlay->setID("dimOverlay"_spr);
        this->addChild(overlay, 1000);
        if (auto enable = Mod::get()->getSettingValue<bool>("rgb")) {
            float speed = abs(Mod::get()->getSettingValue<double>("rgb-speed") - 10);
            if (speed < 0.3) 
                speed = 0.3;
            createRGB(enable, speed, overlay);
        }
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
                if (auto enable = Mod::get()->getSettingValue<bool>("rgb")) {
                    float speed = abs(Mod::get()->getSettingValue<double>("rgb-speed") - 10);
                    if (speed < 0.3) 
                        speed = 0.3;
                    createRGB(enable, speed, overlay);
                }
            }
        }
   });
   geode::listenForSettingChanges("overlay-color", +[](ccColor3B value) {
        if (auto pl = PlayLayer::get()) {
            auto overlay = static_cast<CCLayerColor*>(pl->getChildByID("dimOverlay"_spr));
            float dimFactor = getDimFactor();
            value.r *= dimFactor;
            value.g *= dimFactor;
            value.b *= dimFactor;
            overlay->setColor(value);
        }
   });
   geode::listenForSettingChanges("enable-overlay", +[](bool value) {
        if (auto pl = PlayLayer::get()) {
            auto overlay = static_cast<CCLayerColor*>(pl->getChildByID("dimOverlay"_spr));
            if (value == true) 
                overlay->setOpacity(Mod::get()->getSettingValue<int64_t>("set-opacity") * 2.55);
            else
                overlay->setOpacity(0);
        }
   });
   geode::listenForSettingChanges("rgb", +[](bool value) {
        if (auto pl = PlayLayer::get()) {
            auto overlay = static_cast<CCLayerColor*>(pl->getChildByID("dimOverlay"_spr));
            float speed = abs(Mod::get()->getSettingValue<double>("rgb-speed") - 10);
            if (speed < 0.3) 
                speed = 0.3;
            createRGB(value, speed, overlay);
        }
   });
   geode::listenForSettingChanges("rgb-speed", +[](double  value) {
    if (auto pl = PlayLayer::get()) {
            auto overlay = static_cast<CCLayerColor*>(pl->getChildByID("dimOverlay"_spr));
            auto enable = Mod::get()->getSettingValue<bool>("rgb");
            float speed = (abs(value - 10));
            if (speed < 0.3) 
                speed = 0.3;
            if (enable) createRGB(enable, speed, overlay);
        }
   });
};


