#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorUI.hpp>

// I'll probably only use this file as a notepad and testing ground for new features

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
