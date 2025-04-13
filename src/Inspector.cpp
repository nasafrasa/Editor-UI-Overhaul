#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include "Geode/cocos/cocoa/CCString.h"
#include "ObjectProperties.hpp"

using namespace geode::prelude;
EditorUI* ui;
void createInspector(GameObject* p0, int tab = 0);

std::string floatToFormattedString(float num, int round) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(round) << num;
    std::string string = ss.str();
    string.erase(string.find_last_not_of('0') + 1, std::string::npos);
    if (string.back() == '.') string.pop_back();
    return string;
}

bool containsString(const std::vector<std::string>& arr, const std::string& target) {
    return std::any_of(arr.begin(), arr.end(), [&](const std::string& s) {
        return s == target;
    });
}

class InspectorInput : public CCLayer {
    public:
    void onToggleChange(CCObject* sender) {
        auto toggler = static_cast<CCMenuItemToggler*>(sender);
        auto array = static_cast<CCArray*>(toggler->getUserObject());
        
        auto obj = static_cast<EffectGameObject*>(array->objectAtIndex(0));
        auto property = static_cast<CCString*>(array->objectAtIndex(1));

        if (property->m_sString == "Multi Trigger") {
            obj->m_isMultiTriggered = !obj->m_isMultiTriggered;
            createInspector(obj, 1);
        }
    }
};

CCMenu* createNumberField(
    std::function<void (const std::string &)> callback, 
    const std::string string, 
    size_t maxCharCount = 0, 
    std::string filter = "1234567890"
) {
    auto field = TextInput::create(60, "num");
    field->getInputNode()->setScale(1.7);
    field->getInputNode()->setPositionX(9);
    field->getInputNode()->setAnchorPoint({ 0.f, 0.f });
    field->getChildByType(0)->setContentSize(CCSize(230, 60));
    field->getChildByType(0)->setPositionX(6);
    field->setPosition(ccp(107.5, 1));
    field->setAnchorPoint({ 1.f, 0.f });
    field->setScale(0.225);

    field->setMaxCharCount(maxCharCount);
    field->setFilter(filter);
    field->setString(string, false);
    field->setCallback(callback);

    auto menu = CCMenu::create();
    menu->addChild(field);
    menu->setPosition(ccp(0, 0));
    return menu;
}

CCMenu* createCheckboxField(EffectGameObject* obj, std::string property) {
    auto toggler = CCMenuItemToggler::createWithStandardSprites(
        ui, 
        menu_selector(InspectorInput::onToggleChange),
        1
    );
    
    if (property == "Multi Trigger") {
        toggler->toggle(obj->m_isMultiTriggered);
    }

    auto arr = CCArray::create();
    arr->addObject(obj);
    arr->addObject(CCString::create(property));
    toggler->setUserObject(arr);

    auto menu = CCMenu::create();
    menu->addChild(toggler);
    menu->setScale(0.25);
    menu->setPosition({-108.5f, -115.5f});
    return menu;
}

void createInspector(GameObject* p0, int tab) {
    auto obj = static_cast<EffectGameObject*>(p0);
    auto winSize = CCDirector::get()->getWinSize();
    
    auto listItems = CCArray::create();
    std::vector<std::string> propertyNames = {" "};
    auto inspectorTitleText = "Joe H. Bruh";

    // For dynamic menus :(
    if (obj->m_isMultiTriggered) {
        tab = 1;
    }

    // Get specific properties for the object and tab
    for (const auto& object : objectInsProp) {
        if (obj->m_objectID == object.id) {
            propertyNames = object.inspectorPanelProperties[tab];
            inspectorTitleText = object.name.c_str();
        } else {
            propertyNames = {" "};
            inspectorTitleText = "Work In Progress";
        }
    }



    // Create inspector elements
    for (const auto& property : propertyNames) {
        CCMenu* propertyField = nullptr;
        auto labelText = property;

        // Check for checkboxes
        std::vector<std::string> checkboxProperties = {
            "Multi Trigger"
        };
        bool isCheckbox = std::find(
            checkboxProperties.begin(),
            checkboxProperties.end(),
            property
        ) != checkboxProperties.end();
        if (isCheckbox) {
            propertyField = createCheckboxField(obj, property);
        }

        // Check for number inputs
        if (property == "Target Group ID") {
            propertyField = createNumberField(
                [propertyField, obj] (const std::string& input) {
                    if (!input.empty()) {
                        obj->m_targetGroupID = std::stoi(input);
                        obj->getObjectLabel()->setString(std::to_string(std::stoi(input)).c_str());
                    }
                },
                std::to_string(obj->m_targetGroupID).c_str(),
                4
            );
        }
        if (property == "Fade Duration") {
            propertyField = createNumberField(
                [obj] (const std::string& input) {
                    if (input.find_first_of("1234567890") != std::string::npos) obj->m_duration = std::stof(input);
                },
                floatToFormattedString(obj->m_duration, 2).c_str(),
                0,
                "1234567890."
            );
        }
        if (property == "Opacity") {
            propertyField = createNumberField(
                [obj] (const std::string& input) {
                    if (input.find_first_of("1234567890") != std::string::npos) {
                        if (std::stof(input) <= 1) obj->m_opacity = std::stof(input);
                    }
                },
                floatToFormattedString(obj->m_opacity, 2).c_str(),
                0,
                "1234567890."
            );
        }

        auto label = CCLabelBMFont::create(labelText.c_str(), "bigFont.fnt");
        label->setAnchorPoint({ 0.f, 0.f });
        label->setPosition(ccp(1.5, 1.5));
        label->setScale(0.2);

        auto wrapper = CCNode::create();
        wrapper->addChild(label);
        if (propertyField) wrapper->addChild(propertyField);
        wrapper->setContentSize(label->getContentSize());
        
        listItems->addObject(wrapper);
    }

    

    // Make Inspector List
    if (ui->getChildByID("inspector-list")) ui->getChildByID("inspector-list")->removeMeAndCleanup();
    auto inspectorList = ListView::create(
        listItems,
        9.0f,
        109.0f,
        194.0f
    );
    inspectorList->setPosition({ 451.5, 103.5});
    inspectorList->setScale(1);
    inspectorList->setZOrder(1);
    inspectorList->setTouchEnabled(true);
    inspectorList->setTouchPriority(-1);
    inspectorList->setCellOpacity(50);
    inspectorList->setPrimaryCellColor(cocos2d::ccc3(0, 0, 10));
    inspectorList->setSecondaryCellColor(cocos2d::ccc3(50, 50, 60));
    inspectorList->setCellBorderColor(cocos2d::ccc4(0, 0, 0, 0));
    inspectorList->setID("inspector-list");
    ui->addChild(inspectorList);

    // Make Inspector Title
    if (ui->getChildByID("inspector-title")) ui->getChildByID("inspector-title")->removeFromParent();
    auto inspectorTitle = CCLabelBMFont::create(inspectorTitleText, "goldFont.fnt");
    inspectorTitle->setID("inspector-title");
    inspectorTitle->setPosition(winSize / 2);
    inspectorTitle->setScale(0.425);
    inspectorTitle->setPosition(ccp(453, 306));
    inspectorTitle->setAnchorPoint(ccp(0, 0.5));
    ui->addChild(inspectorTitle);
}

void destroyInspector() {
    if (ui->getChildByID("inspector-list")) ui->getChildByID("inspector-list")->removeFromParent();
    if (ui->getChildByID("inspector-title")) ui->getChildByID("inspector-title")->removeFromParent();
}

class $modify(InspectorPanel, EditorUI) {
    void selectObject(GameObject* p0, bool p1) {
        EditorUI::selectObject(p0, p1);
        createInspector(p0, 0);
    }

    void selectObjects(CCArray* p0, bool p1) {
        EditorUI::selectObjects(p0, p1);
        destroyInspector();
    }
    
    void deselectObject(GameObject* p0) {
        EditorUI::deselectObject(p0);
        destroyInspector();
    }

    void deselectAll() {
        EditorUI::deselectAll();
        destroyInspector();
    }

    bool init(LevelEditorLayer* p0) {
		if (!EditorUI::init(p0)) return false;
        ui = this;
        return true;
    }
};
