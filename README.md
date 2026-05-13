# Interactive 3D Art Museum

An interactive 3D art museum built with C++ and Qt6 OpenGL. Walk through three connected gallery rooms, view paintings on the walls, and examine a 3D sculpture displayed on a pedestal in the third room. Click on any painting to see a popup with the artwork's title, artist, and description.

## Features

- First-person navigation using WASD keys and mouse look
- Three connected gallery rooms with textured walls, floors, and ceilings
- Seven paintings displayed in frames on the walls, each clickable for artwork information
- A 3D sculpture (Old Arrow Maker, Smithsonian collection) loaded from a Wavefront `.obj` file and displayed on a wooden pedestal
- Phong lighting with two light sources — one for the main gallery rooms and one dedicated to the sculpture room
- Popup dialog system with artwork metadata triggered by mouse click

## How to Compile

**Requirements:**
- Qt 6.5 or later
- CMake 3.19 or later
- A C++17 compatible compiler (MinGW 64-bit on Windows, or equivalent)

**Steps:**

1. Clone the repository
2. Open Qt Creator and select **File → Open Project**, then navigate to `CMakeLists.txt`
3. Select your Qt kit (Desktop Qt 6.x MinGW 64-bit recommended)
4. Click **Build → Build All**
5. Click **Run**

> **Note:** The first full build may take several minutes due to the TinyObjLoader library included for 3D model parsing. Subsequent builds are significantly faster.

## Controls

| Input | Action |
|---|---|
| `W` `A` `S` `D` | Move forward / left / backward / right |
| Mouse movement | Look around |
| Left click (on painting) | View artwork information popup |
| Left click (elsewhere) | Lock mouse for navigation |
| `Escape` | Unlock mouse cursor |

## Project Structure

| File | Description |
|---|---|
| `MuseumWidget.cpp` | Core OpenGL widget, shader setup, lighting, texture loading, camera and input handling |
| `scene/RoomBuilder.cpp` | Constructs the museum room geometry |
| `scene/PaintingBuilder.cpp` | Places painting frames and artwork on walls |
| `scene/SculptureBuilder.cpp` | Loads and places the 3D sculpture using TinyObjLoader |
| `geometry/GeometryBuilder.cpp` | Reusable helper for building rectangles and other geometry |
| `rendering/Vertex.h` | Defines the Vertex struct used throughout the pipeline |
| `scene/tiny_obj_loader.h` | Single-header library for parsing Wavefront `.obj` files |
