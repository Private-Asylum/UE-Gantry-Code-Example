# Railhead: the Gantry C++ example

A crew of one to eight keeps a freight train running across a continent that loads as it goes.
Every stretch of track is a Gantry experience; the world outside the windows changes in tunnels
while the train keeps moving.

This is the C++ companion to [the Blueprint example](https://github.com/Private-Asylum/UE-Gantry-Examples).
Where that one proves everything works without a line of code, this one shows the code side: the
game's own subclasses of Gantry's subsystems and asset types, C++ stages and game feature actions,
a Slate loading screen, every build target, and mods that build on the game's declared API.

## Requirements

- Unreal Engine 5.6, 5.7 or 5.8, with [Gantry from Fab][gantry-fab] (or copied into `Plugins/`).
- A C++ toolchain for your platform.
- Targets: `GantryCodeExampleFull` (the game: solo, host or join), `GantryCodeExampleClient` (join
  only), `GantryCodeExampleServer` (dedicated) and `GantryCodeExampleEditor`. Client and Server
  need an engine built with those targets (a source build); Full and Editor build on a launcher
  install.

## Building

The project is set up for Unreal Engine 5.6. For 5.7 or 5.8, right-click
`GantryCodeExample.uproject` and choose Switch Unreal Engine Version (or change
`EngineAssociation` in it), then regenerate the project files.

- **From the editor:** open `GantryCodeExample.uproject` and let it build the modules it asks for.
- **From the command line:** run the engine's build script with the editor target, for example
  `Engine/Build/BatchFiles/Linux/Build.sh GantryCodeExampleEditor Linux Development
  -Project=<path>/GantryCodeExample.uproject` (`Mac/Build.sh ... Mac`, or `Build.bat ... Win64`
  on Windows). The other targets build the same way.

## Playing

The game opens on the main menu: the train runs Lighthouse Point at sunset behind it. Ride alone,
host a crew on a port, join one at an address, or open the settings. The Client build only joins.
Aboard, Escape (or the gamepad's Menu button) brings up the pause menu (settings, leave the train).

- **Move** WASD or the left stick; **look** mouse or the right stick; **jump** Space or A.
- **Interact** E, left mouse or X: take a shovel of coal from the tender, carry it to the firebox in
  the locomotive's cab. Pressure sets the speed and drains as the train runs.
- **Host and join** from the main menu, or `?listen` on the map URL and `open <address>` from the
  console.

Development aids (console, development builds only, not Shipping): `railhead.AutoStoke 1` keeps
the boiler fed, `railhead.SpeedScale 4` runs the train faster, `railhead.StretchLengthScale 0.25`
shortens every stretch, and Gantry's own `Gantry.Experience.StageDelay 2` waits that many seconds
between stages so the tunnels have something to wait for.

## How it is put together

| Gantry feature | Where Railhead uses it |
|---|---|
| Experience tree | `EXP_Base` (the train and the line) → `EXP_Coast` (the route) → three stretches |
| A game's own experience class | `URailheadStretchDefinition`: length, scenery, light, the next stretch |
| A stage in C++ | `URailheadStage_Scenery`, on the route, serving every stretch on it; Client Only |
| Tracked loading | the scenery stage's loads, nested under Gantry's stage scope |
| Live experience swaps | `ARailheadLine` asks for the next stretch inside every tunnel |
| Streaming progress | a tunnel whose far side is still loading slows the train to match |
| Ready tiers | the line on the high tier; the HUD and the loading screen on the low one |
| Multiplayer | the server picks each stretch; clients follow, joining mid-route |
| Startup ladder | `MapExperiences` starts the Coast Line; `?Experience=Experience.Coast.Cliffs` starts elsewhere; the menu map starts Lighthouse Point |
| Player settings | Video and Crew pages declared in C++ (`URailheadSettingsProvider`, values bound to functions over the engine's user settings or kept in the save; Video for the first player only, Crew for each); the Coast Line adds its own page asset while it runs (an Add Settings Pages stage). `SRailheadSettings` draws them in Slate |
| Mods page | `URailheadModsProvider`: a toggle for each mod Gantry found, for the first player only, built again whenever the mods change |
| Loading screens | `FRailheadLoadFeed` lays out Gantry's scopes, assets and mods, paced over five seconds; a curtain for a world's first experience, a panel in tunnels |
| Message channels | `ARailheadLine` announces the tunnel and the daylight beyond (`Railhead.Message.Line.*`); the HUD's stretch title listens instead of watching the line. The game's module declares every channel it offers (`RailheadModule.cpp`) |
| Hooks | every tick the server's `ARailheadLine` runs `Railhead.Hook.Line.Speed` (`FRailheadLineSpeed`) with the speed the train heads for; the Weather stage's Train Speed Scale listens and changes it |
| Declared API | classes, properties and functions marked `meta=(GantryAPI)`, with the declared channels, generated into `Config/DefaultGantryAPI.ini` |
| Automatic receivers | Automatic Receiver Classes makes the line and the train Game Framework Component Manager receivers, so game features can add components to them |
| UI extension slots | the HUD owns `Railhead.HUD.Notes`; `EXP_Coast` puts its note there with an Add UI Extensions stage |
| Input from experiences | `EXP_Base`'s Add Input Mappings stage puts `IMC_Crew` on every player (rebindable); its Data carries the actions (`URailheadInputConfig`) |
| Key bindings | the Controls page (`URailheadControlsProvider`, one Gantry key-binding setting per rebindable key), added by `EXP_Base` after the mappings; `SRailheadSettings` draws a key selector |
| Readiness | `URailheadCrewReadiness`, a Gantry init-state component: player state, controls bound, then ready; it holds the loading screen until then (a Gantry loading hold) |
| Experience data | `URailheadLineRules`: `EXP_Base` brings the defaults, `EXP_Coast` overrides them; the line reads the nearest |
| Data patches | the Blizzard mod's Patch Data action changes `DA_LineRules_Coast`'s Acceleration while it runs |
| Runtime readings | the Video page's Show Performance setting puts Gantry's frame timings on the HUD |

The world is a treadmill: the train stands still at the origin and the line scrolls the track,
ground, scenery and tunnels past it. The crew walk on an actor that never moves, the track never
ends, and every machine places the same scenery from the stretch and the distance alone.

## Mods

Railhead takes mods from `Mods/` beside the game: data-only game features that ride along on the
Coast Line through the game's own extension points (its Couple Cars and Weather stages, the line's
speed hook, Patch Data on its line rules, its livery config). `Mods/README.md` describes the
examples, the two in `ModsWithProblems/` that show Gantry reporting a clash and refusing a mod made
for a newer game, how to try them in the editor, and how to cook a mod against a release.

## Status

A playable slice: one route, one crew job, a main menu, settings and content mods. Code mods, more
routes and the rest of the Gantry feature list are on their way.

## Known engine issues

- **5.8, now and then at editor startup:** `LogClass: Error: StructProperty
  FDataflowToolNodeSnapshot::Date is not initialized properly`. Epic's Dataflow plugin (enabled by
  default, and needed by Interchange and Chaos) initialises that field to the current time, and the
  engine's startup check builds the struct twice and compares; when the clock ticks over a
  millisecond in between, it reports the field. Nothing in Railhead or Gantry is involved, and it
  never affects cooking or packaged builds. Hiding it would mean lowering that check for every
  engine struct, so it is left visible.

[gantry-fab]: https://www.fab.com/listings/308b7a4a-7b05-45f6-8cf8-b05dec2779a6
