#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <string>

namespace m1
{
    struct Rail {
        glm::vec3 startPosition;
        glm::vec3 endPosition;
        std::string type;       // Surface, Bridge, Tunnel
        Rail* next;

        bool isBroken;
        float timeSinceBroken;
        float currentRepairAmount;
    };

    struct Station {
        glm::vec3 position;
        std::string name;
        std::string meshType;
    };

    class RailManager
    {
    public:
        RailManager();
        ~RailManager();

        void Init();

        const std::vector<Rail*>& GetRails() const;
        const std::vector<Station>& GetStations() const;
        Rail* GetHeadRail() const;

        static glm::vec3 GetTerrainColor(const std::string& railType);

    private:
        Rail* CreateSingleRail(glm::vec3 start, glm::vec3 end, std::string type);
        void CreateSegment(glm::vec3 start, glm::vec3 end, std::string type);

        std::vector<Rail*> allRails;
        Rail* headRail;
        std::vector<Station> stations;
    };
}