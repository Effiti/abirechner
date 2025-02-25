all: wasm

wasm:
	clang  --target=wasm32 -nostdlib -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined -o main.wasm main.c lib.c
	mv main.wasm ../public
