#include "ObjectProperties.hpp"

const std::vector<ObjectProperties> objectInsProp = {
    {
        1007,
        "Alpha", 
        {
            { "Target Group ID", "Duration", "Opacity", " ", "Trigger Type" },
            { "Target Group ID", "Duration", "Opacity", " ", "Trigger Type", "Multi Trigger" }
        }
    }
};

std::string getNameFromID(int objectID) {
    switch (objectID) {
        case 33:
            return "Disable Trail";
        case 32:
            return "Enable Trail";
        case 1613:
            return "Show Player";
        case 1612:
            return "Hide Player";
        case 1818:
            return "Enable BG Effect";
        case 1819:
            return "Disable BG Effect";
        case 1917:
            return "Reverse Gameplay";
    }
    return "Error";
}
