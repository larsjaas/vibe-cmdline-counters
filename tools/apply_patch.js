#!/usr/bin/env node

// CommonJS version of apply_patch.js
const fs = require("fs").promises;
const readline = require("readline");

async function readStdin() {
  const rl = readline.createInterface({ input: process.stdin });
  let data = "";
  for await (const line of rl) data += line + "\n";
  return data;
}

async function main() {
  try {
    const inputStr = await readStdin();
    const args = JSON.parse(inputStr);
    const patch = args.patch;
    if (!patch) throw new Error("Missing patch field");

    const lines = patch.split(/\r?\n/);

    let currentFile = null;
    let fileLines = [];
    let fileBuffer = [];
    let inHunk = false;

    for (let i = 0; i < lines.length; i++) {
      const line = lines[i];

      if (line.startsWith("*** Begin Patch") || line.trim() === "") continue;

      if (line.startsWith("*** Update File:")) {
        if (currentFile && fileBuffer.length) {
          await fs.writeFile(currentFile, fileBuffer.join("\n") + "\n");
          console.log(`Patched file ${currentFile}`);
        }
        currentFile = line.replace("*** Update File:", "").trim();
        try {
          fileLines = (await fs.readFile(currentFile, "utf-8")).split(/\r?\n/);
        } catch {
          fileLines = [];
        }
        fileBuffer = [...fileLines];
        inHunk = false;
        continue;
      }

      if (line.startsWith("*** End Patch")) {
        if (currentFile && fileBuffer.length) {
          await fs.writeFile(currentFile, fileBuffer.join("\n") + "\n");
          console.log(`Patched file ${currentFile}`);
        }
        currentFile = null;
        fileBuffer = [];
        continue;
      }

      if (line.startsWith("@@")) {
        inHunk = true;
        continue;
      }

      if (inHunk && currentFile) {
        if (line.startsWith("-")) {
          const oldLine = line.slice(1);
          const index = fileBuffer.indexOf(oldLine);
          if (index >= 0) fileBuffer.splice(index, 1);
        } else if (line.startsWith("+")) {
          const newLine = line.slice(1);
          fileBuffer.push(newLine);
        }
      }
    }

    if (currentFile && fileBuffer.length) {
      await fs.writeFile(currentFile, fileBuffer.join("\n") + "\n");
      console.log(`Patched file ${currentFile}`);
    }
  } catch (err) {
    console.error("Error applying patch:", err.message);
    process.exit(1);
  }
}

main();
