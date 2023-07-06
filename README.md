#C-ProjektArbeit

To build project:
inside project dir run:

cmake -S . -B build -G "MinGW Makefiles"

cd build

make

This should have created the gentxtsrccode.exe inside th build dir.
"MinGW Makefiles" can be replaced with a different make generator.
