# gl-sprint

Starter project for the OpenGL Week Sprint. Each day has exercise code for you to
write and a test program that checks it on your GPU, so you don't have to judge
your own work by eye.

## Setup on CachyOS

```sh
sudo pacman -S --needed base-devel cmake ninja glfw glm mesa-utils
```

glad (OpenGL 3.3 core loader) and the stb image headers are already in `external/`.

## Build and run from a terminal

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build

./build/day1              # your Day 1 program
./build/day1_tests        # the automatic checks for Day 1
./build/day1_tests wire   # only checks whose name contains "wire"
./build/day1_tests --list # list the checks
ctest --test-dir build --output-on-failure   # all days at once
```

The tests open a hidden window, so run them from your desktop session.

## In CLion

1. File → Open, pick this folder, "Open as CMake project".
2. Settings → Build, Execution, Deployment → CMake: Debug profile, generator Ninja.
3. Pick `day1_tests` in the run configuration list and press Shift+F10.
   The output shows PASS or FAIL for every check, with what was expected.
4. Pick `day1` to run your interactive program.
5. If your code crashes, run the test target with the debugger (Shift+F9) to see the line.

You don't need to set a working directory: shader and image paths go through
`SPRINT_DAY_FILE("...")`, which CMake points at the right day folder.

CLion also creates an "All CTest" configuration that runs every day.

## What is where

| Folder | Who edits it |
|---|---|
| `days/dayN/*.hpp` | Read these. They say exactly what each function must do. Keep the public names and parameters, the tests call them. |
| `days/dayN/*.cpp`, `days/dayN/shaders/*` | **You.** Replace every `TODO`. |
| `days/dayN/main.cpp` | **You.** The interactive program for that day. |
| `days/dayN/assets/` | Put the LearnOpenGL images here (`container.jpg`, `awesomeface.png` on Day 2, `container2.png`, `container2_specular.png` on Day 5). |
| `tests/` | Provided. The checks for each day. |
| `framework/` | Provided. Test runner, offscreen rendering, and a window helper for Days 2 to 5. |
| `external/` | glad and stb. |

Each day can use the code of the days before it: Day 3 uses your Day 2 `Shader`,
Day 5 uses your camera from Day 4. So finish the days in order.

## How the checks work

Every check renders into an offscreen framebuffer, reads pixels back with
`glReadPixels`, and compares them with what the exercise asks for. The lighting
checks on Days 4 and 5 compute the expected color of each pixel on the CPU with
the formulas written in `lighting.hpp` and `lights.hpp`. Any OpenGL error your
code raises during a check makes that check fail.

Some criteria can't be checked automatically, such as whether keys toggle once per
press or whether you can explain the code. Those are listed under "Check by eye" in
the plan.

## Starting over on a day

The tests never change your code. To start a day again, restore its folder from
the zip.
