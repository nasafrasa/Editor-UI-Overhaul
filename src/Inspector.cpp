#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include "Geode/cocos/cocoa/CCString.h"
#include "ObjectProperties.hpp"

using namespace geode::prelude;
EditorUI* ui;
void createInspector(GameObject* p0, int tab = 0);
struct NumberFieldPair {
    CCMenu* menu;
    TextInput* field;
};

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
    void onCheckboxChange(CCObject* sender) {
        auto toggler = static_cast<CCMenuItemToggler*>(sender);
        auto array = static_cast<CCArray*>(toggler->getUserObject());
        
        auto obj = static_cast<EffectGameObject*>(array->objectAtIndex(0));
        auto property = static_cast<CCString*>(array->objectAtIndex(1))->m_sString;

        if (property == "Multi Trigger") {
            obj->m_isMultiTriggered = !obj->m_isMultiTriggered;
        }
    }

    void onSlider(CCObject* sender) {
        auto thumb = static_cast<SliderThumb*>(sender);
        auto sliderLogic = dynamic_cast<SliderTouchLogic*>(thumb->getParent());
        auto slider = dynamic_cast<Slider*>(sliderLogic->getParent());
        auto arr = static_cast<CCArray*>(slider->getUserObject());

        auto obj = static_cast<EffectGameObject*>(arr->objectAtIndex(0));
        auto property = static_cast<CCString*>(arr->objectAtIndex(1))->m_sString;

        std::string value = std::to_string(thumb->getValue());

        auto linkedField = static_cast<TextInput*>(arr->objectAtIndex(2));
        if (property == "Opacity") {
            obj->m_opacity = thumb->getValue();
            linkedField->setString(floatToFormattedString(thumb->getValue(), 2));
        }
        if (property == "Duration") {
            obj->m_duration = thumb->getValue() * 10.f;
            linkedField->setString(floatToFormattedString(thumb->getValue() * 10.f, 2));
            ui->moveObject(obj, {0, 1}); // theres probably a better way to do this LMAOOOOO
            ui->moveObject(obj, {0, -1});
        }
    }

    void onToggleChange(CCObject* sender) {
        auto spr = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto array = static_cast<CCArray*>(spr->getUserObject());
        
        auto obj = static_cast<EffectGameObject*>(array->objectAtIndex(0));
        auto property = static_cast<CCString*>(array->objectAtIndex(1))->m_sString;

        if (property == "Trigger Type") {
            int state = 2;
            if (obj->m_isSpawnTriggered) {
                state = 0;
            } else if (obj->m_isTouchTriggered) {
                state = 1;
            }
            state = (state + 1) % 3;

            const char* labels[] = { "Spawn", "Touch", "Time" };
            bool spawn = (state == 0);
            bool touch = (state == 1);

            spr->setSprite(ButtonSprite::create(labels[state]));
            spr->updateSprite();

            obj->m_isSpawnTriggered = spawn;
            obj->m_isTouchTriggered = touch;
            
            if (spawn || touch) {
                createInspector(obj, 1);
            } else {
                createInspector(obj, 0);
            }
        }
    }
};

NumberFieldPair createNumberField(
    std::function<void (const std::string &)> callback, 
    const std::string& string, 
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
    menu->setPosition(ccp(-0.5, 0.25));
    return { menu, field };
}

CCMenu* createCheckboxField(EffectGameObject* obj, std::string property) {
    auto toggler = CCMenuItemToggler::createWithStandardSprites(
        ui, 
        menu_selector(InspectorInput::onCheckboxChange),
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

std::pair<CCMenu*, Slider*> createSliderField(
    EffectGameObject* obj, std::string property, TextInput* linkedField
) {
    auto slider = Slider::create(
        ui,
        menu_selector(InspectorInput::onSlider),
        1.5
    );
    
    slider->setValue(0);
    if (property == "Opacity") {
        slider->setValue(obj->m_opacity);
    }
    if (property == "Duration") {
        slider->setValue(obj->m_duration / 10.f);
    }
    
    slider->setAnchorPoint({0, 0});
    slider->setPosition({300, 100});
    slider->setLiveDragging(true);

    auto sliderBar = slider->getChildByType(0);
    sliderBar->setScaleY(1);
    auto sliderBtn1 = slider->getChildByType(1)->getChildByType(0)->getChildByType(0);
    sliderBtn1->setPosition(sliderBtn1->getContentSize() / 2);
    sliderBtn1->setScale(0.650);
    sliderBtn1->setAnchorPoint(ccp(0.5, 0.5));
    auto sliderBtn2 = slider->getChildByType(1)->getChildByType(0)->getChildByType(1);
    sliderBtn2->setPosition(sliderBtn2->getContentSize() / 2);
    sliderBtn2->setScale(0.650);
    sliderBtn2->setAnchorPoint(ccp(0.5, 0.5));
    
    auto arr = CCArray::create();
    arr->addObject(obj);
    arr->addObject(CCString::create(property));
    arr->addObject(linkedField);
    slider->setUserObject(arr);

    auto menu = CCMenu::create();
    menu->addChild(slider);
    menu->setPosition(ccp(-234.5, -136));
    menu->setScale(0.325);
    return { menu, slider };
}

CCMenu* createToggleField(EffectGameObject* obj, std::string property) {
    auto spr = ButtonSprite::create("joe");    

    if (property == "Trigger Type") {
        if (obj->m_isSpawnTriggered) {
            spr = ButtonSprite::create("Spawn");
        } else if (obj->m_isTouchTriggered) {
            spr = ButtonSprite::create("Touch");
        } else {
            spr = ButtonSprite::create("Time");
        }
    }

    auto btn = CCMenuItemSpriteExtra::create(
        spr, ui, menu_selector(InspectorInput::onToggleChange)
    );
    btn->setAnchorPoint({ 1.f, 0.5f });

    auto arr = CCArray::create();
    arr->addObject(obj);
    arr->addObject(CCString::create(property));
    btn->setUserObject(arr);

    auto menu = CCMenu::create();
    menu->addChild(btn);
    menu->setScale(0.275);
    menu->setPosition({-97.5f, -111.5f});
    return menu;
}

int getTab(EffectGameObject* obj) {
    if ( (obj->m_isSpawnTriggered || obj->m_isTouchTriggered) ) {
        return 1;
    }
    return 0;
}

void createInspector(GameObject* p0, int tab) {
    auto obj = static_cast<EffectGameObject*>(p0);
    auto winSize = CCDirector::get()->getWinSize();
    
    auto listItems = CCArray::create();
    std::vector<std::string> propertyNames = {" "};
    auto inspectorTitleText = "Joe H. Bruh";

    // Check for checkboxes
    std::vector<int> simpleTriggers = {
        33,
        32,
        1613,
        1612,
        1818,
        1819,
        1917
    };
    bool isSimpleTrigger = std::find(
        simpleTriggers.begin(),
        simpleTriggers.end(),
        obj->m_objectID
    ) != simpleTriggers.end();

    // Get specific properties for the object and tab
    for (const auto& object : objectInsProp) {
        if (obj->m_objectID == object.id) {
            tab = getTab(obj);
            propertyNames = object.inspectorPanelProperties[tab];
            inspectorTitleText = object.name.c_str();
            break;
        } else {
            propertyNames = {" "};
            inspectorTitleText = "Work In Progress";
        }
    }

    if (isSimpleTrigger) {
        tab = getTab(obj);
        std::vector<std::vector<std::string>> defaultProperties = {
            { "Trigger Type" },
            { "Trigger Type", "Multi Trigger" }
        };
        propertyNames = defaultProperties[tab];
        inspectorTitleText = getNameFromID(obj->m_objectID).c_str();
    }



    // Create inspector elements
    for (const auto& property : propertyNames) {
        CCMenu* propertyField = nullptr;
        auto labelText = property;
        auto wrapper = CCNode::create();
        CCNode* bonusWrapper = nullptr;

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

        // Check for toggles
        std::vector<std::string> toggleProperties = {
            "Trigger Type"
        };
        bool isToggle = std::find(
            toggleProperties.begin(),
            toggleProperties.end(),
            property
        ) != toggleProperties.end();
        if (isToggle) {
            propertyField = createToggleField(obj, property);
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
            ).menu;
        }
        if (property == "Duration") {
            auto sliderPtr = std::make_shared<Slider*>(nullptr);
            auto propertyFieldMenu = createNumberField(
                [obj, sliderPtr](const std::string& input) {
                    if (input.find_first_of("1234567890") != std::string::npos) {
                        float val = std::stof(input);
                        obj->m_duration = std::stof(input);
                        if (*sliderPtr) {
                            (*sliderPtr)->setValue(val / 10.f);
                        }
                    }
                },
                floatToFormattedString(obj->m_duration, 2).c_str(),
                0,
                "1234567890."
            );
            auto [sliderMenu, createdSlider] = createSliderField(obj, property, propertyFieldMenu.field);
            *sliderPtr = createdSlider;
            propertyField = propertyFieldMenu.menu;
            bonusWrapper = CCNode::create();
            bonusWrapper->addChild(sliderMenu);
            if (obj->m_objectID == 1007) {
                labelText = "Fade Duration";
            }
        }
        if (property == "Opacity") {
            auto sliderPtr = std::make_shared<Slider*>(nullptr);
            auto propertyFieldMenu = createNumberField(
                [obj, sliderPtr](const std::string& input) {
                    if (input.find_first_of("1234567890") != std::string::npos) {
                        float val = std::stof(input);
                        if (val <= 1.0f) {
                            obj->m_opacity = val;
                            if (*sliderPtr) {
                                (*sliderPtr)->setValue(val);
                            }
                        }
                    }
                },
                floatToFormattedString(obj->m_opacity, 2),
                0,
                "1234567890."
            );
            auto [sliderMenu, createdSlider] = createSliderField(obj, property, propertyFieldMenu.field);
            *sliderPtr = createdSlider;
            propertyField = propertyFieldMenu.menu;
            bonusWrapper = CCNode::create();
            bonusWrapper->addChild(sliderMenu);
        }
        
        

        auto label = CCLabelBMFont::create(labelText.c_str(), "bigFont.fnt");
        label->setAnchorPoint({ 0.f, 0.f });
        label->setPosition(ccp(1.5, 1.5));
        label->setScale(0.2);

        wrapper->addChild(label);
        if (propertyField) wrapper->addChild(propertyField);
        
        listItems->addObject(wrapper);
        if (bonusWrapper) listItems->addObject(bonusWrapper);
    }

    // Make Inspector List
    if (ui->getChildByID("euio-inspector-list")) ui->getChildByID("euio-inspector-list")->removeMeAndCleanup();
    auto inspectorList = ListView::create(
        listItems,
        9,
        109,
        194
    );

    auto layT = ColumnLayout::create();
    layT->setAxisReverse(true);
    layT->setCrossAxisAlignment(AxisAlignment::Start);
    layT->setCrossAxisLineAlignment(AxisAlignment::Start);
    layT->setGrowCrossAxis(false);
    layT->setAxisAlignment(AxisAlignment::End);
    layT->setGap(0);
    auto listing = inspectorList->getChildByType(0)->getChildByType(0);
    listing->setScaledContentSize(CCSize(109, 193));
    listing->setPosition(ccp(0, 0));
    listing->setLayout(layT);

    inspectorList->setPosition(ccp(451.5, 103.5));
    inspectorList->setZOrder(1);
    inspectorList->setTouchEnabled(true);
    inspectorList->setTouchPriority(-1);
    inspectorList->setCellOpacity(50);
    inspectorList->setPrimaryCellColor(ccc3(0, 0, 10));
    inspectorList->setSecondaryCellColor(ccc3(50, 50, 60));
    inspectorList->setCellBorderColor(ccc4(0, 0, 0, 0));
    inspectorList->updateTransform();
    inspectorList->setID("euio-inspector-list");
    ui->addChild(inspectorList);

    // Make Inspector Title
    if (ui->getChildByID("euio-inspector-title")) ui->getChildByID("euio-inspector-title")->removeFromParent();
    auto inspectorTitle = CCLabelBMFont::create(inspectorTitleText, "goldFont.fnt");
    inspectorTitle->setID("euio-inspector-title");
    inspectorTitle->setPosition(winSize / 2);
    inspectorTitle->setScale(0.425);
    inspectorTitle->setPosition(ccp(453, 306));
    inspectorTitle->setAnchorPoint(ccp(0, 0.5));
    ui->addChild(inspectorTitle);
}

void destroyInspector() {
    if (ui->getChildByID("euio-inspector-list")) ui->getChildByID("euio-inspector-list")->removeFromParent();
    if (ui->getChildByID("euio-inspector-title")) ui->getChildByID("euio-inspector-title")->removeFromParent();
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
