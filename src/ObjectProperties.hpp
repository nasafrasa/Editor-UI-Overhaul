#pragma once

struct ObjectProperties {
    int id;
    std::string name;
    std::vector<std::vector<std::string>> inspectorPanelProperties;
};

extern const std::vector<ObjectProperties> objectInsProp;

std::string getNameFromID(int objectID);
