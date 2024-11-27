#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
//#include <Geode/loader/SettingEvent.hpp>
#include <geode.custom-keybinds/include/Keybinds.hpp>

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
        overlay->runAction(CCTintTo::create(0.5f, overlayColor.r * dimFactor, overlayColor.g * dimFactor, overlayColor.b * dimFactor));
    }
}

void updateOpacityLabel() {
    auto label = static_cast<CCLabelBMFont*>(PlayLayer::get()->getChildByID("opacityLabel"_spr));
    auto opacity = Mod::get()->getSettingValue<int64_t>("set-opacity");
    label->setString(fmt::format("Opacity: {}", opacity).c_str());

    label->stopAllActions();
    auto delay = CCDelayTime::create(0.5f);
    auto fadeIn = CCFadeTo::create(0.25f, 128);
    auto fadeOut = CCFadeTo::create(0.25f, 0);
    auto sequence = CCSequence::create(fadeIn, delay, fadeOut, nullptr);
    label->runAction(sequence);
}

class $modify (dimSettings, PauseLayer) {
    void onSettings(CCObject* sender) {
        PauseLayer::onSettings(sender);
        if (!Mod::get()->getSettingValue<bool>("show-opacity-button")) return;
        auto optionsLayer = typeinfo_cast<GameOptionsLayer*>(CCDirector::get()->getRunningScene()->getChildren()->lastObject());
        if (!optionsLayer) return;
        auto optionsMenu = optionsLayer->m_mainLayer->getChildByType<CCMenu>(1);
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
        auto settings = geode::openSettingsPopup(Mod::get(), false);
        settings->setID("dimSettings"_spr);
    }
};

using namespace keybinds;

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
        auto label = CCLabelBMFont::create("Opacity: ", "bigFont.fnt");
        label->setID("opacityLabel"_spr);
        label->setOpacity(0);
        label->setPosition({4, 310});
        label->setScale(0.3);
        label->setAnchorPoint({0, 0.5});
        this->addChild(label, 1001);
    }

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
            if (event->isDown() && !CCDirector::get()->getRunningScene()->getChildByID("dimSettings"_spr)) {

                auto overlay = static_cast<CCLayerColor*>(PlayLayer::get()->getChildByID("dimOverlay"_spr));
                if (Mod::get()->getSettingValue<bool>("enable-overlay")) 
                    overlay->runAction(CCFadeTo::create(0.5f, 0));
                else
                    overlay->runAction(CCFadeTo::create(0.5f, Mod::get()->getSettingValue<int64_t>("set-opacity") * 2.55));
                Mod::get()->setSettingValue<bool>("enable-overlay", !Mod::get()->getSettingValue<bool>("enable-overlay"));
            }
            return ListenerResult::Propagate;
        }, "toggle-overlay"_spr);

        this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
            if (event->isDown() && !CCDirector::get()->getRunningScene()->getChildByID("dimSettings"_spr)) {
                if (auto pl = PlayLayer::get()) {
                auto overlay = static_cast<CCLayerColor*>(pl->getChildByID("dimOverlay"_spr));
                auto color = Mod::get()->getSettingValue<ccColor3B>("overlay-color");
                Mod::get()->setSettingValue<bool>("rgb", !Mod::get()->getSettingValue<bool>("rgb"));
                }
            }
            return ListenerResult::Propagate;
        }, "toggle-rgb"_spr);

        this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
            if (event->isDown() && !CCDirector::get()->getRunningScene()->getChildByID("dimSettings"_spr)) {
                if (auto enable = Mod::get()->getSettingValue<bool>("enable-overlay")) {
                    auto overlay = static_cast<CCLayerColor*>(PlayLayer::get()->getChildByID("dimOverlay"_spr));
                    auto opacity = Mod::get()->getSettingValue<int64_t>("set-opacity");
                    if (opacity != 100) opacity += 1;  
                    Mod::get()->setSettingValue<int64_t>("set-opacity", opacity);
                    updateOpacityLabel();
                }     
            }
            return ListenerResult::Propagate;
        }, "increase-opacity"_spr);

        this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
            if (event->isDown() && !CCDirector::get()->getRunningScene()->getChildByID("dimSettings"_spr)) {
                if (auto enable = Mod::get()->getSettingValue<bool>("enable-overlay")) {
                    auto overlay = static_cast<CCLayerColor*>(PlayLayer::get()->getChildByID("dimOverlay"_spr));
                    auto opacity = Mod::get()->getSettingValue<int64_t>("set-opacity");
                    if (opacity != 0) opacity -= 1;  
                    Mod::get()->setSettingValue<int64_t>("set-opacity", opacity);
                    updateOpacityLabel();
                }        
            }
            return ListenerResult::Propagate;
        }, "decrease-opacity"_spr);

        return PlayLayer::init(level, useReplay, dontCreateObjects);
    }
};

$execute {
    geode::listenForSettingChanges("set-opacity", +[](int64_t  value) {
        auto pl = PlayLayer::get();
        if (pl && Mod::get()->getSettingValue<bool>("enable-overlay")) {
            auto overlay = static_cast<CCLayerColor*>(pl->getChildByID("dimOverlay"_spr));
            overlay->setOpacity(value * 2.55);
            if (auto enable = Mod::get()->getSettingValue<bool>("rgb")) {
                float speed = abs(Mod::get()->getSettingValue<double>("rgb-speed") - 10);
                if (speed < 0.3) 
                    speed = 0.3;
                createRGB(enable, speed, overlay);
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

    BindManager* bm = BindManager::get();

    bm->registerBindable({
        "toggle-overlay"_spr,
        "Toggle Overlay",
        "Toggles overlay visibility",
        {Keybind::create(KEY_O, Modifier::Alt)},
        "Dim Level"
    });

    bm->registerBindable({
        "toggle-rgb"_spr,
        "Toggle RGB",
        "Toggles the RGB setting.",
        {Keybind::create(KEY_P, Modifier::Alt)},
        "Dim Level"
    });

    bm->registerBindable({
        "increase-opacity"_spr,
        "Increase Opacity",
        "Increases the opacity of the overlay",
        {Keybind::create(KEY_I, Modifier::Alt)},
        "Dim Level"
   });

    bm->registerBindable({
        "decrease-opacity"_spr,
        "Decrease Opacity",
        "Decreases the opacity of the overlay",
        {Keybind::create(KEY_U, Modifier::Alt)},
        "Dim Level"
   });

    bm->setRepeatOptionsFor("toggle-overlay"_spr, { false, 0, 0 });
    bm->setRepeatOptionsFor("toggle-rgb"_spr, { false, 0, 0 });
    bm->setRepeatOptionsFor("increase-opacity"_spr, { true, 20, 450 });
    bm->setRepeatOptionsFor("decrease-opacity"_spr, { true, 20, 450 });
};