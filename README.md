# Rail Maintenance Simulator

A 3D simulation and strategy game developed in **C++** using **OpenGL**. The player takes on the role of a railway mechanic tasked with maintaining the track infrastructure while autonomous trains continuously navigate between stations.

This project showcases 3D rendering techniques, autonomous agent behaviors, dynamic camera systems, and real-time state management.

## 🚀 Gameplay Mechanics

* **Dynamic Degradation System**: Railway segments degrade randomly over time. Damage is visually indicated through geometric deformations and color/flickering effects.
* **Handcar Repairs**: The player controls an animated handcar using standard `WASD` movement and interacts with damaged segments to repair them (using the `F` key).
* **Autonomous Trains**: AI-driven locomotives navigate the track network, automatically halting before damaged rails to prevent derailments. They dock at a minimum of three distinct stations before receiving new destinations.
* **Game Over Conditions**: The simulation ends if a train is forced to wait for repairs for more than 30 seconds, or if the overall track degradation exceeds 50%.

## 🧠 Technical Implementation

* **3D Rendering**: Implements a complex track layout featuring both ground-level and suspended/elevated railway portions.
* **Camera Control**: Features a dynamic Third-Person (TPS) camera perspective that smoothly tracks the player's handcar.
* **Procedural Animations**: Includes kinematic animations, such as the swinging motion of the handcar's lever and real-time geometric transformations for breaking/repairing rail segments.
* **UI & Minimap**: Features a 2D graphical overlay (minimap) indicating the real-time status of all track segments (functional vs. damaged) and the player's current position.

## 💻 Build and Run

To compile and run the game locally, you need a C++ compiler, **CMake**, and an OpenGL-compatible environment.

```bash
# Example build steps (assuming CMake and dependencies are installed)
mkdir build && cd build
cmake ..
make
# Run the executable generated in the build/bin folder