#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/EditLevelLayer.hpp>

/**
 * !KNOWN QUIRKS ATM:
 * alpha trigger is the only trigger that has an inspector panel rn LMAOOOOO
 * ui only looks normal when the screen size is 1360x768 or any multiple
 * has not been tested on windows yet, it should be fine :clueless:
 * its probably terrible on mobile
 * and not compatible with most editor mods probably
 * OH YEAH AND i think it requires nodeids, but my stupid mac just throws fit when i try and add a requirement
 * 
 * TODOS:
 * figure out how to structure the code properly LMAO
 * make dynamic inspector properties. ex. multitrigger checkbox appears only in spawn and touch mode
 * make general dropdown method
 * object menu catergory dropdown/remove object tabs
 * scrollable object menu
 * reshape swipe, free move, etc. into horizontal tabs below the top bar (add build helper and other stuff too)
 * add group tab on inspector
 * add special tab on inspector
 * add level name on top bar
 * remove edit group, special, and object buttons
 * reformat delete mode buttons to sidebar
 * overhaul editor pause menu also
 * add inspector input types:
 *      dropdown
 *      gallery (ex. startpos gamemode/speed)
 *      color wheel
 *      slider
 *      image (ex. change bg)
 *      music
 *      sfx
 *      text (for text objects)
 *      number arrays (object groups)
 *      dual number arrays (ex. advrand, spawn remapping)
 *      "extended" numbers (ex. for when the input can be smth like P1, BG, etc.) (they'll just have a lil + beside them)
 *      misc (like advfollow presets, keyframe button doohickeys, equation thing in item edit, layer range in shader)
 */

using namespace geode::prelude;

EditorUI* newEditorUI;
auto levelName = "nope";
std::vector<std::tuple<int, std::string, std::vector<int>>> objectInsProp = {
    {1007, "Alpha", {1, 2, 3, 0, 4, 5}}
};

std::string floatToFormattedString(float num, int round) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(round) << num;
    std::string string = ss.str();
    string.erase(string.find_last_not_of('0') + 1, std::string::npos);
    if (string.back() == '.') string.pop_back();
    return string;
}

void createInspectorList(CCArray* items) {
    if (newEditorUI->getChildByID("inspector-list")) newEditorUI->getChildByID("inspector-list")->removeMeAndCleanup();

    auto list = ListView::create(
        items,
        9.0f,
        109.0f,
        194.0f
    );
    list->setPosition({ 451.5, 103.5});
    list->setScale(1);
    list->setZOrder(1);
    list->setTouchEnabled(true);
    list->setTouchPriority(-1);
    list->setCellOpacity(50);
    list->setPrimaryCellColor(cocos2d::ccColor3B(0, 0, 10));
    list->setSecondaryCellColor(cocos2d::ccColor3B(50, 50, 60));
    list->setCellBorderColor(cocos2d::ccColor4B(0, 0, 0, 0));
    list->setID("inspector-list");
    newEditorUI->addChild(list);
}

TextInput* createNumberField() {
    auto field = TextInput::create(60, "num");
    field->getInputNode()->setScale(1.7);
    field->getInputNode()->setPositionX(9);
    field->getInputNode()->setAnchorPoint({ 0.f, 0.f });
    field->getChildByType(0)->setContentSize(CCSize(230, 60));
    field->getChildByType(0)->setPositionX(6);
    field->setPosition(ccp(107.5, 1));
    field->setAnchorPoint({ 1.f, 0.f });
    field->setScale(0.225);
    return field;
}

class InspectorInput : public CCLayer {
    public:
    void onDropdownChange(CCObject* sender) {

        sender->getObjType();
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        btn->setSprite(ButtonSprite::create("joe"));
        btn->updateSprite();
        auto thing = static_cast<EffectGameObject*>(sender)->getUserObject();
        auto obj = static_cast<EffectGameObject*>(thing);

        switch (sender->getTag()) {
            case 0:
                static int state0 = 0;
                state0 = (state0 + 1) % 3;
                bool spawn = false;
                bool touch = false;
                if (state0 == 0) {
                    spawn = true;
                    touch = false;
                    btn->setSprite(ButtonSprite::create("Spawn"));
                    btn->updateSprite();
                } else if (state0 == 1) {
                    spawn = false;
                    touch = true;
                    btn->setSprite(ButtonSprite::create("Touch"));
                    btn->updateSprite();
                } else if (state0 == 2) {
                    spawn = false;
                    touch = false;
                    btn->setSprite(ButtonSprite::create("Time"));
                    btn->updateSprite();
                }
                
                obj->m_isSpawnTriggered = spawn;
                obj->m_isTouchTriggered = touch;
                break;
        }
    }

    void onToggleChange(CCObject* sender) {
        auto thing = static_cast<EffectGameObject*>(sender)->getUserObject();
        auto obj = static_cast<EffectGameObject*>(thing);

        switch (sender->getTag()) {
            case 0:
                obj->m_isMultiTriggered = !obj->m_isMultiTriggered;
                break;
        }
    }
};

CCMenu* createDropdownField(EffectGameObject* obj, int dropdownID) {
    auto spr = ButtonSprite::create("joe");    

    if (dropdownID == 0) {
        if (obj->m_isSpawnTriggered) {
            spr = ButtonSprite::create("Spawn");
        } else if (obj->m_isTouchTriggered) {
            spr = ButtonSprite::create("Touch");
        } else {
            spr = ButtonSprite::create("Time");
        }
    }
    
    auto btn = CCMenuItemSpriteExtra::create(
        spr, newEditorUI, menu_selector(InspectorInput::onDropdownChange)
    );
    auto menu = CCMenu::create();
    btn->setAnchorPoint({ 1.f, 0.5f });
    btn->setUserObject(obj);
    btn->setTag(dropdownID);
    menu->addChild(btn);
    menu->setScale(0.275);
    menu->setPosition({-97.5f, -111.5f});
    return menu;
}

CCMenu* createCheckboxField(EffectGameObject* obj, int checkboxID) {
    auto toggler = CCMenuItemToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"), 
        newEditorUI, 
        menu_selector(InspectorInput::onToggleChange)
    );
    switch (checkboxID) {
        case 0:
            toggler->toggle(obj->m_isMultiTriggered);
            break;
    }
    toggler->setUserObject(obj);
    toggler->setTag(checkboxID);
    auto menu = CCMenu::create();
    menu->addChild(toggler);
    menu->setScale(0.25);
    menu->setPosition({-108.5f, -115.5f});
    return menu;
}

void createInspectorElements(CCArray* list, EffectGameObject* obj, std::vector<int> propertyIDs) {
    std::string name = "hey";
    for (int i = 0; i < propertyIDs.size(); i++) {

        auto usesNumberField = true;
        auto numberField = createNumberField();
        auto usesDropdownField = false;
        auto dropdownField = createDropdownField(obj, 0);
        auto usesCheckboxField = false;
        auto checkboxField = createCheckboxField(obj, 0);

        switch (propertyIDs[i]) {
            case 0:
                name = " ";
                usesNumberField = false;
                break;
            case 1:
                name = "Target Group ID";
                numberField = createNumberField();
                numberField->setMaxCharCount(4);
                numberField->setString(std::to_string(obj->m_targetGroupID).c_str(), false);
                numberField->setCallback([numberField, obj] (const std::string& input) {
                    if (!input.empty()) {
                        obj->m_targetGroupID = std::stoi(input);
                        obj->getObjectLabel()->setString(std::to_string(std::stoi(input)).c_str());
                    }
                });
                break;
            case 2:
                name = "Fade Duration";
                numberField = createNumberField();
                numberField->setFilter("1234567890.");
                numberField->setString(floatToFormattedString(obj->m_duration, 2).c_str(), false);
                numberField->setCallback([obj] (const std::string& input) {
                    if (input.find_first_of("1234567890") != std::string::npos) obj->m_duration = std::stof(input);
                });
                break;
            case 3:
                name = "Opacity";
                numberField = createNumberField();
                numberField->setFilter("1234567890.");
                numberField->setString(floatToFormattedString(obj->m_opacity, 2).c_str(), false);
                numberField->setCallback([obj] (const std::string& input) {
                    if (input.find_first_of("1234567890") != std::string::npos) {
                        if (std::stof(input) <= 1) obj->m_opacity = std::stof(input);
                    }
                });
                break;
            case 4:
                name = "Trigger Type";
                usesNumberField = false;
                usesDropdownField = true;
                dropdownField = createDropdownField(obj, 0);
                break;
            case 5:
                name = "Multi Trigger";
                usesNumberField = false;
                usesCheckboxField = true;
                checkboxField = createCheckboxField(obj, 0);
                break;
        }

        auto label = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
        label->setAnchorPoint({ 0.f, 0.f });
        label->setPosition(ccp(1.5, 1.5));
        label->setScale(0.2);

        
        
        
        
        auto wrapper = CCNode::create();
        wrapper->addChild(label);
        if (usesNumberField) wrapper->addChild(numberField);
        if (usesDropdownField) wrapper->addChild(dropdownField);
        if (usesCheckboxField) wrapper->addChild(checkboxField);
        wrapper->setContentSize(label->getContentSize());
        
        list->addObject(wrapper);
    }
}

class SwallowNode : public CCNode, public CCTargetedTouchDelegate {
public:
    static SwallowNode* create() {
        auto ret = new SwallowNode();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init() override {
        CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -100, true);
        return true;
    }

    // Touch handling (swallow touch events)
    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
        auto location = touch->getLocation();
        if (this->boundingBox().containsPoint(location)) {
            return true; // Eat the touch
        }
        return false;
    }
};

class $modify(Editor, LevelEditorLayer) {
    void selectObject(GameObject* object) {
        auto obj = static_cast<EffectGameObject*>(object);
        auto winSize = CCDirector::get()->getWinSize();




        auto items = CCArray::create();
        std::vector<int> propertyIDs = {0};
        auto objectName = "Joe H. Bruh";

        for (int i = 0; i < objectInsProp.size(); i++) {
            int targetObjectID = std::get<0>(objectInsProp[i]);
            if (obj->m_objectID == targetObjectID) {
                std::string name = std::get<1>(objectInsProp[i]);
                std::vector<int> values = std::get<2>(objectInsProp[i]);
                
                propertyIDs = values;
                objectName = name.c_str();
            } else {
                propertyIDs = {0};
                objectName = "Work In Progress";
            }
        }
        
        createInspectorElements(items, obj, propertyIDs);
        createInspectorList(items);

        if (newEditorUI->getChildByID("inspector-title")) newEditorUI->getChildByID("inspector-title")->removeMeAndCleanup();
        auto inspectorTitle = CCLabelBMFont::create(objectName, "goldFont.fnt");
        inspectorTitle->setID("inspector-title");
        inspectorTitle->setPosition(winSize / 2);
        inspectorTitle->setScale(0.425);
        inspectorTitle->setPosition(ccp(453, 306));
        inspectorTitle->setAnchorPoint(ccp(0, 0.5));
        newEditorUI->addChild(inspectorTitle);



        if (this->getChildByID("testing-label")) {
            this->getChildByID("testing-label")->removeMeAndCleanup();
        }
        auto labelnew = CCLabelBMFont::create(std::to_string(obj->m_targetGroupID).c_str(), "bigFont.fnt");
        labelnew->setID("testing-label");
        labelnew->setPosition(winSize / 2);
		//this->addChild(labelnew);

    }

    void deselectObject() {
        if (newEditorUI->getChildByID("inspector-list")) newEditorUI->getChildByID("inspector-list")->removeMeAndCleanup();
        if (newEditorUI->getChildByID("inspector-title")) newEditorUI->getChildByID("inspector-title")->removeMeAndCleanup();
    }
};

class $modify(EditUI, EditorUI) {
    struct Fields {
        GameObject* selectedObjectCache; // ensure it selects the right object
    };

    void selectObject(GameObject* p0, bool p1) {
        EditorUI::selectObject(p0, p1);
        static_cast<Editor*>(LevelEditorLayer::get())->selectObject(p0);
    }
    void selectObjects(CCArray* p0, bool p1) {
        EditorUI::selectObjects(p0, p1);
        static_cast<Editor*>(LevelEditorLayer::get())->deselectObject();
    }
    
    void deselectObject(GameObject* p0) {
        EditorUI::deselectObject(p0);
        static_cast<Editor*>(LevelEditorLayer::get())->deselectObject();
    }

    void deselectAll() {
        EditorUI::deselectAll();
        static_cast<Editor*>(LevelEditorLayer::get())->deselectObject();
    }

    bool init(LevelEditorLayer* lel) {
		if (!EditorUI::init(lel))
			return false;

        newEditorUI = this;

        NodeIDs::provideFor(newEditorUI);
        auto menu = newEditorUI->getChildByID("playback-menu");
        auto winSize = CCDirector::get()->getWinSize();
		// auto spr = ButtonSprite::create("hey.......");
        // auto btn = CCMenuItemSpriteExtra::create(
        //     spr, newEditorUI, menu_selector(EditUI::coolthing)
        // );


        // things
        auto thing1 = CCScale9Sprite::create("translucent_panel.png"_spr);
        thing1->setCapInsets(CCRectMake(10, 10, 20, 20));
        thing1->setContentSize(CCSizeMake(108, 101));
        thing1->setPosition(ccp(76, 201));
        thing1->setScale(0.5);
        newEditorUI->addChild(thing1);
        auto thing2 = CCScale9Sprite::create("translucent_panel.png"_spr);
        thing2->setCapInsets(CCRectMake(10, 10, 20, 20));
        thing2->setContentSize(CCSizeMake(108, 101));
        thing2->setPosition(ccp(76, 258));
        thing2->setScale(0.5);
        newEditorUI->addChild(thing2);
        auto thing3 = CCScale9Sprite::create("translucent_panel.png"_spr);
        thing3->setCapInsets(CCRectMake(10, 10, 20, 20));
        thing3->setContentSize(CCSizeMake(108, 150));
        thing3->setPosition(ccp(76, 130));
        thing3->setScale(0.5);
        newEditorUI->addChild(thing3);
        auto thing4 = CCScale9Sprite::create("translucent_panel.png"_spr);
        thing4->setCapInsets(CCRectMake(10, 10, 20, 20));
        thing4->setContentSize(CCSizeMake(108, 150));
        thing4->setPosition(ccp(39, 42));
        thing4->setScale(0.5);
        newEditorUI->addChild(thing4);
        auto thing5 = CCScale9Sprite::create("translucent_panel.png"_spr);
        thing5->setCapInsets(CCRectMake(10, 10, 20, 20));
        thing5->setContentSize(CCSizeMake(214, 150));
        thing5->setPosition(ccp(506, 67));
        thing5->setScale(0.5);
        newEditorUI->addChild(thing5);
        auto thing6 = CCScale9Sprite::create("translucent_panel.png"_spr);
        thing6->setCapInsets(CCRectMake(10, 10, 20, 20));
        thing6->setContentSize(CCSizeMake(271, 120));
        thing6->setPosition(ccp(294, 0));
        thing6->setScale(0.5);
        newEditorUI->addChild(thing6);



        // inspector-panel
        auto inspectorPanel = CCScale9Sprite::create("GJ_square01.png");
        inspectorPanel->setCapInsets(CCRectMake(10, 10, 20, 20));
        inspectorPanel->setContentSize(CCSizeMake(231, 435));
        inspectorPanel->setPosition(ccp(506, 207));
        inspectorPanel->setScale(0.5);
        newEditorUI->addChild(inspectorPanel);

        // toolbox-panel
        auto toolboxPanel = CCScale9Sprite::create("GJ_square02.png");
        toolboxPanel->setCapInsets(CCRectMake(10, 10, 20, 20));
        toolboxPanel->setContentSize(CCSizeMake(144, 503));
        toolboxPanel->setPosition(ccp(39, 190));
        toolboxPanel->setScale(0.5);
        newEditorUI->addChild(toolboxPanel);

        // header-panel
        auto headerPanel = CCScale9Sprite::create("GJ_square02.png");
        headerPanel->setCapInsets(CCRectMake(10, 10, 20, 20));
        headerPanel->setContentSize(CCSizeMake(730, 57));
        headerPanel->setPosition(ccp(261, 301));
        headerPanel->setScale(0.5);
        newEditorUI->addChild(headerPanel);

        // layer-panel
        auto layerPanel = CCScale9Sprite::create("GJ_square02.png");
        layerPanel->setCapInsets(CCRectMake(10, 10, 20, 20));
        layerPanel->setContentSize(CCSizeMake(231, 57));
        layerPanel->setPosition(ccp(506, 20));
        layerPanel->setScale(0.5);
        newEditorUI->addChild(layerPanel);

        // touch priority garbo
        auto blocker = SwallowNode::create();
        blocker->setContentSize(CCSize(231, 435));
        blocker->setPosition(ccp(449, -49));
        newEditorUI->addChild(blocker, inspectorPanel->getZOrder() - 1);
        auto blocker2 = SwallowNode::create();
        blocker2->setContentSize(CCSize(231, 435));
        blocker2->setPosition(ccp(-156, -49));
        newEditorUI->addChild(blocker2, inspectorPanel->getZOrder() - 1);
        auto blocker3 = SwallowNode::create();
        blocker3->setContentSize(CCSize(231, 435));
        blocker3->setPosition(ccp(43, 518));
        blocker3->setRotation(90);
        newEditorUI->addChild(blocker3, inspectorPanel->getZOrder() - 1);

        // testing
        CCNode * tabBars[15] = {
            newEditorUI->getChildByID("block-tab-bar"),
            newEditorUI->getChildByID("outline-tab-bar"),
            newEditorUI->getChildByID("slope-tab-bar"),
            newEditorUI->getChildByID("hazard-tab-bar"),
            newEditorUI->getChildByID("3d-tab-bar"),
            newEditorUI->getChildByID("portal-tab-bar"),
            newEditorUI->getChildByID("monster-tab-bar"),
            newEditorUI->getChildByID("pixel-tab-bar"),
            newEditorUI->getChildByID("collectible-tab-bar"),
            newEditorUI->getChildByID("icon-tab-bar"),
            newEditorUI->getChildByID("deco-tab-bar"),
            newEditorUI->getChildByID("sawblade-tab-bar"),
            newEditorUI->getChildByID("trigger-tab-bar"),
            newEditorUI->getChildByID("custom-tab-bar"),
            newEditorUI->getChildByID("edit-tab-bar")
        };

        for (CCNode * tabBar : tabBars) {
            auto thing1 = tabBar->getChildByType(0)->getChildByType(0)->getChildren();

            if (tabBar->getChildByType(1)) {
                auto arrow1 = tabBar->getChildByType(1)->getChildByType(0);
                auto arrow2 = tabBar->getChildByType(1)->getChildByType(1);
                auto aS1 = arrow1->getChildByType(0);
                auto aS2 = arrow2->getChildByType(0);

                aS1->setScale(0.5);
                aS2->setScale(0.5);
                arrow1->setPosition(ccp(-265, 143));
                arrow2->setPosition(ccp(-224, 143));
            }

            if (tabBar->getChildByType(0)->getChildByType(1)) {
                auto dots = tabBar->getChildByType(0)->getChildByType(1);
                dots->setRotation(90);
                dots->setScale(0.4);
                dots->setPosition(ccp(6, 292));
            }

            CCObject* obj;
            CCARRAY_FOREACH(thing1, obj) {
                auto thing2 = static_cast<CCNode*>(obj)->getChildByType(0);
                auto layT = RowLayout::create();

                layT->setGrowCrossAxis(true);
                layT->setAxis(Axis::Row);
                layT->setAxisAlignment(AxisAlignment::Start);

                thing2->setContentSize(CCSize(130, 535));
                thing2->setLayout(layT);
                thing2->setPositionY(277);
                thing2->setScale(0.450);
                thing2->setPosition(CCPoint(38.5, 182));
            }
        }

        // background-sprite | spacer-line-left | spacer-line-right
        auto backgroundSprite = newEditorUI->getChildByID("background-sprite");
        backgroundSprite->removeMeAndCleanup();
        auto spacerLineLeft = newEditorUI->getChildByID("spacer-line-left");
        spacerLineLeft->removeMeAndCleanup();
        auto spacerLineRight = newEditorUI->getChildByID("spacer-line-right");
        spacerLineRight->removeMeAndCleanup();

        auto hsvButton = newEditorUI->getChildByID("hsv-button");
        //hsvButton->setParent(newEditorUI->getChildByID("EditorUI"));
        //hsvButton->setScale(0);
        auto editObject = newEditorUI->getChildByID("edit-object-button");
        //editObject->setParent(newEditorUI->getChildByID("EditorUI"));
        //editObject->setPositionX(10);
        auto editGroup = newEditorUI->getChildByID("edit-group-button");
        //editGroup->setParent(newEditorUI->getChildByID("EditorUI"));
        //editGroup->setPositionX(10);
        auto editSpecial = newEditorUI->getChildByID("edit-special-button");
        //editSpecial->setParent(newEditorUI->getChildByID("EditorUI"));
        //editSpecial->setPositionX(10);

        // object-info-label
        auto objectInfoLabel = newEditorUI->getChildByID("object-info-label");
        objectInfoLabel->setScale(0.35);
        objectInfoLabel->setPosition(CCPoint(108, 270));
        // position-slider
        auto positionSlider = newEditorUI->getChildByID("position-slider");
        positionSlider->setScale(0.575);
        positionSlider->setPosition(CCPoint(172.5, -53));
        // toolbar-categories-menu
        auto toolbarCatergoriesMenu = newEditorUI->getChildByID("toolbar-categories-menu");
        toolbarCatergoriesMenu->setScale(0.6);
        toolbarCatergoriesMenu->setPosition(CCPoint(39, 33));
        // toolbar-toggles-menu
        auto toolbarTogglesMenu = newEditorUI->getChildByID("toolbar-toggles-menu");
        toolbarTogglesMenu->setScale(0.5);
        toolbarTogglesMenu->setPosition(CCPoint(422, 260));

        // undo-menu
        auto undoMenu = newEditorUI->getChildByID("undo-menu");
        undoMenu->setScale(0.525);
        undoMenu->setPosition(CCPoint(89, 130));
        undoMenu->setContentSize(CCSize(50, 137));
        auto undoLayout = RowLayout::create();
        undoLayout->setGrowCrossAxis(true);
        undoLayout->setCrossAxisAlignment(AxisAlignment::Start);
        undoLayout->setGap(5);
        undoMenu->setLayout(undoLayout);
        // playback-menu
        auto playbackMenu = typeinfo_cast<CCMenu*>(newEditorUI->getChildByID("playback-menu"));
        playbackMenu->setScale(0.525);
        playbackMenu->setPosition(CCPoint(108, 301));
        playbackMenu->setTouchPriority(-129);
        // playtest-menu
        auto playtestMenu = newEditorUI->getChildByID("playtest-menu");
        playtestMenu->setScale(0.525);
        playtestMenu->setPosition(CCPoint(129, 301));
        // zoom-menu
        auto zoomMenu = newEditorUI->getChildByID("zoom-menu");
        zoomMenu->setScale(0.675);
        zoomMenu->setPosition(CCPoint(89, 255));
        // link-menu
        auto linkMenu = newEditorUI->getChildByID("link-menu");
        linkMenu->setScale(0.675);
        linkMenu->setPosition(CCPoint(89, 197.5));
        // settings-menu
        auto settingsMenu = newEditorUI->getChildByID("settings-menu");
        settingsMenu->setScale(0.525);
        settingsMenu->setPosition(CCPoint(402, 301));
        // editor-buttons-menu
        auto editorButtonsMenu = typeinfo_cast<CCMenu*>(newEditorUI->getChildByID("editor-buttons-menu"));
        editorButtonsMenu->setScale(0.75);
        editorButtonsMenu->setPosition(CCPoint(509, 65.5));
        editorButtonsMenu->setContentSize(CCSize(210, 80));
        editorButtonsMenu->updateLayout(true);
        editorButtonsMenu->setTouchPriority(-101);
        // layer-menu
        auto layerMenu = newEditorUI->getChildByID("layer-menu");
        layerMenu->setScale(0.725);
        layerMenu->setPosition(CCPoint(517, 20));
        // build-tabs-menu
        auto buildTabsMenu = newEditorUI->getChildByID("build-tabs-menu");
        buildTabsMenu->setScale(0.525);
        buildTabsMenu->setPosition(CCPoint(257.5, 292));

        //auto another = CCLabelBMFont::create(levelName, "bigFont.fnt");
		//another->setPosition({ winSize.width / 2, winSize.height / 2});
		//newEditorUI->addChild(another);

		return true;
	}
};
