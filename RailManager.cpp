#include "RailManager.h"
#include <iostream>
#include <cmath> 

using namespace m1;

RailManager::RailManager() : headRail(nullptr) {}

RailManager::~RailManager() {
    for (Rail* rail : allRails) delete rail;
    allRails.clear();
}

Rail* RailManager::CreateSingleRail(glm::vec3 start, glm::vec3 end, std::string type) {
    Rail* newRail = new Rail();

    newRail->startPosition = start;
    newRail->endPosition = end;
    newRail->type = type;
    newRail->next = nullptr;

    newRail->isBroken = false;
    newRail->timeSinceBroken = 0.0f;
    newRail->currentRepairAmount = 0.0f;

    return newRail;
}

void RailManager::CreateSegment(glm::vec3 start, glm::vec3 end, std::string type) {
    // Impart segmentele lungi in bucati de maxim 20 unitati pentru fluiditate
    float totalDist = glm::distance(start, end);
    float maxLen = 20.0f;

    int segments = (int)std::ceil(totalDist / maxLen);
    if (segments < 1) segments = 1;

    glm::vec3 dir = glm::normalize(end - start);
    float segLen = totalDist / segments;

    for (int i = 0; i < segments; i++) {
        glm::vec3 s = start + dir * (segLen * (float)i);
        glm::vec3 e = start + dir * (segLen * (float)(i + 1));

        allRails.push_back(CreateSingleRail(s, e, type));
    }
}

// Initializare harta

void RailManager::Init()
{
    for (Rail* rail : allRails) delete rail;
    allRails.clear();
    stations.clear();

    float h = 0.5f;

	// Constructia traseului

    CreateSegment(glm::vec3(-4, h, 54), glm::vec3(-24, h, 54), "Surface");
    CreateSegment(glm::vec3(-24, h, 54), glm::vec3(-30, h, 54), "Surface");
    CreateSegment(glm::vec3(-30, h, 54), glm::vec3(-48, h, 54), "Tunnel");
    CreateSegment(glm::vec3(-48, h, 54), glm::vec3(-56, h, 54), "Surface");
    CreateSegment(glm::vec3(-56, h, 54), glm::vec3(-56, h, 38), "Surface");
    CreateSegment(glm::vec3(-56, h, 38), glm::vec3(-40, h, 38), "Surface");
    CreateSegment(glm::vec3(-40, h, 38), glm::vec3(-24, h, 38), "Surface");
    CreateSegment(glm::vec3(-24, h, 38), glm::vec3(-24, h, 30), "Surface");
    CreateSegment(glm::vec3(-24, h, 30), glm::vec3(-24, h, 18), "Bridge");
    CreateSegment(glm::vec3(-24, h, 18), glm::vec3(-24, h, 12), "Surface");
    CreateSegment(glm::vec3(-24, h, 12), glm::vec3(-38, h, 12), "Surface");
    CreateSegment(glm::vec3(-38, h, 12), glm::vec3(-38, h, 6), "Surface");
    CreateSegment(glm::vec3(-38, h, 6), glm::vec3(-38, h, -12), "Tunnel");
    CreateSegment(glm::vec3(-38, h, -12), glm::vec3(-38, h, -24), "Tunnel");
    CreateSegment(glm::vec3(-38, h, -24), glm::vec3(-38, h, -44), "Surface");
    CreateSegment(glm::vec3(-38, h, -44), glm::vec3(-20, h, -44), "Surface");
    CreateSegment(glm::vec3(-20, h, -44), glm::vec3(-13, h, -44), "Surface");
    CreateSegment(glm::vec3(-13, h, -44), glm::vec3(5, h, -44), "Tunnel");
    CreateSegment(glm::vec3(5, h, -44), glm::vec3(24, h, -44), "Bridge");
    CreateSegment(glm::vec3(24, h, -44), glm::vec3(30, h, -44), "Surface");
    CreateSegment(glm::vec3(30, h, -44), glm::vec3(40, h, -44), "Surface");
    CreateSegment(glm::vec3(40, h, -44), glm::vec3(40, h, -24), "Surface");
    CreateSegment(glm::vec3(40, h, -24), glm::vec3(40, h, -6), "Bridge");
    CreateSegment(glm::vec3(40, h, -6), glm::vec3(40, h, 4), "Surface");
    CreateSegment(glm::vec3(40, h, 4), glm::vec3(25, h, 4), "Surface");
    CreateSegment(glm::vec3(25, h, 4), glm::vec3(10, h, 4), "Surface");
    CreateSegment(glm::vec3(10, h, 4), glm::vec3(10, h, -14), "Surface");
    CreateSegment(glm::vec3(10, h, -14), glm::vec3(10, h, -30), "Surface");
    CreateSegment(glm::vec3(10, h, -30), glm::vec3(-24, h, -30), "Surface");
    CreateSegment(glm::vec3(-24, h, -30), glm::vec3(-24, h, -13), "Surface");
    CreateSegment(glm::vec3(-24, h, -13), glm::vec3(-24, h, -1), "Tunnel");
    CreateSegment(glm::vec3(-24, h, -1), glm::vec3(-24, h, 6), "Surface");
    CreateSegment(glm::vec3(-24, h, 6), glm::vec3(-8, h, 6), "Surface");
    CreateSegment(glm::vec3(-8, h, 6), glm::vec3(-8, h, 20), "Surface");
    CreateSegment(glm::vec3(-8, h, 20), glm::vec3(10, h, 20), "Surface");
    CreateSegment(glm::vec3(10, h, 20), glm::vec3(24, h, 20), "Surface");
    CreateSegment(glm::vec3(24, h, 20), glm::vec3(24, h, 30), "Surface");
    CreateSegment(glm::vec3(24, h, 30), glm::vec3(24, h, 36), "Bridge");
    CreateSegment(glm::vec3(24, h, 36), glm::vec3(30, h, 36), "Bridge");
    CreateSegment(glm::vec3(30, h, 36), glm::vec3(36, h, 36), "Surface");
    CreateSegment(glm::vec3(36, h, 36), glm::vec3(36, h, 54), "Surface");
    CreateSegment(glm::vec3(36, h, 54), glm::vec3(16, h, 54), "Surface");
    CreateSegment(glm::vec3(16, h, 54), glm::vec3(-4, h, 54), "Surface");

    // Legarea sinelor sa formeze un loop
    if (!allRails.empty()) {
        for (size_t i = 0; i < allRails.size() - 1; i++) {
            allRails[i]->next = allRails[i + 1];
        }
        allRails.back()->next = allRails[0];
        headRail = allRails[0];
    }

	// Adaugarea statiilor
    stations.push_back({ glm::vec3(-40, h, 40.0f),  "Gara 1", "box" });
    stations.push_back({ glm::vec3(-41, h, -37.0f), "Gara 2", "cylinder" });
    stations.push_back({ glm::vec3(43,  h, -30.0f), "Gara 3", "cone" });
    stations.push_back({ glm::vec3(13,  h, -12.0f), "Gara 4", "box" });
    stations.push_back({ glm::vec3(8,   h, 22.0f),  "Gara 5", "cone" });
    stations.push_back({ glm::vec3(16,  h, 56.5f),  "Gara 6", "cylinder" });
}

const std::vector<Rail*>& RailManager::GetRails() const { return allRails; }
const std::vector<Station>& RailManager::GetStations() const { return stations; }
Rail* RailManager::GetHeadRail() const { return headRail; }

glm::vec3 RailManager::GetTerrainColor(const std::string& railType) {
    if (railType == "Surface") return glm::vec3(0.3f, 0.8f, 0.3f);
    if (railType == "Bridge")  return glm::vec3(0.2f, 0.4f, 0.9f);
    if (railType == "Tunnel")  return glm::vec3(0.5f, 0.3f, 0.1f);
    return glm::vec3(0, 0, 0);
}