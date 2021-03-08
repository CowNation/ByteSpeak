echo "Compiling & linking.."
clang++-7 -pthread -std=c++17 -o main main.cpp
./main
nasm -f elf output.asm
ld -m elf_i386 -s -o build output.o

echo "Build complete, running.."
./build