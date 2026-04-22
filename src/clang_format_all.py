from pathlib import Path
import subprocess

if __name__ == "__main__":
    for path in Path(".").rglob("*"):
        if path.suffix == ".cpp" or path.suffix == ".hpp":
            subprocess.run(["clang-format", "-i", path])
