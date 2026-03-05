#include "Train.h"
#include <iostream>
#include <cmath> 

using namespace m1;

// Constructor: Initializare locomotiva si vagoane
Train::Train(float initialSpeed, Rail* startRail, RailManager* manager, int numberOfWagons, bool isExpress)
    : speed(initialSpeed), isPaused(false), railManager(manager), isExpressTrain(isExpress)
{
    float distBetweenCars = 4.0f;
    isWaitingForRepair = false;
    brokenRailWaitTimer = 0.0f;

    isStoppedAtStation = false;
    stationWaitTimer = 0.0f;
    stationWaitDuration = 0.0f;
    lastStationVisited = "";

    PickNewDestination();

    // Configurare Locomotiva
    TrainCar loco;
    loco.currentRail = startRail;
    loco.distanceOnRail = 20.0f;
    ComputeCarTransform(loco);
    cars.push_back(loco);

    // Configurare Vagoane (calculam pozitia in spate urmarind sinele invers)
    Rail* cursorRail = startRail;
    float cursorDist = loco.distanceOnRail;

    for (int i = 0; i < numberOfWagons; i++) {
        cursorDist -= distBetweenCars;
        while (cursorDist < 0) {
            Rail* prev = GetPreviousRail(cursorRail);
            if (!prev) { cursorDist = 0.0f; break; }

            cursorRail = prev;
            float prevLen = glm::distance(prev->startPosition, prev->endPosition);
            cursorDist += prevLen;
        }

        TrainCar wagon;
        wagon.currentRail = cursorRail;
        wagon.distanceOnRail = cursorDist;
        ComputeCarTransform(wagon);
        cars.push_back(wagon);
    }
}

Train::~Train() {}

void Train::TogglePause() { isPaused = !isPaused; }

bool Train::IsExpress() const { return isExpressTrain; }

bool Train::IsStuckDueToBreak() const { return isWaitingForRepair; }

bool Train::HasWaitedTooLong() const { return brokenRailWaitTimer > 15.0f; }

float Train::GetCurrentWaitTime() const { return brokenRailWaitTimer; }

const std::vector<TrainCar>& Train::GetCars() const { return cars; }

glm::vec3 Train::GetLocomotivePosition() const {
    if (cars.empty()) return glm::vec3(0);
    return cars[0].position;
}

// Cauta sina anterioara pentru a putea plasa vagoanele
Rail* Train::GetPreviousRail(Rail* current) {
    if (!current || !railManager) return nullptr;
    const std::vector<Rail*>& allRails = railManager->GetRails();
    for (Rail* r : allRails) {
        if (r->next == current) return r;
    }
    return nullptr;
}

// Calculeaza pozitia 3D si rotatia pe baza distantei pe sina
void Train::ComputeCarTransform(TrainCar& car) {
    if (!car.currentRail) return;

    float railLen = glm::distance(car.currentRail->startPosition, car.currentRail->endPosition);
    float t = (railLen > 0.001f) ? (car.distanceOnRail / railLen) : 0.0f;

    car.position = car.currentRail->startPosition * (1.0f - t) + car.currentRail->endPosition * t;

    glm::vec3 direction = glm::normalize(car.currentRail->endPosition - car.currentRail->startPosition);
    car.rotationAngle = atan2(direction.x, direction.z);
}

void Train::PickNewDestination() {
    const std::vector<Station>& stations = railManager->GetStations();
    if (stations.empty()) return;

    int randomIndex = rand() % stations.size();
    if (stations.size() > 1) {
        while (stations[randomIndex].name == lastStationVisited) {
            randomIndex = rand() % stations.size();
        }
    }
    targetStationName = stations[randomIndex].name;
}

// Logica Principala (Update)

void Train::Update(float deltaTime)
{
    if (isPaused) return;

    bool currentlyWaitingForBreak = false;

	// Oprire in statie
    if (isStoppedAtStation) {
        stationWaitTimer += deltaTime;
        if (stationWaitTimer >= stationWaitDuration) {
            isStoppedAtStation = false;

            PickNewDestination();
        }
        return;
    }

    // Verificare sine stricate
    if (!cars.empty()) {
        TrainCar& loco = cars[0];

        // Verificam daca sina curenta sau urmatoarea e stricata
        if (loco.currentRail) {
            if (loco.currentRail->isBroken) {
                currentlyWaitingForBreak = true;
            }
            else {
                float currentLen = glm::distance(loco.currentRail->startPosition, loco.currentRail->endPosition);
                // Daca suntem aproape de capat, verificam urmatoarea sina
                if (loco.distanceOnRail > currentLen - 2.0f) {
                    if (loco.currentRail->next && loco.currentRail->next->isBroken) {
                        currentlyWaitingForBreak = true;
                    }
                }
            }
        }

        // Verificam daca am ajuns in statia destinatie
        if (loco.currentRail) {
            const std::vector<Station>& stations = railManager->GetStations();
            glm::vec3 trainDir = glm::normalize(loco.currentRail->endPosition - loco.currentRail->startPosition);

            for (const auto& station : stations) {

                glm::vec3 toStation = station.position - loco.position;
                float dist = glm::length(toStation);

                if (dist < 6.0f) {
                    // Produs scalar pentru a vedea daca am trecut de centrul statiei
                    float dotProd = glm::dot(trainDir, toStation);

                    // Verificam sa nu ne oprim instantaneu in aceeasi statie din care tocmai am plecat
                    if (dotProd < -1.0f) {
                        if (station.name != lastStationVisited) {
                            isStoppedAtStation = true;
                            lastStationVisited = station.name;
                            stationWaitTimer = 0.0f;

                            // Trenurile express stau mai putin
                            float baseWait = isExpressTrain ? 1.0f : 2.0f;
                            stationWaitDuration = baseWait + (rand() % 200) / 100.0f;
                        }
                    }
                }
            }
        }
    }

    // Actualizare flag-uri avarie
    isWaitingForRepair = currentlyWaitingForBreak;

    if (currentlyWaitingForBreak) {
        brokenRailWaitTimer += deltaTime;
        return; // Trenul nu se misca daca e blocat
    }
    else {
        brokenRailWaitTimer = 0.0f;
    }

    // Actualizare pozitie tren (Miscare)
    for (auto& car : cars) {
        if (!car.currentRail) continue;

        car.distanceOnRail += speed * deltaTime;

        float currentLen = glm::distance(car.currentRail->startPosition, car.currentRail->endPosition);

        // Trecerea pe urmatoarea sina
        if (car.distanceOnRail >= currentLen) {
            float surplus = car.distanceOnRail - currentLen;
            if (car.currentRail->next) {
                car.currentRail = car.currentRail->next;
                car.distanceOnRail = surplus;
            }
            else {
                car.distanceOnRail = currentLen;
            }
        }
        ComputeCarTransform(car);
    }
}