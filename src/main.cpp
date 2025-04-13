#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/EditLevelLayer.hpp>

/**
 * !KNOWN QUIRKS ATM:
 * cant select anything below certain point
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
std::vector<std::tuple<int, std::string, std::vector<std::vector<int>>>> objectInsProp = {
    {
        1007,
        "Alpha", 
        {
            {1, 2, 3, 0, 4},
            {1, 2, 3, 0, 4, 5}
        }
    }
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

class $modify(Editor, LevelEditorLayer) {
    void selectObject(GameObject* object, int pageID = 0) {
        auto obj = static_cast<EffectGameObject*>(object);
        auto winSize = CCDirector::get()->getWinSize();




        auto items = CCArray::create();
        std::vector<int> propertyIDs = {0};
        auto objectName = "Joe H. Bruh";

        for (int i = 0; i < objectInsProp.size(); i++) {
            int targetObjectID = std::get<0>(objectInsProp[i]);
            if (obj->m_objectID == targetObjectID) {
                std::string name = std::get<1>(objectInsProp[i]);
                std::vector< std::vector<int>> valueValues = std::get<2>(objectInsProp[i]);
                std::vector<int> values = valueValues[pageID];
                
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



        if (newEditorUI->getChildByID("testing-label")) {
            newEditorUI->getChildByID("testing-label")->removeMeAndCleanup();
        }
        auto labelnew = CCLabelBMFont::create(std::to_string(obj->m_targetGroupID).c_str(), "bigFont.fnt");
        labelnew->setID("testing-label");
        labelnew->setPosition(winSize / 2);
		//newEditorUI->addChild(labelnew);

    }

    void deselectObject() {
        if (newEditorUI->getChildByID("inspector-list")) newEditorUI->getChildByID("inspector-list")->removeMeAndCleanup();
        if (newEditorUI->getChildByID("inspector-title")) newEditorUI->getChildByID("inspector-title")->removeMeAndCleanup();
    }
};
