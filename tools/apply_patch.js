#!/usr/bin/env node

// CommonJS version of apply_patch.js
const fs = require("fs").promises;
const readline = require("readline");
const path = require("path");

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
        if (currentFile !== null) {
          await fs.mkdir(path.dirname(currentFile), { recursive: true });
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

      if (line.startsWith("*** Delete File:")) {
        const filePath = line.replace("*** Delete File:", "").trim();
      
        try {
          await fs.unlink(filePath);
          console.log(`Deleted file ${filePath}`);
        } catch (err) {
          console.error(`Failed to delete ${filePath}: ${err.message}`);
        }
      
        // reset state
        currentFile = null;
        fileBuffer = [];
        inHunk = false;
        continue;
      }

      if (line.startsWith("*** Add File:")) {
        const filePath = line.replace("*** Add File:", "").trim();
        currentFile = filePath;
        fileBuffer = [];
        inHunk = "add";
        continue;
      }

      if (line.startsWith("*** End Patch")) {
        if (currentFile !== null) {
          await fs.mkdir(path.dirname(currentFile), { recursive: true });
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
        if (inHunk === "add") {
          fileBuffer.push(line);
          continue;
        }
      
        if (line.startsWith("-")) {
          const oldLine = line.slice(1);
          const nextLine = lines[i + 1];
        
          if (nextLine && nextLine.startsWith("+")) {
            const newLine = nextLine.slice(1);
            const index = fileBuffer.indexOf(oldLine);
        
            if (index >= 0) {
              fileBuffer.splice(index, 1, newLine); // replace in-place
            }
        
            i++; // skip the "+" line
          } else {
            // pure deletion
            const index = fileBuffer.indexOf(oldLine);
            if (index >= 0) fileBuffer.splice(index, 1);
          }
        } else if (line.startsWith("+")) {
          fileBuffer.push(line.slice(1));
        }
      }
    }

    if (currentFile !== null) {
      await fs.mkdir(path.dirname(currentFile), { recursive: true });
      await fs.writeFile(currentFile, fileBuffer.join("\n") + "\n");
      console.log(`Patched file ${currentFile}`);
    }
  } catch (err) {
    console.error("Error applying patch:", err.message);
    process.exit(1);
  }
}

main();
