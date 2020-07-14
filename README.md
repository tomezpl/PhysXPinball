# PhysXPinball
This is a very basic Pinball-type 3D game written in C++. It uses the PhysX 3 SDK for real-time rigidbody physics simulation and a custom OpenGL renderer for 3D graphics.

It was written as part of my Physics Simulation coursework at the University of Lincoln and as such should be treated as a sample implementation of the PhysX SDK, rather than a full game.

## Building
The project is set up to build as a Visual Studio solution (MSBuild), as it uses NuGet to obtain the OpenGL libraries. 

**The PhysX 3.4.2 SDK location needs to be provided by environment variable *PHYSX_SDK* in order for the project to build.**

You can also find a binary release [here](https://github.com/tomezpl/PhysXPinball/releases).

## Usage
Running the executable will start the game.
### Controls
Hold and release **Right Shift** to launch the ball.

Press **Left Arrow** or **Right Arrow** to move the flippers.

![](https://i.imgur.com/NlBsb6A.gif)
