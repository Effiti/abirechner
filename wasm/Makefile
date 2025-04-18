all: wasm

wasm:
	emcc -g3 --target=wasm32 -nostdlib -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined -o main.wasm main.c lib.c -gseparate-dwarf=temp.debug.wasm
	mv *.wasm ../public
