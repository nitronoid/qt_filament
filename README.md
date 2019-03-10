# Qt Filament hello world example
This example program builds a basic Qt window and displays a colored triangle.
The goal was to show how filament and Qt interoperability could be achieved.
Please feel free to correct areas and build upon this example.

## Build Instruction
We need to first compile our material to access it from the program. Following that, we simply run qmake, and then make.
```
> matc -o materials/bakedColor.inc -f header materials/bakedColor.mat
> qmake
> make -j
> ./build/bin/QtFilament
```

## Notes
The `filament_raii.h` header contains some simple wrapper classes around filament entities and engine registered objects, to ensure they are correctly destroyed in a modern C++ manor.
If you would rather not use them, you should simply define a destructor in the FilamentWindow class, that destroys all of the resources manually.

## Screen shot
<p align="center">
<img src="https://i.imgur.com/WE3APrk.png" width="500">
</p>


