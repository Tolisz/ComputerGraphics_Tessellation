# Tessellation

https://github.com/user-attachments/assets/009bd82e-4ba5-4b47-806f-561346ed38a8

## About project 

This is one of the projects created during *"Computer Graphics 2"* course, specialty **CAD/CAM systems design** of degree in *Computer Science and Information Systems* at *Warsaw University of Technology*. 

Project features: 

- Tessellation is used to draw Bézier patches
- Dynamic LOD based on distance of the camera from the surface.
- Normal mapping of the tessellated surface.
- Displacement mapping of the tessellated surface.

## Navigation

<table>
    <tbody>
		<tr>
            <th colspan=2> Camera</th>
        </tr>
		<tr>
            <td><kbd>LMB</kbd></td>
            <td>camera rotation</td>
        </tr>
		<tr>
            <td><kbd>RMB</kbd></td>
            <td>camera zoom in/out</td>
        </tr>
		<tr>
            <th colspan=2>Bézier Patch</th>
        </tr>
		<tr>
            <td><kbd>W</kbd></td>
            <td>turn on/off grid view</td>
        </tr>
		<tr>
            <td><kbd>F</kbd></td>
            <td>choose Bézier control points' shapes</td>
        </tr>
		<tr>
            <td><kbd>C</kbd></td>
            <td>show Bézier control points</td>
        </tr>
		<tr>
            <td><kbd>S</kbd></td>
            <td>turn on/off Phong shading of the surface</td>
        </tr>
		<tr>
            <td><kbd>Q</kbd></td>
            <td>turn on/off display of 16 Bézier patches</td>
        </tr>
		<tr>
            <td><kbd>E</kbd></td>
            <td>turn on/off dynamic level of details</td>
        </tr>
		<tr>
            <td><kbd>T</kbd></td>
            <td>turn on/off surface texturing</td>
        </tr>
		<tr>
            <td><kbd>R</kbd></td>
            <td>turn on/off normal mapping</td>
        </tr>
		<tr>
            <td><kbd>D</kbd></td>
            <td>turn on/off displacement mapping</td>
        </tr>
	</tbody>
</table>

## Compilation 

The project is written in C++ (OpenGL) using CMake as a build system. It has two configuration CMake options: 

| Option | Description | 
| :---:         |     :---      |
| `TESSELLATION_EMBEDDED_SHADERS`   | If `ON` shader code will be embedded into a resulting app's binary. If `OFF` shader code will be read from `shaders` directory, so a binary has to be run from repository root directory.      |
| `TESSELLATION_SHOW_DEBUG_CONSOLE` | If `ON` - beside app's window - a debug console will be shown on app start up. If `OFF` only app's window will be shown.        |

### Windows 

All subsequent commands should be called from Windows Power Shell.

```
git clone --recurse-submodules -j8 https://github.com/Tolisz/ComputerGraphics_Tessellation.git
cd ComputerGraphics_Tessellation
cmake -S . -B build -DTESSELLATION_EMBEDDED_SHADERS=ON -DTESSELLATION_SHOW_DEBUG_CONSOLE=OFF
cmake --build build --config Release
```

```
.\build\Release\tessellation.exe
```

### Linux 

All subsequent commands should be called from terminal. Compilation was checked using **g++** (version 11.4.0) and **clang++** (version 14.0.0) compilers.

```
git clone --recurse-submodules -j8 https://github.com/Tolisz/ComputerGraphics_Tessellation.git
cd ComputerGraphics_Tessellation
cmake -S . -B build -DTESSELLATION_EMBEDDED_SHADERS=ON -DTESSELLATION_SHOW_DEBUG_CONSOLE=OFF
cmake --build build --config Release -j 26
```

```
./build/tessellation
```

## Used Libraries

Libraries `GLFW`, `GLM`, `GLI` and `ImGui` are used as submodules located in `externals` directory. `GLAD` is used as a generated file from [glad](https://glad.dav1d.de/) web page. 

| Library | Licence |
| :---:   | :---:   |
| [GLFW](https://github.com/glfw/glfw) | [Zlib](https://github.com/glfw/glfw?tab=Zlib-1-ov-file#readme) |
| [GLM](https://github.com/g-truc/glm) | [Modified MIT](https://github.com/g-truc/glm?tab=License-1-ov-file#readme)|
| [ImGui](https://github.com/ocornut/imgui) | [MIT](https://github.com/ocornut/imgui?tab=MIT-1-ov-file#readme)| 
| [GLAD](https://github.com/Dav1dde/glad) | [MIT](https://github.com/Dav1dde/glad?tab=License-1-ov-file#readme) | 
| [GLI](https://github.com/g-truc/gli) | [Happy Bunny License (Modified MIT)](https://github.com/g-truc/gli/blob/master/manual.md#licenses-) | 