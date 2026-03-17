#!/usr/bin/env node

const fs = require("fs");
const fsp = require("fs/promises");
const path = require("path");

// ---- Read stdin safely ----
async function readStdin() {
  return new Promise((resolve, reject) => {
    let data = "";
    process.stdin.setEncoding("utf8");

    process.stdin.on("data", chunk => (data += chunk));
    process.stdin.on("end", () => resolve(data));
    process.stdin.on("error", reject);
  });
}

// ---- Ensure directory exists ----
async function ensureDir(filePath) {
  await fsp.mkdir(path.dirname(filePath), { recursive: true });
}

// ---- Apply patch ----
async function applyPatch(patchText) {
  const lines = patchText.split("\n");

  let currentFile = null;
  let fileBuffer = [];
  let originalLines = [];

  let mode = null; // null | "add" | "update"
  let inHunk = false;

  async function flushFile() {
    if (currentFile === null) return;

    await ensureDir(currentFile);
    await fsp.writeFile(currentFile, fileBuffer.join("\n"), "utf8");
  }

  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];

    // ---- FILE OPERATIONS ----

    if (line.startsWith("*** Update File: ")) {
      await flushFile();

      currentFile = line.replace("*** Update File: ", "").trim();
      mode = "update";
      inHunk = false;

      try {
        const content = await fsp.readFile(currentFile, "utf8");
        originalLines = content.split("\n");
      } catch {
        originalLines = [];
      }

      fileBuffer = [...originalLines];
      continue;
    }

    if (line.startsWith("*** Add File: ")) {
      await flushFile();

      currentFile = line.replace("*** Add File: ", "").trim();
      mode = "add";
      inHunk = false;
      fileBuffer = [];
      continue;
    }

    if (line.startsWith("*** Delete File: ")) {
      const fileToDelete = line.replace("*** Delete File: ", "").trim();

      try {
        await fsp.unlink(fileToDelete);
        console.error(`Deleted ${fileToDelete}`);
      } catch {
        console.error(`File not found for deletion: ${fileToDelete}`);
      }

      continue;
    }

    if (line.startsWith("*** End Patch")) {
      await flushFile();
      currentFile = null;
      mode = null;
      continue;
    }

    // ---- HUNKS ----

    if (line.startsWith("@@")) {
      inHunk = true;
      continue;
    }

    // ---- ADD FILE MODE ----

    if (mode === "add") {
      if (line.startsWith("+")) {
        fileBuffer.push(line.slice(1));
      } else if (!line.startsWith("@@")) {
        fileBuffer.push(line);
      }
      continue;
    }

    // ---- UPDATE MODE ----

    if (mode === "update" && inHunk) {
      // Replace (- followed by +)
      if (line.startsWith("-")) {
        const oldLine = line.slice(1);
        const nextLine = lines[i + 1];

        const index = fileBuffer.indexOf(oldLine);

        if (nextLine && nextLine.startsWith("+")) {
          const newLine = nextLine.slice(1);

          if (index >= 0) {
            fileBuffer.splice(index, 1, newLine);
          } else {
            console.error(`WARN: line not found for replace: ${oldLine}`);
          }

          i++; // skip "+"
        } else {
          // Pure delete
          if (index >= 0) {
            fileBuffer.splice(index, 1);
          } else {
            console.error(`WARN: line not found for delete: ${oldLine}`);
          }
        }

        continue;
      }

      // Standalone add
      if (line.startsWith("+")) {
        fileBuffer.push(line.slice(1));
        continue;
      }

      // Context line (ignored in this simple implementation)
      continue;
    }
  }

  // Final flush (if no *** End Patch)
  await flushFile();
}

// ---- Main ----
(async () => {
  try {
    const input = await readStdin();
    const json = JSON.parse(input);

    if (!json.patch) {
      throw new Error("Missing patch field");
    }

    await applyPatch(json.patch);

    console.log("Patch applied successfully");
  } catch (err) {
    console.error("Error applying patch:", err.message);
    process.exit(1);
  }
})();

