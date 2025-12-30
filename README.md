**English** | [简体中文](README-ZN.md)

# MonsterWar
**MonsterWar** is a cross-platform tower defense game developed in C++ with Entt, SDL3, glm, ImGui, nlohmann-json and Tiled.

> This project is a teaching demonstration project; it is the 4th episode in a series of tutorials titled "[C++ 游戏开发之旅](https://cppgamedev.top/)".

## Control
```
Mouse left - select a unit from UI portrait / place a unit on map;
S - Skill active shortcut;
R - Retreat shortcut;
U - Upgrade shortcut;
P - pause or resume;
A,D / left,right - to move UI portrait panel;
```

## Play on Webpage
[MonsterWar](https://wispsnow.github.io/MonsterWar/)

- **Note**: The game uses ImGui for temporary UI, so you need to resize the webpage size to fit the screen. Otherwise some UI elements may not be visible.


## ScreenShot
<img src="https://theorhythm.top/gamedev/MW/screen_shot_mw-en1.webp" style='width: 800px;'/>
<img src="https://theorhythm.top/gamedev/MW/screen_shot_mw-en2.webp" style='width: 800px;'/>
<img src="https://theorhythm.top/gamedev/MW/screen_shot_mw-en3.webp" style='width: 800px;'/>

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

- Sponsors: `sino`, `李同学`, `swrainbow`, `爱发电用户_b7469`, `玉笔难图`, `jl`

## Contact

For support or feedback, please contact us through the GitHub issues section of this repository. Your feedback is crucial for making this series of tutorials better!

## Buy Me a Coffee
[!["Buy Me A Coffee"](https://storage.ko-fi.com/cdn/kofi2.png?v=3)](https://ko-fi.com/ziyugamedev)
[!["Support me on Afdian"](https://pic1.afdiancdn.com/static/img/welcome/button-sponsorme.png)](https://afdian.com/a/ziyugamedev)

## QQ Discussion Group and My WeChat QR Code

<div style="display: flex; gap: 10px;">
  <img src="https://theorhythm.top/personal/qq_group.webp" width="200" />
  <img src="https://theorhythm.top/personal/wechat_qr.webp" width="200" />
</div>