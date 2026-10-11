# Railhead mods

Example mods for Railhead, built the way anyone outside the studio would build one: data only, on
the extension points the game exposes. The game never names them; it says only that mods live in
`Mods/` next to its `.uproject` (Project Settings > Gantry Experience > Mod Directories, with
Mount Mods At Startup on). Each attaches under `Experience.Base` and **Extends** `Experience.Coast`,
so it rides along on every Coast Line stretch.

| Mod | What it shows |
|---|---|
| `Blizzard` | Weather from data. Its experience runs the game's **Weather** stage: a low white sky, falling light, snow on the ground and the track. |
| `FreightVan` | A car from data. Its experience runs the game's **Couple Cars** stage with its own van, a Blueprint subclass of the game's car with its length, body and paint set. Shipped as one `.gantry` file and dropped into a running game, it joins at the next tunnel. |
| `LiveryCobalt` | Config only. Its experience activates the mod's own game feature, and the feature's config repaints the caboose cobalt blue (see Config, below). |

## Mods with problems

`ModsWithProblems/` holds two more, outside the game's mod directories so ordinary runs never see
them. Add `ModsWithProblems` to Mod Directories and mount to see Gantry deal with them:

| Mod | What happens |
|---|---|
| `LiverySaffron` | Sets the same livery key as `LiveryCobalt`. Gantry reports the clash: whichever activates last paints the caboose. |
| `LocomotiveMkII` | Made for a newer game: its game feature data needs game version 2.0. Gantry registers it, finds it incompatible, and unmounts it again, saying why. |

## Config

A game feature's config changes go in `Config/<Mod><Branch>.ini`: `LiveryCobaltGame.ini` for the
Game branch. When the feature activates, the engine layers that file over the game's own config.
It applies nothing else from `Config/` then; a `DefaultGame.ini` there is never read, and Gantry
notes it on the mod. `Config/Tags/` is read when the mod is registered, so mods bring their own
experience tags.

## Code

These mods carry no code, so they work in every build of the game. A mod with C++ modules needs a
game that can load code from a plugin: the editor, or a game built modular from a source engine,
with the mod built against that exact build. A packaged game is normally a single executable, and
there Gantry refuses a mod with modules before mounting anything of it.

## Cooking against a release

As for any Gantry game: package the game with `-createreleaseversion=<release>`, then cook each mod
with the stock BuildCookRun (`-cook -stage -pak -dlcname=<path to its .uplugin>
-basedonreleaseversion=<release>`, plus `-target=GantryCodeExampleServer -server -noclient` or
`-target=GantryCodeExampleClient -client` for dedicated builds). The staged containers go in the
mod's `Content/Paks/<Platform>/`; copy the mod folder (`.uplugin`, `Config`, `Content/Paks`) into
the packaged game's `Mods/`, or zip it, stored, as `<Mod>.gantry` and drop that in.

## Playing with mods over the network

Every machine needs the same mods. A client without a mod the host runs cannot ride that stretch:
Gantry refuses it with a reason naming both chains, and Railhead shows it with the way back to the
main menu.
