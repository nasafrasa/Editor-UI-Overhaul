#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

std::vector<std::string> newUINodes = {
    "euio-solid-panel-0", "euio-solid-panel-1", "euio-solid-panel-2", "euio-solid-panel-3",
    "euio-translucent-panel-0", "euio-translucent-panel-1", "euio-translucent-panel-2", 
    "euio-translucent-panel-3", "euio-translucent-panel-4", "euio-translucent-panel-5"
};

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

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
        auto location = touch->getLocation();
        if (this->boundingBox().containsPoint(location)) {
            return true;
        }
        return false;
    }
};

class $modify(ArrangeUI, EditorUI) {
    struct Fields {
        GameObject* selectedObjectCache;
    };

    void onPlaytest(cocos2d::CCObject *p0) {
        EditorUI::onPlaytest(p0);
        for (const auto& node : newUINodes) {
            this->getChildByID(node)->setVisible(false);
        }
    }

    void onStopPlaytest(cocos2d::CCObject *p0) {
        EditorUI::onStopPlaytest(p0);
        for (const auto& node : newUINodes) {
            this->getChildByID(node)->setVisible(true);
        }
    }

    bool init(LevelEditorLayer* p0) {
		if (!EditorUI::init(p0)) return false;

        this->m_toolbarHeight = 0;

        NodeIDs::provideFor(this);
        auto menu = this->getChildByID("playback-menu");
        auto winSize = CCDirector::get()->getWinSize();

        // Translucent Panels
        std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> translucentPanels = {
            { {108, 101}, {76, 201} },      // Link Controls
            { {108, 101}, {76, 258} },      // Zoom Controls
            { {108, 150}, {76, 130} },      // Undo Controls
            { {108, 150}, {39, 42} },       // Editor Modes
            { {214, 150}, {506, 67} },      // Edit Buttons
            { {636, 120}, {254, 0} },       // Pan Bar
        };
        auto spriteID = 0;
        for (const auto& panel : translucentPanels) {
            const auto& [contentSize, position] = panel;
            auto sprite = CCScale9Sprite::create("translucent_panel.png"_spr);
            sprite->setCapInsets(CCRectMake(10, 10, 20, 20));
            sprite->setContentSize(CCSizeMake(contentSize.first, contentSize.second));
            sprite->setPosition(ccp(position.first, position.second));
            sprite->setScale(0.5);
            sprite->setID("euio-translucent-panel-" + std::to_string(spriteID));
            this->addChild(sprite);
            spriteID++;
        }

        // Solid Panels
        struct SolidPanel {
            std::string spriteImage;
            std::vector<int> contentSize;
            std::vector<int> position;
        };
        std::vector<SolidPanel> solidPanels = {
            { "GJ_square01.png", {231, 435}, {506, 207} },      // inspector-panel
            { "GJ_square02.png", {144, 503}, {39, 190} },       // toolbox-panel
            { "GJ_square02.png", {730, 57}, {261, 301} },       // header-panel
            { "GJ_square02.png", {231, 57}, {506, 20} }         // layer-panel
        };
        spriteID = 0;
        for (SolidPanel& panel : solidPanels) {
            auto sprite = CCScale9Sprite::create(panel.spriteImage.c_str());
            sprite->setCapInsets(CCRectMake(10, 10, 20, 20));
            sprite->setContentSize(CCSizeMake(panel.contentSize[0], panel.contentSize[1]));
            sprite->setPosition(ccp(panel.position[0], panel.position[1]));
            sprite->setScale(0.5);
            sprite->setID("euio-solid-panel-" + std::to_string(spriteID));
            this->addChild(sprite);
            spriteID++;
        }

        // Touch Blockers
        auto blocker = SwallowNode::create();
        blocker->setContentSize(CCSize(231, 435));
        blocker->setPosition(ccp(449, -49));
        this->addChild(blocker, -1);
        auto blocker2 = SwallowNode::create();
        blocker2->setContentSize(CCSize(231, 435));
        blocker2->setPosition(ccp(-156, -49));
        this->addChild(blocker2, -1);
        auto blocker3 = SwallowNode::create();
        blocker3->setContentSize(CCSize(231, 435));
        blocker3->setPosition(ccp(43, 518));
        blocker3->setRotation(90);
        this->addChild(blocker3, -1);

        // testing
        std::vector<std::string> tabBars = {
            "block", "outline", "slope", "hazard", "3d",
            "portal", "monster", "pixel", "collectible", "icon",
            "deco", "sawblade", "trigger", "custom", "edit"
        };
        for (std::string tabBarName : tabBars) {
            auto tabBar = this->getChildByID(tabBarName + "-tab-bar");
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
        auto backgroundSprite = this->getChildByID("background-sprite");
        backgroundSprite->removeMeAndCleanup();
        auto spacerLineLeft = this->getChildByID("spacer-line-left");
        spacerLineLeft->removeMeAndCleanup();
        auto spacerLineRight = this->getChildByID("spacer-line-right");
        spacerLineRight->removeMeAndCleanup();

        auto hsvButton = this->getChildByID("editor-buttons-menu")->getChildByType(11);
        auto editObject = this->getChildByID("editor-buttons-menu")->getChildByType(1);
        auto editGroup = this->getChildByID("editor-buttons-menu")->getChildByType(5);
        auto editSpecial = this->getChildByID("editor-buttons-menu")->getChildByType(9);
        //hsvButton->removeFromParent();
        //editObject->removeFromParent();
        //editGroup->removeFromParent();
        //editSpecial->removeFromParent();

        // position-slider
        auto positionSlider = this->getChildByID("position-slider");
        positionSlider->setScale(1.575);
        positionSlider->setPosition(CCPoint(417, 106));
        auto sliderBar = positionSlider->getChildByType(0);
        sliderBar->setScaleY(0.25);
        auto sliderBtn1 = positionSlider->getChildByType(1)->getChildByType(0)->getChildByType(0);
        sliderBtn1->setPosition(ccp(12, 20));
        sliderBtn1->setScale(0.35);
        sliderBtn1->setAnchorPoint(ccp(0, 0.5));
        auto sliderBtn2 = positionSlider->getChildByType(1)->getChildByType(0)->getChildByType(1);
        sliderBtn2->setPosition(ccp(12, 20));
        sliderBtn2->setScale(0.35);
        sliderBtn2->setAnchorPoint(ccp(0, 0.5));

        // object-info-label
        auto objectInfoLabel = this->getChildByID("object-info-label");
        objectInfoLabel->setScale(0.35);
        objectInfoLabel->setPosition(CCPoint(108, 270));
        // toolbar-categories-menu
        auto toolbarCatergoriesMenu = this->getChildByID("toolbar-categories-menu");
        toolbarCatergoriesMenu->setScale(0.6);
        toolbarCatergoriesMenu->setPosition(CCPoint(39, 34));
        // toolbar-toggles-menu
        auto toolbarTogglesMenu = this->getChildByID("toolbar-toggles-menu");
        toolbarTogglesMenu->setScale(0.5);
        toolbarTogglesMenu->setPosition(CCPoint(422, 260));
        // undo-menu
        auto undoMenu = this->getChildByID("undo-menu");
        undoMenu->setScale(0.525);
        undoMenu->setPosition(CCPoint(89, 130));
        undoMenu->setContentSize(CCSize(50, 137));
        auto undoLayout = RowLayout::create();
        undoLayout->setGrowCrossAxis(true);
        undoLayout->setCrossAxisAlignment(AxisAlignment::Start);
        undoLayout->setGap(5);
        undoMenu->setLayout(undoLayout);
        // playback-menu
        auto playbackMenu = typeinfo_cast<CCMenu*>(this->getChildByID("playback-menu"));
        playbackMenu->setScale(0.525);
        playbackMenu->setPosition(CCPoint(108, 301));
        playbackMenu->setTouchPriority(-129);
        // playtest-menu
        auto playtestMenu = this->getChildByID("playtest-menu");
        playtestMenu->setScale(0.525);
        playtestMenu->setPosition(CCPoint(129, 301));
        // zoom-menu
        auto zoomMenu = this->getChildByID("zoom-menu");
        zoomMenu->setScale(0.675);
        zoomMenu->setPosition(CCPoint(89, 255));
        // link-menu
        auto linkMenu = this->getChildByID("link-menu");
        linkMenu->setScale(0.675);
        linkMenu->setPosition(CCPoint(89, 197.5));
        // settings-menu
        auto settingsMenu = this->getChildByID("settings-menu");
        settingsMenu->setScale(0.525);
        settingsMenu->setPosition(CCPoint(402, 301));
        // editor-buttons-menu
        auto editorButtonsMenu = typeinfo_cast<CCMenu*>(this->getChildByID("editor-buttons-menu"));
        editorButtonsMenu->setScale(0.75);
        editorButtonsMenu->setPosition(CCPoint(485, 65.5));
        editorButtonsMenu->setContentSize(CCSize(210, 80));
        editorButtonsMenu->updateLayout(true);
        editorButtonsMenu->setTouchPriority(-101);
        // layer-menu
        auto layerMenu = this->getChildByID("layer-menu");
        layerMenu->setScale(0.725);
        layerMenu->setPosition(CCPoint(517, 20));
        // build-tabs-menu
        auto buildTabsMenu = this->getChildByID("build-tabs-menu");
        buildTabsMenu->setScale(0.525);
        buildTabsMenu->setPosition(CCPoint(257.5, 292));

		return true;
	}
};
