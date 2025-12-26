[English](README.md) | **简体中文**

# MonsterWar
**MonsterWar** 是一个使用 Entt, SDL3, glm, ImGui, nlohmann-json 和 Tiled 开发的跨平台 C++ 塔防游戏。

> 本项目是一个教学演示项目，是 “[C++ 游戏开发之旅](https://cppgamedev.top/)” 系列教程的第 4 篇。

## 操作说明
```
鼠标左键 - 在 UI 头像处选择单位 / 在地图上放置单位；
S - 技能快捷键；
R - 撤退快捷键；
U - 升级快捷键；
P - 暂停或继续；
A,D / 左,右箭头 - 移动 UI 头像面板；
```

## 网页版试玩
[MonsterWar](https://wispsnow.github.io/MonsterWar/)

- **注意**: 游戏使用 ImGui作为临时 UI，因此你需要调整网页大小以适应屏幕。否则某些 UI 元素可能不可见。


## 游戏截图
<img src="https://theorhythm.top/gamedev/MW/screen_shot_mw1.webp" style='width: 600px;'/>
<img src="https://theorhythm.top/gamedev/MW/screen_shot_mw2.webp" style='width: 600px;'/>
<img src="https://theorhythm.top/gamedev/MW/screen_shot_mw3.webp" style='width: 600px;'/>
<img src="https://theorhythm.top/gamedev/MW/screen_shot_mw4.webp" style='width: 600px;'/>

## 第三方库
* [EnTT](https://github.com/skypjack/entt)
* [SDL3](https://github.com/libsdl-org/SDL)
* [SDL3_image](https://github.com/libsdl-org/SDL_image)
* [SDL3_mixer](https://github.com/libsdl-org/SDL_mixer)
* [SDL3_ttf](https://github.com/libsdl-org/SDL_ttf)
* [glm](https://github.com/g-truc/glm)
* [ImGui](https://github.com/ocornut/imgui)
* [nlohmann-json](https://github.com/nlohmann/json)
* [spdlog](https://github.com/gabime/spdlog)

## 构建指南
依赖项将通过 Git FetchContent 自动下载，构建非常简单：
```bash
git clone https://github.com/WispSnow/MonsterWar.git
cd MonsterWar
cmake -S . -B build
cmake --build build
```

如果你在从 GitHub 下载时遇到问题（尤其是在中国大陆网络环境下），请参考 [wiki](../../wiki) 获取替代构建指南。

# 致谢
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

## 联系方式

如需支持或反馈，请通过本仓库的 GitHub issues 版块联系我们。您的反馈对于改进这一系列教程至关重要！

## 请我喝咖啡
[!["Buy Me A Coffee"](https://storage.ko-fi.com/cdn/kofi2.png?v=3)](https://ko-fi.com/ziyugamedev)
[!["Support me on Afdian"](https://pic1.afdiancdn.com/static/img/welcome/button-sponsorme.png)](https://afdian.com/a/ziyugamedev)

## QQ 交流群及我的微信二维码

<div style="display: flex; gap: 10px;">
  <img src="https://theorhythm.top/personal/qq_group.webp" width="200" />
  <img src="https://theorhythm.top/personal/wechat_qr.webp" width="200" />
</div>
