import wasm from "vite-plugin-wasm";
import topLevelAwait from "vite-plugin-top-level-await";
import { defineConfig } from 'vite'
//@ts-ignore
import { exec } from "child_process";
//@ts-ignore
// import { $ } from "bun";

export default defineConfig({
  plugins: [
    wasm(),
    topLevelAwait(),
    {
      name: "watch-wasm",
      configureServer(server) {
        // Watch for changes in C files
        server.watcher.add("src/**/*.c");

        // Listen for changes
        server.watcher.on("change", async (path) => {
          if (path.endsWith(".c")) {
            console.log(`🔄 Detected change in ${path}, recompiling WASM...`);
            //@ts-ignore
            await exec(`cd wasm; make`);
            server.ws.send({
              type: "full-reload",
            });          }
        });
      },
    },  ]
});
