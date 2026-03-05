#pragma once

#include <string>
#include <vector>
#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"

namespace m1
{
    class ShapeBuilder
    {
    public:
        static Mesh* CreateCylinder(const std::string& name, int segments);

        static Mesh* CreateBox(const std::string& name);
        static Mesh* CreateSquare(const std::string& name, float length);
        static Mesh* CreateCone(const std::string& name, float height, float radius, int slices);
        static Mesh* CreateArrow(const std::string& name);
    };
}