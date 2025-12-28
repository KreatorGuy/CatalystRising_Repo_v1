"""Convert AvatarSDK mesh downloads into Unreal-importable formats.

This is primarily for the Avatar WEB API `animated_face` pipeline, whose `mesh` endpoint
returns a ZIP containing `model.ply`.

This script:
- Extracts the .ply from the zip
- Converts it to .obj (using trimesh)
- Writes a minimal .mtl referencing the provided texture

Example:
  python Tools/avatar_mesh_convert.py \
    --mesh-zip Generated/<avatar>_mesh.zip \
    --texture Generated/<avatar>_texture.jpg \
    --outdir Generated/<avatar>_unreal

Then import the resulting OBJ + texture into Unreal.
"""

from __future__ import annotations

import argparse
import os
import shutil
import tempfile
import zipfile
from pathlib import Path


def _guess_texture_name_from_ply(ply_path: Path) -> str | None:
    """Try to infer the sidecar texture filename referenced by the PLY.

    Some AvatarSDK PLYs reference an external image (often `model.jpg`). Trimesh
    will try to resolve it relative to the PLY directory, so we stage the user
    provided texture there under the expected name.
    """
    try:
        head = ply_path.read_text(encoding="utf-8", errors="ignore")
    except Exception:
        return None

    # Only scan a prefix to avoid loading huge files into memory.
    head = "\n".join(head.splitlines()[:400])
    lowered = head.lower()

    # Cheap heuristic: find the first token that looks like an image filename.
    for ext in (".jpg", ".jpeg", ".png", ".tga", ".bmp"):
        idx = lowered.find(ext)
        if idx == -1:
            continue
        # Walk backward to find the start of the filename.
        start = idx
        while start > 0 and lowered[start - 1] not in " \t\r\n\"'":
            start -= 1
        candidate = head[start : idx + len(ext)].strip().strip("\"'")
        if candidate:
            return os.path.basename(candidate)

    return None


def _require_trimesh():
    try:
        import trimesh  # type: ignore

        return trimesh
    except Exception as e:
        raise SystemExit(
            "Missing dependency: trimesh. Install with: pip install trimesh\n"
            f"Original error: {e}"
        )


def _ensure_obj_has_mtllib(obj_path: Path, mtl_name: str, material_name: str) -> None:
    text = obj_path.read_text(encoding="utf-8", errors="ignore").splitlines()
    has_mtllib = any(line.strip().lower().startswith("mtllib ") for line in text)
    if not has_mtllib:
        text.insert(0, f"mtllib {mtl_name}")

    has_usemtl = any(line.strip().lower().startswith("usemtl ") for line in text)
    if not has_usemtl:
        # Insert after mtllib (line 0) and any comments.
        insert_at = 1
        while insert_at < len(text) and (text[insert_at].startswith("#") or not text[insert_at].strip()):
            insert_at += 1
        text.insert(insert_at, f"usemtl {material_name}")

    obj_path.write_text("\n".join(text) + "\n", encoding="utf-8")


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--mesh-zip", required=True, help="Path to <avatar>_mesh.zip downloaded from AvatarSDK")
    p.add_argument("--texture", required=True, help="Path to <avatar>_texture.jpg downloaded from AvatarSDK")
    p.add_argument("--outdir", required=True, help="Output directory")
    p.add_argument("--name", default="avatar", help="Base name for output files")
    args = p.parse_args()

    mesh_zip = Path(args.mesh_zip)
    texture = Path(args.texture)
    outdir = Path(args.outdir)

    if not mesh_zip.exists():
        print(f"Error: mesh zip not found: {mesh_zip}")
        return 2
    if not texture.exists():
        print(f"Error: texture not found: {texture}")
        return 2

    outdir.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory(prefix="avatarsdk_mesh_") as td:
        tmp = Path(td)

        with zipfile.ZipFile(mesh_zip, "r") as z:
            ply_candidates = [n for n in z.namelist() if n.lower().endswith(".ply")]
            if not ply_candidates:
                print("Error: mesh zip did not contain a .ply file")
                print("Contents:")
                for n in z.namelist():
                    print(f"- {n}")
                return 3

            # Prefer model.ply if present.
            ply_name = "model.ply" if "model.ply" in z.namelist() else ply_candidates[0]
            z.extract(ply_name, path=tmp)

        ply_path = tmp / ply_name
        if not ply_path.exists():
            # Handle nested folder in zip
            ply_path = next(tmp.rglob("*.ply"), None)  # type: ignore
            if not ply_path:
                print("Error: failed to locate extracted .ply")
                return 3

        # Stage the texture next to the PLY under the name the PLY expects.
        # This prevents trimesh from warning/failing when it tries to resolve
        # an external image reference.
        expected_tex_name = _guess_texture_name_from_ply(ply_path) or "model.jpg"
        staged_tex_path = ply_path.parent / expected_tex_name
        try:
            shutil.copy2(texture, staged_tex_path)
        except Exception:
            # Non-fatal; conversion can still succeed.
            staged_tex_path = None

        trimesh = _require_trimesh()

        mesh = trimesh.load(str(ply_path), force="mesh")
        if mesh is None:
            print("Error: trimesh failed to load mesh")
            return 3

        obj_path = outdir / f"{args.name}.obj"
        mtl_name = f"{args.name}.mtl"
        mtl_path = outdir / mtl_name

        # Export OBJ. trimesh may or may not write MTL depending on visuals.
        exported = mesh.export(file_type="obj")
        if isinstance(exported, bytes):
            obj_path.write_bytes(exported)
        else:
            obj_path.write_text(str(exported), encoding="utf-8")

        # Copy texture into outdir with stable name
        tex_dest = outdir / f"{args.name}{texture.suffix.lower()}"
        shutil.copy2(texture, tex_dest)

        # Write a minimal MTL (Unreal can import OBJ+MTL+texture; material setup may still be manual).
        material_name = "avatar_mat"
        mtl_path.write_text(
            "\n".join(
                [
                    f"newmtl {material_name}",
                    "Kd 1.000 1.000 1.000",
                    f"map_Kd {tex_dest.name}",
                    "",
                ]
            ),
            encoding="utf-8",
        )

        _ensure_obj_has_mtllib(obj_path, mtl_name=mtl_name, material_name=material_name)

    print(f"Wrote: {obj_path}")
    print(f"Wrote: {mtl_path}")
    print(f"Copied: {tex_dest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
