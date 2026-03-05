#include "Tema2.h"
#include "ShapeBuilder.h"
#include "RailManager.h"
#include "Train.h"
#include <vector>
#include <string>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace m1;

static void RenderMeshColor(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color,
    const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    if (!mesh || !shader || !shader->program) return;

    shader->Use();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    GLint locObjectColor = glGetUniformLocation(shader->program, "object_color");
    if (locObjectColor != -1) {
        glUniform3f(locObjectColor, color.x, color.y, color.z);
    }
    mesh->Render();
}

Tema2::Tema2()
    : camera(nullptr)
    , projectionMatrix(1.0f)
    , renderCameraTarget(false)
    , is_orthographic(false)
    , fov(60.0f)
    , leftOrtho(-8.0f)
    , rightOrtho(8.0f)
    , bottomOrtho(-8.0f)
    , topOrtho(8.0f)
    , railManager(nullptr)
    , environment(nullptr)
    , textRenderer(nullptr)
{
    // Initializare variabile stare jucator
    controlDrezina = true;
    drezinaPosition = glm::vec3(0, 0.3f, 0);
    drezinaRotation = 0.0f;

    // Variabile gameplay
    isRepairing = false;
    repairTimer = 0.0f;
    railToRepair = nullptr;
    breakTimer = 0.0f;
    totalGameTime = 0.0f;

    isGamePaused = false;
    isGameOver = false;
}

Tema2::~Tema2()
{
    delete camera;
    for (Train* t : trains) { delete t; }
    trains.clear();

    delete railManager;
    delete environment;

    if (textRenderer) {
        delete textRenderer;
        textRenderer = nullptr;
    }
}

void Tema2::Init()
{
    // Configurare camera si proiectie
    fov = 60.0f;
    renderCameraTarget = false;
    leftOrtho = -8.0f; rightOrtho = 8.0f;
    topOrtho = 8.0f; bottomOrtho = -8.0f;
    is_orthographic = false;

    camera = new tema::Camera();
    camera->Set(glm::vec3(0, 15, 25), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    projectionMatrix = glm::perspective(RADIANS(fov), window->props.aspectRatio, 0.01f, 200.0f);

    // Generare meshe procedurale
    { Mesh* m = ShapeBuilder::CreateBox("box"); meshes[m->GetMeshID()] = m; }
    { Mesh* m = ShapeBuilder::CreateCylinder("cylinder", 32); meshes[m->GetMeshID()] = m; }
    { Mesh* m = ShapeBuilder::CreateCylinder("sphere", 32); meshes[m->GetMeshID()] = m; }
    { Mesh* m = ShapeBuilder::CreateCone("cone", 2.0f, 1.0f, 4); meshes[m->GetMeshID()] = m; }
    { Mesh* m = ShapeBuilder::CreateArrow("arrow"); meshes[m->GetMeshID()] = m; }

    // Initializare componente joc (sine, mediu)
    railManager = new RailManager();
    railManager->Init();

    environment = new Environment();
    environment->Init();

    // Initializare trenuri
    const std::vector<Rail*>& allRails = railManager->GetRails();
    if (!allRails.empty()) {
        // Tren 1
        Train* t1 = new Train(10.0f, allRails[0], railManager, 2, false);
        trains.push_back(t1);

        if (allRails.size() > 5) {
            // Tren 2
            int midIndex = allRails.size() / 2;
            Train* t2 = new Train(7.5f, allRails[midIndex], railManager, 3, false);
            trains.push_back(t2);

            // Tren Express
            int expressIndex = allRails.size() / 4;
            Train* tExpress = new Train(10.0f, allRails[expressIndex], railManager, 2, true);
            trains.push_back(tExpress);
        }
    }

    // Incarcare shader
    {
        Shader* shader = new Shader("Simple");
        shader->AddShader(PATH_JOIN(window->props.selfDir, "src", "lab_m1", "Tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, "src", "lab_m1", "Tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Initializare text renderer
    glm::ivec2 resolution = window->GetResolution();
    textRenderer = new gfxc::TextRenderer(window->props.selfDir, resolution.x, resolution.y);
    textRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "Hack-Bold.ttf"), 48);
}

void Tema2::FrameStart()
{
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}

void Tema2::Update(float deltaTimeSeconds)
{
    glm::ivec2 resolution = window->GetResolution();

    // Actualizare cronometru global
    if (!isGamePaused && !isGameOver) {
        totalGameTime += deltaTimeSeconds;
    }

    // Verificare conditii de Game Over (infrastructura distrusa)
    const std::vector<Rail*>& allRails = railManager->GetRails();
    int brokenCount = 0;
    for (Rail* r : allRails) {
        if (r->isBroken) brokenCount++;
    }

    if (!allRails.empty() && (float)brokenCount / allRails.size() > 0.25f) {
        isGameOver = true;
    }

    // Afisare ecran Game Over si oprire update
    if (isGameOver) {
        std::string message = "GAME OVER";
        float textScale = 2.0f;
        float estimatedCharWidth = 40.0f;
        float textWidth = message.length() * (estimatedCharWidth * textScale);
        float x = 375;
        float y = 300;

        textRenderer->RenderText(message, x, y, textScale, glm::vec3(1.0f, 1.0f, 1.0f));
        return;
    }

    // Update logica trenuri
    float maxWaitTime = 0.0f;
    for (Train* t : trains) {
        t->Update(deltaTimeSeconds);

        float w = t->GetCurrentWaitTime();
        if (w > maxWaitTime) maxWaitTime = w;

        // Verificari critice pentru trenuri
        if (t->IsExpress() && t->IsStuckDueToBreak()) {
            isGameOver = true;
        }
        if (t->HasWaitedTooLong()) {
            isGameOver = true;
        }
    }

    // Randare mediu inconjurator si trenuri
    if (environment) environment->Render(railManager, camera, projectionMatrix, meshes, shaders);
    RenderTrains();

    // Logica miscare Drezina si Camera
    float handleAngle = 0.0f;
    static float movementTimer = 0.0f;
    bool isMoving = false;

    if (controlDrezina) {
        float drezinaSpeed = 15.0f;
        float rotSpeed = 2.0f;
        float mapLimit = 60.0f;
        bool isRightClickHeld = window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT);

        if (!isGamePaused && !isRightClickHeld) {
            if (window->KeyHold(GLFW_KEY_W)) {
                drezinaPosition.x -= sin(drezinaRotation) * drezinaSpeed * deltaTimeSeconds;
                drezinaPosition.z -= cos(drezinaRotation) * drezinaSpeed * deltaTimeSeconds;
                isMoving = true;
            }
            if (window->KeyHold(GLFW_KEY_S)) {
                drezinaPosition.x += sin(drezinaRotation) * drezinaSpeed * deltaTimeSeconds;
                drezinaPosition.z += cos(drezinaRotation) * drezinaSpeed * deltaTimeSeconds;
                isMoving = true;
            }
            if (window->KeyHold(GLFW_KEY_A)) {
                drezinaRotation += rotSpeed * deltaTimeSeconds;
                isMoving = true;
            }
            if (window->KeyHold(GLFW_KEY_D)) {
                drezinaRotation -= rotSpeed * deltaTimeSeconds;
                isMoving = true;
            }
        }

        // Limitare pe harta
        if (drezinaPosition.x > mapLimit) drezinaPosition.x = mapLimit;
        if (drezinaPosition.x < -mapLimit) drezinaPosition.x = -mapLimit;
        if (drezinaPosition.z > mapLimit) drezinaPosition.z = mapLimit;
        if (drezinaPosition.z < -mapLimit) drezinaPosition.z = -mapLimit;

        // Update pozitie
        if (!isRightClickHeld) {
            glm::vec3 forward = glm::vec3(-sin(drezinaRotation), 0, -cos(drezinaRotation));
            glm::vec3 camPos = drezinaPosition - forward * 8.0f + glm::vec3(0, 5.0f, 0);
            camera->Set(camPos, drezinaPosition, glm::vec3(0, 1, 0));
        }
    }

    // Animatie manere drezina
    if (isMoving) {
        movementTimer += deltaTimeSeconds;
        handleAngle = sin(movementTimer * 10.0f);
    }
    else {
        handleAngle = 0.0f;
    }

    // Mecanica de avarii aleatoare
    if (!isGamePaused) {
        breakTimer += deltaTimeSeconds;
        if (breakTimer > 10.0f) {
            breakTimer = 0.0f;
            if (!allRails.empty()) {
                int idx = rand() % allRails.size();
                if (!allRails[idx]->isBroken) {
                    allRails[idx]->isBroken = true;
                    allRails[idx]->timeSinceBroken = 0.0f;
                    allRails[idx]->currentRepairAmount = 0.0f;
                }
            }
        }
        for (Rail* r : allRails) {
            if (r->isBroken) {
                r->timeSinceBroken += deltaTimeSeconds;
            }
            else {
                r->timeSinceBroken = 0.0f;
                r->currentRepairAmount = 0.0f;
            }
        }
    }

    // Mecanica de reparatii si UI bara progres
    if (controlDrezina && window->KeyHold(GLFW_KEY_F)) {
        Rail* nearestBrokenRail = nullptr;
        float minDst = 10000.0f;
        float repairRadius = 7.5f;

        for (Rail* rail : allRails) {
            if (rail->isBroken) {
                glm::vec3 railCenter = (rail->startPosition + rail->endPosition) * 0.5f;
                float dst = glm::distance(drezinaPosition, railCenter);
                if (dst < repairRadius && dst < minDst) {
                    minDst = dst;
                    nearestBrokenRail = rail;
                }
            }
        }

        if (nearestBrokenRail) {
            float baseTime = 0.5f;
            float penalty = nearestBrokenRail->timeSinceBroken * 0.02f;
            float totalRequiredTime = baseTime + penalty;

            nearestBrokenRail->currentRepairAmount += deltaTimeSeconds;

            float progress = nearestBrokenRail->currentRepairAmount / totalRequiredTime;
            if (progress > 1.0f) progress = 1.0f;

            handleAngle = sin(glfwGetTime() * 20.0f) * 0.5f;

            // Randare bara progres reparatie
            glm::mat4 uiProjection = glm::ortho(0.0f, (float)resolution.x, 0.0f, (float)resolution.y, -1.0f, 1.0f);
            glm::mat4 uiView = glm::mat4(1.0f);
            float barWidth = 300.0f;
            float barHeight = 30.0f;
            glm::vec3 centerScreen = glm::vec3(resolution.x / 2.0f, resolution.y / 2.0f, 0);

            // Fundal bara
            {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, centerScreen);
                model = glm::scale(model, glm::vec3(barWidth + 4.0f, barHeight + 4.0f, 1.0f));
                RenderMeshColor(meshes["box"], shaders["Simple"], model, glm::vec3(0.0f, 0.0f, 0.0f), uiView, uiProjection);
            }

            // Bara verde
            if (progress > 0.01f) {
                float currentWidth = barWidth * progress;
                glm::mat4 model = glm::mat4(1.0f);
                float leftEdge = centerScreen.x - (barWidth / 2.0f);
                model = glm::translate(model, glm::vec3(leftEdge + currentWidth / 2.0f, centerScreen.y, 0.1f));
                model = glm::scale(model, glm::vec3(currentWidth, barHeight, 1.0f));
                RenderMeshColor(meshes["box"], shaders["Simple"], model, glm::vec3(0.0f, 1.0f, 0.0f), uiView, uiProjection);
            }

            // Finalizare reparatie
            if (nearestBrokenRail->currentRepairAmount >= totalRequiredTime) {
                nearestBrokenRail->isBroken = false;
                nearestBrokenRail->timeSinceBroken = 0.0f;
                nearestBrokenRail->currentRepairAmount = 0.0f;
            }
        }
    }
    else {
        // Resetare progres daca se ia mana de pe tasta
        for (Rail* r : allRails) {
            if (r->isBroken) {
                r->currentRepairAmount = 0.0f;
            }
        }
    }

    // Randare elemente jucator
    RenderDrezina(drezinaPosition, drezinaRotation, handleAngle);

    if (is_orthographic)
        projectionMatrix = glm::ortho(leftOrtho, rightOrtho, bottomOrtho, topOrtho, 0.01f, 200.0f);
    else
        projectionMatrix = glm::perspective(RADIANS(fov), window->props.aspectRatio, 0.01f, 200.0f);

    RenderMinimap(allRails, trains, drezinaPosition, drezinaRotation, meshes["box"], meshes["cone"], meshes["cylinder"], shaders["Simple"], resolution);

    // Randare HUD (text informatii)
    float hudScale = 0.5f;
    glm::vec3 hudColor = glm::vec3(1.0f, 1.0f, 1.0f);
    float lineSpacing = 30.0f;
    float startY = 10.0f;
    float startX = 20.0f;

    int minutes = (int)totalGameTime / 60;
    int seconds = (int)totalGameTime % 60;
    std::string timeStr = "TIME:          " + std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
    textRenderer->RenderText(timeStr, startX, startY, hudScale, hudColor);

    glm::vec3 brokenColor = (brokenCount > 3) ? glm::vec3(1.0f, 0.2f, 0.2f) : glm::vec3(1.0f, 1.0f, 1.0f);
    std::string brokenStr = "BROKEN RAILS: " + std::to_string(brokenCount);
    textRenderer->RenderText(brokenStr, startX, startY + lineSpacing, hudScale, brokenColor);

    glm::vec3 waitColor = glm::vec3(1.0f, 1.0f, 1.0f);
    if (maxWaitTime > 10.0f) waitColor = glm::vec3(1.0f, 0.0f, 0.0f);
    else if (maxWaitTime > 5.0f) waitColor = glm::vec3(1.0f, 1.0f, 0.0f);

    char buffer[50];
    sprintf(buffer, "MAX WAIT:      %.1f s / 15.0 s", maxWaitTime);
    textRenderer->RenderText(std::string(buffer), startX, startY + lineSpacing * 2, hudScale, waitColor);
}

void Tema2::FrameEnd() {}

void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
        float cameraSpeed = 20.0f;
        if (window->KeyHold(GLFW_KEY_W)) camera->TranslateForward(cameraSpeed * deltaTime);
        if (window->KeyHold(GLFW_KEY_S)) camera->TranslateForward(-cameraSpeed * deltaTime);
        if (window->KeyHold(GLFW_KEY_A)) camera->TranslateRight(-cameraSpeed * deltaTime);
        if (window->KeyHold(GLFW_KEY_D)) camera->TranslateRight(cameraSpeed * deltaTime);
        if (window->KeyHold(GLFW_KEY_Q)) camera->TranslateUpward(-cameraSpeed * deltaTime);
        if (window->KeyHold(GLFW_KEY_E)) camera->TranslateUpward(cameraSpeed * deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_Z)) fov -= 50.0f * deltaTime;
    if (window->KeyHold(GLFW_KEY_X)) fov += 50.0f * deltaTime;
}

void Tema2::OnKeyPress(int key, int mods)
{
    if (key == GLFW_KEY_SPACE) {
        isGamePaused = !isGamePaused;
        for (Train* t : trains) t->TogglePause();
    }
    if (key == GLFW_KEY_T) renderCameraTarget = !renderCameraTarget;
    if (key == GLFW_KEY_O) is_orthographic = true;
    if (key == GLFW_KEY_P) is_orthographic = false;
}

void Tema2::OnKeyRelease(int key, int mods) {}

void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
        float sens = 0.002f;
        camera->RotateFirstPerson_OX(-deltaY * sens);
        camera->RotateFirstPerson_OY(-deltaX * sens);
    }
}

void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) {}
void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) {}
void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) {}
void Tema2::OnWindowResize(int width, int height) {}

void Tema2::RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix)
{
    if (!mesh || !shader || !shader->program) return;
    shader->Use();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    GLint locObjectColor = glGetUniformLocation(shader->program, "object_color");
    if (locObjectColor != -1) glUniform3f(locObjectColor, 0.5f, 0.5f, 0.5f);
    mesh->Render();
}

void Tema2::RenderTrains()
{
    for (Train* t : trains) {
        if (!t) continue;
        const std::vector<TrainCar>& cars = t->GetCars();

        glm::vec3 cBody, cCabin, cEngine, cWagon;

        // Setare culori in functie de tipul trenului
        if (t->IsExpress()) {
            cBody = glm::vec3(0.9f, 0.1f, 0.1f);
            cCabin = glm::vec3(1.0f, 1.0f, 1.0f);
            cEngine = glm::vec3(0.9f, 0.9f, 0.9f);
            cWagon = glm::vec3(1.0f, 1.0f, 1.0f);
        }
        else {
            cBody = glm::vec3(1.0f, 1.0f, 0.0f);
            cCabin = glm::vec3(0.0f, 1.0f, 0.0f);
            cEngine = glm::vec3(0.0f, 0.0f, 1.0f);
            cWagon = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        for (size_t i = 0; i < cars.size(); i++) {
            const TrainCar& car = cars[i];
            glm::mat4 model = glm::mat4(1);
            model = glm::translate(model, car.position);
            model = glm::rotate(model, car.rotationAngle, glm::vec3(0, 1, 0));

            if (i == 0) RenderLocomotive(model, cBody, cCabin, cEngine);
            else RenderWagon(model, cWagon);
        }
    }
}

void Tema2::RenderLocomotive(const glm::mat4& modelMatrix, glm::vec3 colorBody, glm::vec3 colorCabin, glm::vec3 colorEngine)
{
    float sY = 0.75f; float sZ = 0.75f;
    // Sasiu
    {
        glm::mat4 chassis = glm::translate(modelMatrix, glm::vec3(0, 0.525f, 0));
        chassis = glm::scale(chassis, glm::vec3(1.2f, 0.2f * sY, 4.2f * sZ));
        RenderMeshColor(meshes["box"], shaders["Simple"], chassis, colorBody, camera->GetViewMatrix(), projectionMatrix);
    }
    // Cabina
    {
        glm::mat4 cabin = glm::translate(modelMatrix, glm::vec3(0, 1.125f, -1.05f));
        cabin = glm::scale(cabin, glm::vec3(1.2f, 1.4f * sY, 1.4f * sZ));
        RenderMeshColor(meshes["box"], shaders["Simple"], cabin, colorCabin, camera->GetViewMatrix(), projectionMatrix);
    }
    // Motor
    {
        glm::mat4 engine = glm::translate(modelMatrix, glm::vec3(0, 0.9f, 0.45f));
        engine = glm::rotate(engine, RADIANS(90), glm::vec3(1, 0, 0));
        engine = glm::scale(engine, glm::vec3(0.5f * sY, 1.3f * sZ, 0.5f * sY));
        RenderMeshColor(meshes["cylinder"], shaders["Simple"], engine, colorEngine, camera->GetViewMatrix(), projectionMatrix);
    }
    // Cos de fum
    {
        glm::mat4 tip = glm::translate(modelMatrix, glm::vec3(0, 0.9f, 1.5f));
        tip = glm::rotate(tip, RADIANS(90), glm::vec3(1, 0, 0));
        tip = glm::scale(tip, glm::vec3(0.2f * sY, 0.1f, 0.2f * sY));
        RenderMeshColor(meshes["cylinder"], shaders["Simple"], tip, colorBody, camera->GetViewMatrix(), projectionMatrix);
    }
    // Roti
    float wheelY = 0.3f * sY; float wheelX = 0.65f;
    for (int i = -3; i <= 3; i++) {
        float zOffset = (i * 0.63f) * sZ;
        float wheelRadius = 0.3f * sY;

        glm::mat4 wheelL = glm::translate(modelMatrix, glm::vec3(wheelX, wheelY, zOffset));
        wheelL = glm::rotate(wheelL, RADIANS(90), glm::vec3(0, 0, 1));
        wheelL = glm::scale(wheelL, glm::vec3(wheelRadius, 0.1f, wheelRadius));
        RenderMeshColor(meshes["cylinder"], shaders["Simple"], wheelL, glm::vec3(1.0f, 0.0f, 0.0f), camera->GetViewMatrix(), projectionMatrix);

        glm::mat4 wheelR = glm::translate(modelMatrix, glm::vec3(-wheelX, wheelY, zOffset));
        wheelR = glm::rotate(wheelR, RADIANS(90), glm::vec3(0, 0, 1));
        wheelR = glm::scale(wheelR, glm::vec3(wheelRadius, 0.1f, wheelRadius));
        RenderMeshColor(meshes["cylinder"], shaders["Simple"], wheelR, glm::vec3(1.0f, 0.0f, 0.0f), camera->GetViewMatrix(), projectionMatrix);
    }
}

void Tema2::RenderWagon(const glm::mat4& modelMatrix, glm::vec3 colorBody)
{
    float sY = 0.75f; float sZ = 0.75f;
    // Placa
    {
        glm::mat4 wagonPlate = glm::translate(modelMatrix, glm::vec3(0, 0.525f, 0));
        wagonPlate = glm::scale(wagonPlate, glm::vec3(1.25f, 0.2f * sY, 4.5f * sZ));
        RenderMeshColor(meshes["box"], shaders["Simple"], wagonPlate, glm::vec3(0.3f, 0.3f, 0.3f), camera->GetViewMatrix(), projectionMatrix);
    }
    // Corp vagon
    {
        glm::mat4 wagonBody = glm::translate(modelMatrix, glm::vec3(0, 1.125f, 0));
        wagonBody = glm::scale(wagonBody, glm::vec3(1.2f, 1.4f * sY, 4.5f * sZ));
        RenderMeshColor(meshes["box"], shaders["Simple"], wagonBody, colorBody, camera->GetViewMatrix(), projectionMatrix);
    }
    // Roti
    float wheelY = 0.3f * sY; float wheelX = 0.65f; float wZ = 1.8f * sZ;
    glm::vec3 wagonWheelOffsets[4] = { {wheelX, wheelY,  wZ}, {wheelX, wheelY, -wZ}, {-wheelX, wheelY,  wZ}, {-wheelX, wheelY, -wZ} };
    float wheelRadius = 0.3f * sY;
    for (int i = 0; i < 4; i++) {
        glm::mat4 wheelModel = glm::translate(modelMatrix, wagonWheelOffsets[i]);
        wheelModel = glm::rotate(wheelModel, RADIANS(90), glm::vec3(0, 0, 1));
        wheelModel = glm::scale(wheelModel, glm::vec3(wheelRadius, 0.1f, wheelRadius));
        RenderMeshColor(meshes["cylinder"], shaders["Simple"], wheelModel, glm::vec3(1.0f, 0.0f, 0.0f), camera->GetViewMatrix(), projectionMatrix);
    }
}

void Tema2::RenderDrezina(const glm::vec3& position, float rotationAngle, float handleAngle)
{
    glm::mat4 baseModel = glm::mat4(1);
    baseModel = glm::translate(baseModel, position);
    baseModel = glm::rotate(baseModel, rotationAngle, glm::vec3(0, 1, 0));
    baseModel = glm::rotate(baseModel, RADIANS(180), glm::vec3(0, 1, 0));

    glm::vec3 orangeColor = glm::vec3(1.0f, 0.5f, 0.0f);
    glm::vec3 darkGrey = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 greenColor = glm::vec3(0.0f, 1.0f, 0.0f);

    float platY = 0.35f; float platHeight = 0.2f;
    // Platforma
    {
        glm::mat4 model = glm::translate(baseModel, glm::vec3(0, platY, 0));
        model = glm::scale(model, glm::vec3(1.5f, platHeight, 2.0f));
        RenderMeshColor(meshes["box"], shaders["Simple"], model, orangeColor, camera->GetViewMatrix(), projectionMatrix);
    }
    // Stalpi sustinere
    float platTop = platY + (platHeight / 2.0f);
    float basePostHeight = 0.75f;
    {
        float postY = platTop + (basePostHeight / 2.0f);
        glm::mat4 model = glm::translate(baseModel, glm::vec3(0, postY, 0));
        model = glm::scale(model, glm::vec3(0.4f, basePostHeight, 0.4f));
        RenderMeshColor(meshes["box"], shaders["Simple"], model, orangeColor, camera->GetViewMatrix(), projectionMatrix);
    }
    // Stalpi superiori
    float topPostHeight = 0.75f;
    {
        float postY = platTop + basePostHeight + (topPostHeight / 2.0f);
        glm::mat4 model = glm::translate(baseModel, glm::vec3(0, postY, 0));
        model = glm::scale(model, glm::vec3(0.2f, topPostHeight, 0.2f));
        RenderMeshColor(meshes["box"], shaders["Simple"], model, darkGrey, camera->GetViewMatrix(), projectionMatrix);
    }
    // Maner animat
    float handleY = platTop + basePostHeight + topPostHeight;
    {
        glm::mat4 handleMatrix = glm::translate(baseModel, glm::vec3(0, handleY, 0));
        handleMatrix = glm::rotate(handleMatrix, handleAngle, glm::vec3(1, 0, 0));

        {
            glm::mat4 bar = glm::scale(handleMatrix, glm::vec3(0.15f, 0.15f, 1.2f));
            RenderMeshColor(meshes["box"], shaders["Simple"], bar, darkGrey, camera->GetViewMatrix(), projectionMatrix);
        }
        {
            glm::mat4 h1 = glm::translate(handleMatrix, glm::vec3(0, 0, 0.675f));
            h1 = glm::scale(h1, glm::vec3(1.2f, 0.15f, 0.15f));
            RenderMeshColor(meshes["box"], shaders["Simple"], h1, greenColor, camera->GetViewMatrix(), projectionMatrix);

            glm::mat4 h2 = glm::translate(handleMatrix, glm::vec3(0, 0, -0.675f));
            h2 = glm::scale(h2, glm::vec3(1.2f, 0.15f, 0.15f));
            RenderMeshColor(meshes["box"], shaders["Simple"], h2, greenColor, camera->GetViewMatrix(), projectionMatrix);
        }
    }
    // Roti
    float wX = 0.85f; float wZ = 0.8f;
    glm::vec3 wheelOffsets[4] = { {wX, 0.15f, wZ}, {wX, 0.15f, -wZ}, {-wX, 0.15f, wZ}, {-wX, 0.15f, -wZ} };
    for (int i = 0; i < 4; i++) {
        glm::mat4 model = glm::translate(baseModel, wheelOffsets[i]);
        model = glm::rotate(model, RADIANS(90), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(0.3f, 0.1f, 0.3f));
        RenderMeshColor(meshes["cylinder"], shaders["Simple"], model, darkGrey, camera->GetViewMatrix(), projectionMatrix);
    }
}

void Tema2::RenderMinimap(const std::vector<Rail*>& rails, const std::vector<Train*>& trains, const glm::vec3& playerPos, float playerRotation, Mesh* boxMesh, Mesh* coneMesh, Mesh* cylinderMesh, Shader* shader, glm::ivec2 resolution)
{
    // Configurare viewport separat pentru harta
    float minimapSize = 250.0f;
    float margin = 20.0f;
    float mapRadius = 65.0f;

    glViewport((GLint)margin, (GLint)margin, (GLint)minimapSize, (GLint)minimapSize);
    glm::mat4 miniProjection = glm::ortho(-mapRadius, mapRadius, -mapRadius, mapRadius, 0.1f, 100.0f);
    glm::mat4 miniView = glm::lookAt(glm::vec3(0, 50, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1));

    glClear(GL_DEPTH_BUFFER_BIT);

    // Fundal harta (negru)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0, -5.0f, 0));
        model = glm::scale(model, glm::vec3(mapRadius * 2.2f, 1.0f, mapRadius * 2.2f));
        RenderMeshColor(boxMesh, shader, model, glm::vec3(0, 0, 0), miniView, miniProjection);
    }

    // Randare teren stilizat (grid)
    std::vector<std::string> layout = {
        "MMMMMMMMMMMMMMGGGGGG", "MMMMMMMMMMMGGGGGGGGG", "GGGGGGGGMMMWWWGGGGGG", "GGGGGGGGMMMWWWGGGGGG",
        "GGGGGGGGGGGGWWGGGGGG", "GGGGGGGGGGGGGGGGGGGG", "MMMMMGGGGGGGGGGGWWWG", "MMMMMGGGGGGGGGGGWWWG",
        "MMMMMMMGGGGGGGGGWWWG", "MMMMMMMGGGGGGGGGGGGG", "GGGMMGGGGGGGGGGGGGGG", "GGGGGGGGGGGGGGGGGGGG",
        "GGGGGGGGGGGGGGGGGMMM", "GGGGGWWGGGGGGGGGGMMM", "GGGGWWWWGGGGGGGGMMMM", "GGGGGGGGGGGWWWWGGGMM",
        "GGGGGGGGGGGGWWWGGGMM", "GGMMGGGGGGGGGGGGGGGG", "GGMMMGGGGGGGGGGGGGGG", "GGMMMGGGGGGGGGGGGGGG"
    };

    int rows = layout.size();
    int cols = layout[0].size();
    float worldSize = 120.0f;
    float cellSize = worldSize / cols;
    float offset = worldSize / 2.0f;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            char type = layout[i][j];
            glm::vec3 color;
            if (type == 'W') color = glm::vec3(0.2f, 0.4f, 0.9f);
            else if (type == 'M') color = glm::vec3(0.4f, 0.25f, 0.15f);
            else color = glm::vec3(0.3f, 0.8f, 0.3f);

            float x = (j * cellSize) - offset + (cellSize / 2.0f);
            float z = (i * cellSize) - offset + (cellSize / 2.0f);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, 0.0f, z));
            model = glm::scale(model, glm::vec3(cellSize + 0.2f, 0.1f, cellSize + 0.2f));
            RenderMeshColor(boxMesh, shader, model, color, miniView, miniProjection);
        }
    }

    // Randare sine pe harta
    for (Rail* rail : rails) {
        glm::vec3 center = (rail->startPosition + rail->endPosition) * 0.5f;
        glm::vec3 diff = rail->endPosition - rail->startPosition;
        float length = glm::length(diff);
        float angle = atan2(diff.x, diff.z);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, center);
        model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
        model[3][1] = 0.5f;

        glm::vec3 color;
        glm::vec3 scaleVector;

        if (rail->isBroken) {
            color = glm::vec3(1.0f, 0.0f, 0.0f);
            scaleVector = glm::vec3(1.5f, 1.5f, length);
        }
        else if (rail->type == "Tunnel") {
            color = glm::vec3(0.0f, 0.0f, 0.0f);
            scaleVector = glm::vec3(2.5f, 2.5f, length);
        }
        else if (rail->type == "Bridge") {
            color = glm::vec3(0.6f, 0.6f, 0.6f);
            scaleVector = glm::vec3(1.5f, 1.5f, length);
        }
        else {
            color = glm::vec3(0.6f, 0.4f, 0.2f);
            scaleVector = glm::vec3(1.5f, 1.5f, length);
        }
        model = glm::scale(model, scaleVector);
        RenderMeshColor(boxMesh, shader, model, color, miniView, miniProjection);
    }

    // Randare statii
    const std::vector<Station>& stations = railManager->GetStations();
    for (const auto& station : stations) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(station.position.x, 2.5f, station.position.z));

        Mesh* meshToRender = boxMesh;
        glm::vec3 color = glm::vec3(1, 1, 1);
        glm::vec3 scale = glm::vec3(1, 1, 1);

        if (station.meshType == "box") {
            meshToRender = boxMesh;
            color = glm::vec3(0.8f, 0.4f, 0.4f);
            scale = glm::vec3(4.5f, 2.0f, 3.0f);
        }
        else if (station.meshType == "cone") {
            meshToRender = coneMesh;
            color = glm::vec3(0.9f, 0.8f, 0.2f);
            scale = glm::vec3(2.5f, 2.5f, 2.5f);
        }
        else {
            meshToRender = cylinderMesh;
            color = glm::vec3(0.3f, 0.6f, 0.9f);
            scale = glm::vec3(2.5f, 2.0f, 2.5f);
        }
        model = glm::scale(model, scale);
        RenderMeshColor(meshToRender, shader, model, color, miniView, miniProjection);
    }

    // Randare trenuri pe harta
    for (Train* t : trains) {
        glm::vec3 trainColor = t->IsExpress() ? glm::vec3(0.9f, 0.1f, 0.1f) : glm::vec3(0.0f, 1.0f, 0.0f);
        const std::vector<TrainCar>& cars = t->GetCars();
        for (const auto& car : cars) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(car.position.x, 1.5f, car.position.z));
            model = glm::rotate(model, car.rotationAngle, glm::vec3(0, 1, 0));
            model = glm::scale(model, glm::vec3(2.2f, 1.0f, 3.5f));
            RenderMeshColor(boxMesh, shader, model, trainColor, miniView, miniProjection);
        }
    }

    // Randare iconita jucator
    {
        glm::vec3 playerColor = glm::vec3(1.0f, 0.5f, 0.0f);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(playerPos.x, 2.5f, playerPos.z));
        model = glm::rotate(model, playerRotation, glm::vec3(0, 1, 0));

        if (meshes.find("arrow") != meshes.end()) {
            model = glm::scale(model, glm::vec3(2.5f, 1.0f, 2.5f));
            RenderMeshColor(meshes["arrow"], shader, model, playerColor, miniView, miniProjection);
        }
        else {
            model = glm::rotate(model, RADIANS(-90), glm::vec3(1, 0, 0));
            model = glm::scale(model, glm::vec3(5.0f, 0.2f, 8.0f));
            Mesh* ptrMesh = coneMesh ? coneMesh : boxMesh;
            RenderMeshColor(ptrMesh, shader, model, playerColor, miniView, miniProjection);
        }
    }

    // Restaurare viewport initial
    glViewport(0, 0, resolution.x, resolution.y);
}