# rt

Renderer/ray tracer test framework.

> ℹ&#xFE0F; All commands in this readme assume CWD is the repository root.

<br><br>

## Building and running

### Prerequisites

-   A C++20-capable compiler
-   The [meson] build system
-   A recent version of Python 3 (required by [meson]).

#### Linux

You may need to install a bunch of packages for x11 and sdl2, e.g.:

```sh
sudo apt install xorg-dev libx11-dev libgl1-mesa-glx libsdl2-dev
```

### Creating a build directory

```sh
# windows with visual studio:
meson setup builddir --backend vs

# everything else:
meson setup builddir
```

### Building

```sh
cd builddir && meson compile
```

### Running

`rt` is a command-line application.

```
Usage: rt [-h] --renderer <name> <path>

Renders a scene with a software renderer of your choosing.

Positional arguments:
  <path>                scene TOML file [required]

Optional arguments:
  -h, --help            shows help message and exits
  -v, --version         prints version information and exits
  -r, --renderer <name> renderer name [default: "mg_scalar_ray_tracer"]
```

Available renderers are listed as part of the program's stdout during regular execution.

<br><br>

## Misc

### Redownloading subprojects

Downloading of the required subproject dependencies should happen for you automatically, but if it doesn't (or you wish to force a re-download/update):

```sh
meson subprojects download && meson subprojects update --reset
```

### Switching between Debug and Release builds

The default build config is Release (highly-optimized but poor debugging experience). If you wish to change to debug:

```sh
cd builddir && meson configure --buildtype debug
```

### Adding a new renderer

Renderers are intended to be self-contained in single `.cpp` files in `src/renderers/`. The general process for adding a new one is as follows:

1. Create your new renderer `.cpp` file in `src/renderers/` (use one of the existing ones as a template if necessary)
2. Add the cpp file to the list in `src/renderers/meson.build`
3. Build and test

### Regenerating SoA types

The struct-of-arrays types are generated using [soagen]. Their code files are already present in the repository so
you won't need to do this, unless you wish to change them in some manner, in which case:

```sh
# initial soagen install
sudo pip3 install soagen

# regenerate files and update soagen.hpp
soagen src\soa.toml --install vendor
```

<br><br>

[meson]: https://mesonbuild.com/Getting-meson.html
[soagen]: https://marzer.github.io/soagen/
