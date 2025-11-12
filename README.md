# MonsterWar
**MonsterWar** is a cross-platfrom tower defense game developed in C++ with Entt, SDL3, glm, ImGui, nlohmann-json and Tiled.

## Control
```
Mouse left - select a unit from UI portrait / place a unit on map;
S - Skill active shortcut;
R - Retreat shortcut;
U - Upgrade shortcut;
P - pause or resume;
A,D / left,right - to move UI portrait panel;
```

## ScreenShot
<img src="https://theorhythm.top/gamedev/MW/screen_shot_mw1.webp" style='width: 600px;'/>
<img src="https://theorhythm.top/gamedev/MW/screen_shot_mw2.webp" style='width: 600px;'/>
<img src="https://theorhythm.top/gamedev/MW/screen_shot_mw3.webp" style='width: 600px;'/>
<img src="https://theorhythm.top/gamedev/MW/screen_shot_mw4.webp" style='width: 600px;'/>

## Third-party libraries
* [EnTT](https://github.com/skypjack/entt)
* [SDL3](https://github.com/libsdl-org/SDL)
* [SDL3_image](https://github.com/libsdl-org/SDL_image)
* [SDL3_mixer](https://github.com/libsdl-org/SDL_mixer)
* [SDL3_ttf](https://github.com/libsdl-org/SDL_ttf)
* [glm](https://github.com/g-truc/glm)
* [ImGui](https://github.com/ocornut/imgui)
* [nlohmann-json](https://github.com/nlohmann/json)
* [spdlog](https://github.com/gabime/spdlog)

## How to build
Dependencies will be automatically downloaded by Git FetchContent to make building quite easy:
```bash
git clone https://github.com/WispSnow/MonsterWar.git
cd MonsterWar
cmake -S . -B build
cmake --build build
```

If you encounter trouble downloading from GitHub (especially on networks in mainland China), please refer to the [wiki](../../wiki) for an alternative building guide.

# Credits
- sprite
    - https://pixelfrog-assets.itch.io/tiny-swords
    - https://pipoya.itch.io/pipoya-free-2d-game-character-sprites
    - https://htmljsgit.itch.io/magic-area
- portrait
    - https://blog.goo.ne.jp/akarise
    - https://roughsketch.en-grey.com/
- FX
    - https://bdragon1727.itch.io/750-effect-and-fx-pixel-all
    - https://sentient-dream-games.itch.io/pixel-vfx-level-up-effect
- font
    - https://timothyqiu.itch.io/vonwaon-bitmap
- UI
    - https://ludicarts.itch.io/free-rpg-icon-set-i
    - https://clockworkraven.itch.io/free-rpg-icon-pack-100-weapons-and-po-clockwork-raven-studios
    - https://kenney.nl/assets/emotes-pack
    - https://bdragon1727.itch.io/custom-border-and-panels-menu-all-part
- sound
    - https://ateliermagicae.itch.io/fantasy-ui-sound-effects
    - https://pixabay.com/sound-effects/violin-lose-4-185125/
    - https://pixabay.com/sound-effects/level-win-6416/
    - https://freesound.org/people/SilverIllusionist/sounds/664265/ (Healing (Balm).wav by Dylan Kelk)
    - https://freesound.org/people/DWOBoyle/sounds/136696/
- music
    - https://tommusic.itch.io/free-fantasy-sfx-and-music-bundle
    - https://www.chosic.com/download-audio/45301/

- Sponsors: `sino`, `李同学`, `swrainbow`, `爱发电用户_b7469`, `玉笔难图`

