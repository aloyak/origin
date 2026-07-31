#!/usr/bin/env python3
"""
name_vert.glsl   -> vertex shader
name_frag.glsl   -> fragment shader
name_geom.glsl   -> geometry shader
name_comp.glsl   -> compute shader
name_tesc.glsl   -> tessellation control shader
name_tese.glsl   -> tessellation evaluation shader

vert.glsl        -> also valid (bare stage name, no prefix)
frag.glsl        -> also valid
...etc.

Any ".glsl" file that does not match one of these patterns is skipped
with a warning, since the compiler has no way to know which pipeline
stage it belongs to.

Compiled output is written next to the source file, same name, with
".spv" replacing ".glsl" (e.g. "name_vert.glsl" -> "name_vert.spv").

USAGE
-----
    python compile_shaders.py --source <path>              Compile all shaders in <path>
    python compile_shaders.py --source <path> --recursive   Compile all shaders, recursing up to 4 dirs deep
    python compile_shaders.py --source <path> --check       Validate shaders only, don't write .spv files
    python compile_shaders.py --source <path> --clean       Remove compiled .spv files
    python compile_shaders.py --source <path> --clean --recursive   Clean recursively too
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

MAX_RECURSE_DEPTH = 4

STAGE_MAP = {
    "vert": "vert",
    "frag": "frag",
    "geom": "geom",
    "comp": "comp",
    "tesc": "tesc",
    "tese": "tese",
}


def find_compiler():
    """Locate glslangValidator (preferred) or glslc on the current system."""
    candidates = ["glslangValidator", "glslc"]

    # Windows executables
    if os.name == "nt":
        candidates = ["glslangValidator.exe", "glslc.exe"] + candidates

    for exe in candidates:
        path = shutil.which(exe)
        if path:
            return path, ("glslc" if "glslc" in exe else "glslangValidator")

    # Fall back to checking VULKAN_SDK env var
    sdk = os.environ.get("VULKAN_SDK")
    if sdk:
        bin_dir = Path(sdk) / "Bin"
        for exe in candidates:
            candidate = bin_dir / exe
            if candidate.exists():
                return str(candidate), ("glslc" if "glslc" in exe else "glslangValidator")

    return None, None


def spv_path_for(glsl_file: Path) -> Path:
    """Return the .spv output path for a given 'name.<stage>.glsl' file.

    Path.with_suffix() only strips the *last* extension, which would turn
    'name.vert.glsl' into 'name.spv' (dropping the stage). We want
    'name.vert.spv', so we strip '.glsl' from the filename directly.
    """
    return glsl_file.with_name(glsl_file.name[: -len(".glsl")] + ".spv")


def get_stage(glsl_path: Path):
    """Return the shader stage for a 'name_<stage>.glsl' or bare '<stage>.glsl' file, or None."""
    stem = glsl_path.stem.lower()  # strips the single ".glsl" suffix

    # Bare stage name, e.g. "vert.glsl"
    if stem in STAGE_MAP:
        return STAGE_MAP[stem]

    # Underscore-suffixed, e.g. "name_vert.glsl" (also handles "a_b_vert.glsl")
    if "_" in stem:
        candidate = stem.rsplit("_", 1)[-1]
        if candidate in STAGE_MAP:
            return STAGE_MAP[candidate]

    return None


def find_shader_files(source: Path, recursive: bool):
    """Yield all .glsl files under source, respecting recursive/depth rules."""
    if source.is_file():
        if source.suffix.lower() == ".glsl":
            yield source
        return

    if not recursive:
        for f in sorted(source.glob("*.glsl")):
            if f.is_file():
                yield f
        return

    base_depth = len(source.resolve().parts)
    for root, dirs, files in os.walk(source):
        root_path = Path(root)
        depth = len(root_path.resolve().parts) - base_depth
        if depth >= MAX_RECURSE_DEPTH:
            dirs[:] = []  # don't descend further
        for name in sorted(files):
            if name.lower().endswith(".glsl"):
                yield root_path / name


def compile_shader(compiler_path: str, compiler_kind: str, glsl_file: Path, stage: str, check_only: bool):
    """Compile (or validate) a single shader. Returns (success, message)."""
    if check_only:
        out_path = Path(tempfile.gettempdir()) / (glsl_file.stem + "_check.spv")
    else:
        out_path = spv_path_for(glsl_file)

    if compiler_kind == "glslangValidator":
        cmd = [compiler_path, "-V", "-S", stage, "-o", str(out_path), str(glsl_file)]
    else:  # glslc
        stage_flag = {
            "vert": "vertex", "frag": "fragment", "geom": "geometry",
            "comp": "compute", "tesc": "tesscontrol", "tese": "tesseval",
        }[stage]
        cmd = [compiler_path, f"-fshader-stage={stage_flag}", str(glsl_file), "-o", str(out_path)]

    result = subprocess.run(cmd, capture_output=True, text=True)

    if check_only and out_path.exists():
        try:
            out_path.unlink()
        except OSError:
            pass

    if result.returncode == 0:
        return True, "OK"
    else:
        err = (result.stderr or result.stdout or "unknown error").strip()
        return False, err


def clean_shaders(source: Path, recursive: bool):
    removed = 0
    for glsl_file in find_shader_files(source, recursive):
        spv_file = spv_path_for(glsl_file)
        if spv_file.exists():
            try:
                spv_file.unlink()
                print(f"[REMOVED] {spv_file}")
                removed += 1
            except OSError as e:
                print(f"[ERROR] Could not remove {spv_file}: {e}")
    print(f"\nClean complete. {removed} file(s) removed.")


def main():
    parser = argparse.ArgumentParser(description="Compile/validate/clean GLSL shaders to SPIR-V")
    parser.add_argument("--source", required=True, help="Path to a shader file or a directory of shaders")
    parser.add_argument("--check", action="store_true", help="Only validate shaders, don't write .spv files")
    parser.add_argument("--clean", action="store_true", help="Remove compiled .spv files instead of compiling")
    parser.add_argument("--recursive", action="store_true", help=f"Search subdirectories, max depth: {MAX_RECURSE_DEPTH}")
    args = parser.parse_args()

    source = Path(args.source)
    if not source.exists():
        print(f"[ERROR] Source path does not exist: {source}")
        sys.exit(1)

    if args.clean:
        clean_shaders(source, args.recursive)
        return

    compiler_path, compiler_kind = find_compiler()
    if not compiler_path:
        print("[ERROR] Could not find 'glslangValidator' or 'glslc' on PATH or via VULKAN_SDK.")
        sys.exit(1)

    print(f"Using compiler: {compiler_path} ({compiler_kind})\n")

    shader_files = list(find_shader_files(source, args.recursive))
    if not shader_files:
        print("No .glsl files found.")
        return

    success_count = 0
    fail_count = 0
    skip_count = 0

    for glsl_file in shader_files:
        stage = get_stage(glsl_file)
        if stage is None:
            print(f"[SKIP] {glsl_file}  (no recognized stage, expected 'name_<stage>.glsl' or '<stage>.glsl')")
            skip_count += 1
            continue

        ok, message = compile_shader(compiler_path, compiler_kind, glsl_file, stage, args.check)
        label = "CHECK" if args.check else "COMPILE"
        if ok:
            print(f"[{label} OK] {glsl_file}")
            success_count += 1
        else:
            print(f"[{label} FAIL] {glsl_file}\n{message}\n")
            fail_count += 1

    print(f"\nDone. {success_count} succeeded, {fail_count} failed, {skip_count} skipped.")
    if fail_count > 0:
        sys.exit(1)


if __name__ == "__main__":
    main()