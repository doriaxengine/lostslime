# Lost Slime

A small side-scrolling platformer made with [Doriax Engine](https://github.com/doriaxengine/doriax).

Run through three levels, grab the key, stomp slimes and bees, keep away from saws and
spikes and reach the exit door before your hearts run out.

Open the folder in Doriax Editor and press **Play** on `Intro Scene`.

## Controls

| Input | Action |
| --- | --- |
| Arrows / WASD, gamepad stick or d-pad | Move |
| Space / Up / W, gamepad A | Jump (let go early for a short hop) |
| Down + Jump | Drop through a platform |
| Esc / P, gamepad Start | Pause |
| Enter, gamepad A | Menu confirm |

On phones and tablets the level shows on-screen buttons instead. They also appear after
touching the screen on any device, and hide again when a key is pressed.

## Project

- `Intro Scene`, `Game Over Scene`, `Win Scene`: menus.
- `Level One`, `Level Two`, `Level Three`: the levels. Each has a `Level` entity with the
  `LevelController` script, and uses `HUD Scene` and `Pause Scene` as child scenes.
- `Loading Scene`: shown by `SceneManager` while a level loads.
- `bundles/`: the player, enemies, pickups, door and the shared sounds.
- `scripts/`: the C++ scripts. `GameState` keeps score, hearts and the key between scenes.

The best score is saved with `System::setIntegerForKey`.

## Credits

Art, fonts and sound effects are CC0 assets by [Kenney](https://kenney.nl): New Platformer
Pack, UI Pack, Mobile Controls and Kenney Fonts. Their licenses are in `assets/licenses/`.

The music and the dust particle were made for this game and are CC0 as well.
