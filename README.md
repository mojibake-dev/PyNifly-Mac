# PyNifly-Mac

A macOS port of [PyNifly](https://github.com/BadDogSkyrim/PyNifly), Bad Dog's Blender add-on for importing and exporting Bethesda nif files. Upstream ships its core as a Windows DLL. This fork compiles the same C++ as a native dylib and patches the few Windows-only calls in the Python.

## "Works on My Machine" Disclaimer

Tested on one Apple Silicon Mac (macOS 26.6, Blender 5.1.1) importing Skyrim SE meshes: heads, hair, eyes, brows and ears, including RaceMenu `BSDynamicTriShape` exports. Export, Fallout 4, Starfield, collisions and animations have **not** been tested. If something breaks, try it on Windows before reporting it upstream; if it works there, it's mine.

## What's different from upstream

| File | Change |
| --- | --- |
| `NiflyDLL/macos/` | CMake build for `libNiflyDLL.dylib`. Compiles the same wrapper sources as `NiflyDLL.vcxproj` against [BadDogSkyrim/nifly](https://github.com/BadDogSkyrim/nifly), pinned by commit, and copies `skeletons/` next to the library the way the vcxproj does. No C++ is edited: `compat/windows.h` maps the one Win32 call (`GetModuleFileNameA`, used to find `skeletons/`) onto `dladdr`. |
| `io_scene_nifly/pyn/niflydll.py` | Loads `libNiflyDLL.dylib` on macOS, `NiflyDLL.dll` everywhere else. Dev mode (`PYNIFLY_DEV_ROOT`) loads `build/libNiflyDLL.dylib` and builds its paths with `os.path.join`. |
| `io_scene_nifly/gamefinder.py` | `winreg` is optional. With no registry, game autodetect finds nothing instead of crashing. |
| `io_scene_nifly/blender_defs.py` | `GetShortPathNameW` is only bound on Windows; elsewhere the long path is used as-is. |
| `io_scene_nifly/pyn/niflytools.py` | Texture paths in nifs use backslashes. They're normalized before lookup so `textures\foo\bar.dds` resolves on a POSIX filesystem. No effect on Windows. |
| `tests/test_tools_bpy.py` | Tests spell fixture paths Windows-style; `test_file` normalizes them so the suite finds its files on a Mac. |

The `macos` branch carries these on top of upstream `main`. `main` is left as a mirror of upstream.

It also carries one fix that isn't about macOS, offered upstream on the `facegen-placeholder-bones` branch: RaceMenu's CharGen export (`SKSE/Plugins/CharGen/*.nif`) parks the facegen bone NiNodes at the origin, which made PyNifly tear the head and hair on import. Placeholder bones now take their reference skeleton position. Test: `TEST_FACEGEN_PLACEHOLDER_BONES`.

## Install

Build the add-on zip (below), then in Blender: Edit > Preferences > Add-ons > Install from Disk, pick the zip, and tick **NIF format (PyNifly)**.

## Build

Needs the Xcode command line tools and CMake 3.24+.

```bash
git clone -b macos https://github.com/mojibake-dev/PyNifly-Mac
cd PyNifly-Mac
cmake -S NiflyDLL/macos -B build
cmake --build build --target addon
# -> build/io_scene_nifly-<version>-macos-<arch>.zip
```

Builds for the host architecture. Pass `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"` for a universal binary.

## macOS Notes

- **Textures.** Upstream finds the game's `Data` folder through the Windows registry. On a Mac, point **Skyrim Texture Path 1** (add-on preferences) at an extracted `Data` folder instead. Textures that sit next to the nif's `meshes` folder are found without any setup.
- **Case.** PyNifly lowercases texture paths before looking them up. That works on the default case-insensitive APFS volume and fails on a case-sensitive one.

## Running the tests

Upstream's harness expects `PYNIFLY_DEV_ROOT` to be a folder holding the checkout as `PyNifly`, and Blender's add-on folder to link back to the checkout's `io_scene_nifly`:

```bash
cmake --build build                      # dev mode loads build/libNiflyDLL.dylib
mkdir -p ~/pyn-dev && ln -s "$PWD" ~/pyn-dev/PyNifly
ln -s "$PWD/io_scene_nifly" ~/Library/Application\ Support/Blender/5.1/scripts/addons/io_scene_nifly
PYNIFLY_DEV_ROOT=~/pyn-dev /Applications/Blender.app/Contents/MacOS/Blender --background --python test_runner.py
```

Tests that read vanilla game textures (`SKYRIM_ASSETS`, `FO4_ASSETS` in `tests/test_tools.py`) fail on log warnings unless those point at extracted game data. Exports land in `tests/tests/Out/`, which has to exist.

## Keeping up with upstream

```bash
git fetch upstream
git switch main && git merge --ff-only upstream/main && git push
git switch macos && git rebase main && git push --force-with-lease
```

If upstream changes `NiflyDLL/`, or the nifly fork moves, bump `GIT_TAG` in `NiflyDLL/macos/CMakeLists.txt` to the nifly commit upstream builds against.

---

## Upstream README

Export/Import tools between Blender and the Nif format, using Bodyslide/Outfit Studio's Nifly layer. Works with official Blender versions 4.0 and later; animation import and export require Blender 4.4 or later. Can handle nifs for Skyrim LE, Skyrim SE, and Fallout 4. Limited support for Fallout 76, Fallout New Vegas, and Fallout 3. Experimental support for Starfield.

Windows only upstream; this fork adds macOS.

Full documentation in the [wiki](https://github.com/BadDogSkyrim/PyNifly/wiki).

**Features**

* Supports Skyrim LE, Skyrim SE, and FO4. Starfield support is a WIP.
* Handles tris and base mesh in one step. No need to separate UV seams or triangulate before exporting
* Handles expression and chargen tri files for Skyrim and FO4
* Import tris into an existing mesh or on their own
* Supports Bodyslide tri files on import and export for body morphs
* Handles multiple bodyweights in one step. Export _0 and _1 armor weights for Skyrim from a single mesh.
* Handles Skyrim and FO4 partitions and FO4 segments
* Handles FO4 dismemberment, including generating the cut offsets a body needs to come apart in game
* Handles skinned and unskinned meshes correctly. Exports head parts to SE correctly.
* Handles shaders correctly. Set them up in Blender and export.
* Nif properties live in named panels in Blender, not scattered custom attributes
* Handles collisions. Represents them as meshes in Blender, and exports them back --
  including Skyrim MOPP collision and FO4's native physics
* Handles animations. Direct import/export to hkx files for FO4, SE, and LE, and
  exports hkx skeletons for Skyrim and FO4
* Handles animated nifs
* Special handling for FO4 connect points on weapons, armor, and workshop parts.
* Handles trees and other switch/multibound nodes
* Starfield: meshes, materials, facial morphs, and facebones
* Import-and-forget. What you import will behave correctly on export without fiddling.

**Background**

Outfit Studio has working export/import for a bunch of the Bethesda games. I'm a modder and I've been using the niftools scripts for years, but I've started modding for Fallout 4 and there's no direct support for that at all.

BS/OS has separated much of their code to deal with nif files into a separate library, nifly. I've used this library and some additional OS code as the core of an export/import addon for Blender. 

My interest is primarily Skyrim, Skyrim SE, and Fallout 4 so I'll be focusing on supporting those games. 

**Credits**

_Core technology_

- Ousnius for the nifly layer PyNifly is based on

_Critical information on nif/HKX encoding_

- Candoran2 
- DagobaKing
- Nikolivanov
- Nitaigao
- PredatorCZ

_Contributions to the tool_

- bitbanger
- jgernandt 
- Reddraconi
- ShroomTip
- ZenithVal

_macOS port_

- macOS port and modifications by Samara Eli, 2026-11-19. Based on upstream `main` at `e1caf11`; changes are listed under "What's different from upstream" above. Licensed GPL-3.0 like the original.
