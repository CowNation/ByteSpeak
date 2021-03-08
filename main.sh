echo "Compiling & linking.."
clang++-7 -pthread -std=c++17 -o compiler main.cpp
./compiler

nasm -f elf output.asm
ld -m elf_i386 -s -o build output.o

echo "Build complete, running.."
./build

echo "Program has been ran, cleaning up files.."
rm compiler
rm output.asm
rm output.o
rm build