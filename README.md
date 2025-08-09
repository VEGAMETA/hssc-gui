# hssc-gui

Simple gui tool to manipulate [hssc](https://github.com/VEGAMETA/hyprland-screen-controller)

## Building

```bash
cd src
make install
```

## Usage

```bash
hssc-gui
```

You can open the gui hidden with any passed argument

```bash
hssc-gui -
```

It can be useful for [pypr's](https://github.com/hyprland-community/pyprland)
[scratchpads](https://hyprland-community.github.io/pyprland/scratchpads.html)

Add this lines @ `~/.config/hypr/pyprland.toml`

```toml
[scratchpads.hssc]
animation = "fromRight"
command = "hssc-gui -"
class = "hssc-gui"
size = "30% 225px"
offset = "200%"
```

Then just add your keybinding @ `~/.config/hypr/hyprland.conf`

```conf
bind = $mainMod, O, exec, pypr toggle hssc
```
