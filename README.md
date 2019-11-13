# Laser Engraver Toolkit
Graphical toolkit that allow you todo basic engraving.

![](https://devwork.space/wp-content/uploads/2019/11/laser-engraver-preview.png)

## Status
Works fine on Linux, but isn't feature complete.

### Compilation and installation
```console
user@workspace # git clone https://github.com/dev-0x7C6/laser-engraver.git
user@workspace # cd laser-engraver
user@workspace # cmake . -DCMAKE_INSTALL_PREFIX=/usr
user@workspace # make -j8
root@workspace # make install
```
### Gentoo ebuild
Ebuild for can be found in my overlay repository (https://github.com/dev-0x7C6/dev1990-overlay)

## Features
* Multi-object-layer workspace scene
* Simple picture engraving
* Label engraving
* Support for multiple engraver profiles
* Grbl firmware compatible (G-code Grbl flavor)
* Tested on EleksMaker A3 and other custom made engraver

## Scheduled features
* Allow to change speed or power of laser while engraving (live parameters customization)
* Allow to save/load workspace project
* Multiple tool profiles (for different laser modules)
* Cutting mode
* Command line options to generate gcode without gui usage

## Pictures
![](https://devwork.space/wp-content/uploads/2019/10/IMG_20191021_193817.jpg)
![](https://devwork.space/wp-content/uploads/2019/10/IMG_20191021_193821.jpg)
![](https://devwork.space/wp-content/uploads/2019/10/laser-engraver-1.png)
![](https://devwork.space/wp-content/uploads/2019/10/IMG_20191022_002848.jpg)
