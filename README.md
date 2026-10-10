# Railhead: the Gantry C++ example

A crew of one to eight keeps a freight train running across a continent that loads as it goes.
Every stretch of track is a Gantry experience; the world outside the windows changes in tunnels
while the train keeps moving.

This is the C++ companion to [the Blueprint example](https://github.com/Private-Asylum/UE-Gantry-Examples).
Where that one proves everything works without a line of code, this one shows the code side: the
game's own subclasses of Gantry's subsystems and asset types, C++ stages and game feature actions,
a Slate loading screen, every build target, and mods that carry compiled modules.

## Requirements

- Unreal Engine 5.6, 5.7 or 5.8, with Gantry installed from Fab.
- A C++ toolchain for your platform.
- Client and Server targets need an engine built with them (a source build); the Game and Editor
  targets build on a launcher install.
