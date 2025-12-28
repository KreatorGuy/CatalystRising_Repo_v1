"""Unreal Editor Python helper to import an avatar mesh into Content/Avatars.

Run this from the Unreal Editor's Python console or the Scripts menu. It will
copy the file into the project's Content folder (if needed) and import it.

Usage inside Editor (Python console):
  import unreal_import_avatar
    unreal_import_avatar.import_avatar(r"C:/path/to/downloaded/avatar.fbx", "/Game/Avatars/MyAvatar")
    unreal_import_avatar.import_avatar(r"C:/path/to/downloaded/avatar.obj", "/Game/Avatars/MyAvatar")

This script is intentionally defensive: Unreal's Python APIs change between
versions; if an API call fails, check the printed error and adapt accordingly.
"""
from __future__ import annotations

import os
import shutil

try:
    import unreal
except Exception:
    unreal = None


_OBJ_SIDECAR_EXTS = {
    ".obj",
    ".mtl",
    ".jpg",
    ".jpeg",
    ".png",
    ".tga",
    ".bmp",
}


def _copy_source_and_sidecars(source_file: str, dest_fs_dir: str) -> str:
    """Copy source file into Content folder.

    For OBJ, also copies sidecar files (MTL + common texture formats) from the
    same directory so the importer can resolve them.

    Returns the destination file path (on disk) of the main source file.
    """
    ext = os.path.splitext(source_file)[1].lower()

    if ext == ".obj":
        source_dir = os.path.dirname(source_file)
        for name in os.listdir(source_dir):
            if os.path.splitext(name)[1].lower() in _OBJ_SIDECAR_EXTS:
                src = os.path.join(source_dir, name)
                dst = os.path.join(dest_fs_dir, name)
                if os.path.isfile(src):
                    shutil.copy2(src, dst)
        return os.path.join(dest_fs_dir, os.path.basename(source_file))

    dest_file = os.path.join(dest_fs_dir, os.path.basename(source_file))
    shutil.copy2(source_file, dest_file)
    return dest_file


def _find_texture_sidecar(dest_fs_dir: str, base_name: str) -> str | None:
    for ext in (".jpg", ".jpeg", ".png", ".tga", ".bmp"):
        candidate = os.path.join(dest_fs_dir, f"{base_name}{ext}")
        if os.path.isfile(candidate):
            return candidate
    return None


def _import_file_task(filename: str, dest_package: str, options: object | None = None):
    task = unreal.AssetImportTask()
    task.filename = filename
    task.destination_path = dest_package
    task.automated = True
    task.replace_existing = True
    task.save = True
    if options is not None:
        task.options = options
    return task


def _create_basic_material_from_texture(dest_package: str, base_name: str, texture) -> object | None:
    """Create a simple material with BaseColor driven by texture."""
    if unreal is None or texture is None:
        return None

    try:
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        material_name = f"M_{base_name}"
        material_path = f"{dest_package}/{material_name}"
        if unreal.EditorAssetLibrary.does_asset_exist(material_path):
            return unreal.load_asset(material_path)

        material_factory = unreal.MaterialFactoryNew()
        material = asset_tools.create_asset(material_name, dest_package, unreal.Material, material_factory)
        if material is None:
            return None

        expr = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionTextureSample, -400, 0
        )
        expr.texture = texture
        unreal.MaterialEditingLibrary.connect_material_property(expr, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
        unreal.MaterialEditingLibrary.recompile_material(material)
        unreal.EditorAssetLibrary.save_loaded_asset(material)
        return material
    except Exception as e:
        print("Material creation failed (non-fatal):", e)
        return None


def _assign_material_to_static_mesh(static_mesh, material) -> None:
    if unreal is None or static_mesh is None or material is None:
        return

    # Try common UE5 API first.
    try:
        subsystem = unreal.StaticMeshEditorSubsystem()
        subsystem.set_material(static_mesh, 0, material)
        unreal.EditorAssetLibrary.save_loaded_asset(static_mesh)
        return
    except Exception:
        pass

    # Fallback for API differences.
    try:
        static_mesh.set_material(0, material)
        unreal.EditorAssetLibrary.save_loaded_asset(static_mesh)
    except Exception as e:
        print("Material assignment failed (non-fatal):", e)


def import_avatar(source_file: str, dest_package: str = "/Game/Avatars") -> None:
    """Import `source_file` into the Unreal project under `dest_package`.

    `dest_package` is an Unreal path like '/Game/Avatars/MyAvatar'.
    """
    if unreal is None:
        print("This helper must be run inside Unreal Editor's Python environment.")
        return

    source_file = os.path.abspath(source_file)
    if not os.path.exists(source_file):
        print(f"Source file not found: {source_file}")
        return

    # Ensure destination folder exists
    folder = dest_package
    if not unreal.EditorAssetLibrary.does_directory_exist(folder):
        unreal.EditorAssetLibrary.make_directory(folder)

    # Copy file into project's Content folder so import paths are tidy
    project_content_dir = unreal.SystemLibrary.get_project_content_directory()
    dest_fs_dir = os.path.join(project_content_dir, folder.replace("/Game/", ""))
    os.makedirs(dest_fs_dir, exist_ok=True)
    dest_file = _copy_source_and_sidecars(source_file, dest_fs_dir)

    # Use Asset Tools to import
    try:
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        tasks = []

        ext = os.path.splitext(dest_file)[1].lower()
        if ext == ".fbx":
            options = unreal.FbxImportUI()
            options.import_mesh = True
            options.auto_detect_type = True
            options.import_as_skeletal = False
            tasks.append(_import_file_task(dest_file, folder, options=options))
        else:
            tasks.append(_import_file_task(dest_file, folder))

        # If importing an OBJ, Unreal may ignore sidecar textures/MTL. Import the texture explicitly.
        base_name = os.path.splitext(os.path.basename(source_file))[0]
        if ext == ".obj":
            tex_file = _find_texture_sidecar(dest_fs_dir, base_name)
            if tex_file:
                tasks.append(_import_file_task(tex_file, folder))

        asset_tools.import_asset_tasks(tasks)

        imported_paths = []
        for t in tasks:
            try:
                imported_paths.extend(list(t.get_editor_property("imported_object_paths") or []))
            except Exception:
                continue

        static_mesh = None
        texture = None
        for p in imported_paths:
            try:
                asset = unreal.load_asset(p)
            except Exception:
                asset = None
            if asset is None:
                continue
            if static_mesh is None and isinstance(asset, unreal.StaticMesh):
                static_mesh = asset
            if texture is None and isinstance(asset, unreal.Texture2D):
                texture = asset

        if static_mesh is not None and texture is not None:
            material = _create_basic_material_from_texture(folder, base_name, texture)
            _assign_material_to_static_mesh(static_mesh, material)

        print("Import completed. Check Content Browser.")
    except Exception as e:
        print("Import failed — API mismatch or other error:", e)


if __name__ == "__main__":
    print("This script is for the Unreal Editor Python environment. Import with import_avatar().")
