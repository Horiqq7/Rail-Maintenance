#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "RailManager.h"

namespace m1
{
    struct TrainCar {
        Rail* currentRail;
        float distanceOnRail;
        glm::vec3 position;
        float rotationAngle;
    };

    class Train {
    public:
        Train(float initialSpeed, Rail* startRail, RailManager* manager, int numberOfWagons, bool isExpress = false);
        ~Train();

        // Logica principala
        void Update(float deltaTime);
        void TogglePause();

        // Getters pentru starea trenului
        const std::vector<TrainCar>& GetCars() const;
        glm::vec3 GetLocomotivePosition() const;
        glm::vec3 GetLocomotiveForward() const;

        // Verificari specifice (Express / Avarii)
        bool IsExpress() const;
        bool IsStuckDueToBreak() const;
        bool HasWaitedTooLong() const;
        float GetCurrentWaitTime() const;

    private:
        // Functii ajutatoare interne
        void PickNewDestination();
        Rail* GetPreviousRail(Rail* current);
        void ComputeCarTransform(TrainCar& car);

    private:
        // Componente si Manageri
        RailManager* railManager;
        std::vector<TrainCar> cars;

        // Parametri de miscare si stare
        float speed;
        bool isPaused;
        bool isExpressTrain;
        bool isWaitingForRepair;
        float brokenRailWaitTimer;

        // Logica pentru statii
        bool isStoppedAtStation;
        float stationWaitTimer;
        float stationWaitDuration;
        std::string lastStationVisited;
        std::string targetStationName;
    };
}