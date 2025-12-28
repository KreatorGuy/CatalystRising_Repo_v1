"""Simple Avatar SDK client helper.

This helper targets the itSeez3D AvatarSDK Cloud Web API ("MetaPerson").

Defaults:
- Base URL: https://api.avatarsdk.com
- Create avatar from photo: POST /avatars/
- Poll status: GET /avatars/{code}/

Auth:
- Uses OAuth2 access token in `Authorization: Bearer <token>`.
- Some modes require `X-PlayerUID` (client access mode). Set `AVATAR_SDK_PLAYER_UID`.

Notes:
- Do NOT commit tokens/keys to source control.
"""
from __future__ import annotations

import mimetypes
import json
import os
import time
import typing as t
import requests

DEFAULT_BASE_URL = os.environ.get("AVATAR_SDK_BASE_URL", "https://api.avatarsdk.com")
DEFAULT_PLAYER_UID = os.environ.get("AVATAR_SDK_PLAYER_UID")


class AvatarJobError(RuntimeError):
    pass


class AvatarAuthError(RuntimeError):
    pass


def _auth_headers(access_token: str | None, player_uid: str | None = DEFAULT_PLAYER_UID) -> dict:
    headers: dict[str, str] = {}
    if access_token:
        headers["Authorization"] = f"Bearer {access_token}"
    if player_uid:
        headers["X-PlayerUID"] = player_uid
    return headers


def get_access_token_client_credentials(
    client_id: str,
    client_secret: str,
    base_url: str = DEFAULT_BASE_URL,
    token_path: str = "/o/token/",
    *,
    timeout: float = 30.0,
) -> str:
    """Retrieve an OAuth2 access token using the client_credentials grant.

    Docs: https://api.avatarsdk.com/ (Authentication with OAuth 2.0)
    """
    if not base_url:
        base_url = DEFAULT_BASE_URL
    url = base_url.rstrip("/") + token_path
    data = {
        "grant_type": "client_credentials",
        "client_id": client_id,
        "client_secret": client_secret,
    }
    resp = requests.post(url, data=data, timeout=timeout)
    if resp.status_code >= 400:
        raise AvatarAuthError(f"Token request failed: HTTP {resp.status_code}: {resp.text}")
    payload = resp.json()
    token = payload.get("access_token")
    if isinstance(token, str) and token:
        return token
    raise AvatarAuthError(f"Token response missing access_token: {payload}")


def submit_photo(
    access_token: str,
    image_path: str,
    base_url: str = DEFAULT_BASE_URL,
    submit_path: str = "/avatars/",
    *,
    name: str | None = None,
    pipeline: str = "metaperson_2.0",
    pipeline_subtype: str | None = None,
    parameters: dict | None = None,
    export_parameters: dict | None = None,
    player_uid: str | None = DEFAULT_PLAYER_UID,
) -> str:
    """Create an avatar from a photo.

    itSeez3D Cloud Web API:
    - POST /avatars/ (multipart/form-data)
    - Returns JSON including `code`.
    """
    if not base_url:
        base_url = DEFAULT_BASE_URL

    url = base_url.rstrip("/") + submit_path
    headers = _auth_headers(access_token, player_uid)
    mime, _ = mimetypes.guess_type(image_path)
    if not mime:
        mime = "image/jpeg"

    if name is None:
        name = os.path.splitext(os.path.basename(image_path))[0]

    data: dict[str, t.Any] = {"name": name, "pipeline": pipeline}
    if pipeline_subtype:
        data["pipeline_subtype"] = pipeline_subtype
    if parameters is not None:
        data["parameters"] = json.dumps(parameters)
    if export_parameters is not None:
        data["export_parameters"] = json.dumps(export_parameters)

    with open(image_path, "rb") as f:
        files = {"photo": (os.path.basename(image_path), f, mime)}
        resp = requests.post(url, headers=headers, data=data, files=files, timeout=120)

    resp.raise_for_status()
    data = resp.json()
    code = data.get("code")
    if isinstance(code, str) and code:
        return code
    raise AvatarJobError(f"Unexpected create-avatar response shape: {data}")


def poll_job(
    access_token: str,
    job_id: str,
    base_url: str = DEFAULT_BASE_URL,
    status_path_template: str = "/avatars/{id}/",
    poll_interval: float = 3.0,
    timeout: float = 600.0,
    *,
    player_uid: str | None = DEFAULT_PLAYER_UID,
) -> dict:
    """Poll avatar generation until completion.

    For itSeez3D Web API, this polls GET /avatars/{code}/ until status is completed.
    """
    if not base_url:
        base_url = DEFAULT_BASE_URL

    url_template = base_url.rstrip("/") + status_path_template
    start = time.time()
    headers = _auth_headers(access_token, player_uid)
    last_status: str | None = None

    while True:
        url = url_template.format(id=job_id)
        resp = requests.get(url, headers=headers, timeout=60)
        resp.raise_for_status()
        data = resp.json()

        status = data.get("status")
        if isinstance(status, str):
            last_status = status
            if status.lower() in ("completed", "done", "ready", "success"):
                return data
            if status.lower() in ("failed", "error"):
                raise AvatarJobError(f"Avatar job failed: {data}")

        if time.time() - start > timeout:
            raise AvatarJobError(f"Timeout while waiting for avatar job to complete (last status: {last_status})")
        time.sleep(poll_interval)


def extract_download_url(result_json: dict) -> t.Optional[str]:
    """Best-effort extraction of a direct download URL from an avatar payload.

    For itSeez3D Web API, the recommended approach is to use the exports API.
    This remains as a fallback for any payloads that do include a direct URL.
    """
    for key in ("download_url", "asset_url", "artifact"):
        v = result_json.get(key)
        if isinstance(v, str) and v.startswith("http"):
            return v
    for v in result_json.values():
        if isinstance(v, str) and v.startswith("http") and any(v.endswith(ext) for ext in (".fbx", ".gltf", ".glb", ".zip")):
            return v
    return None


def list_exports(
    access_token: str,
    avatar_code: str,
    base_url: str = DEFAULT_BASE_URL,
    exports_path_template: str = "/avatars/{code}/exports/",
    *,
    player_uid: str | None = DEFAULT_PLAYER_UID,
) -> dict:
    if not base_url:
        base_url = DEFAULT_BASE_URL
    url = base_url.rstrip("/") + exports_path_template.format(code=avatar_code)
    headers = _auth_headers(access_token, player_uid)
    resp = requests.get(url, headers=headers, timeout=60)
    resp.raise_for_status()
    return resp.json()


def select_first_export_file(exports_json: t.Any) -> tuple[str, str] | None:
    """Return (export_code, file_identity) for the first available export file.

    The official API returns a JSON array of export DTOs.
    """
    items: list[dict] = []
    if isinstance(exports_json, list):
        items = [x for x in exports_json if isinstance(x, dict)]
    elif isinstance(exports_json, dict):
        v = exports_json.get("results")
        if isinstance(v, list):
            items = [x for x in v if isinstance(x, dict)]
        elif isinstance(exports_json.get("exports"), list):
            items = [x for x in exports_json.get("exports") if isinstance(x, dict)]

    for export_item in items:
        status = export_item.get("status")
        if isinstance(status, str) and status.lower() not in ("completed", "done", "ready", "success"):
            continue
        export_code = export_item.get("code")
        if not (isinstance(export_code, str) and export_code):
            continue
        files = export_item.get("files")
        if isinstance(files, list) and files:
            for file_entry in files:
                if isinstance(file_entry, dict):
                    identity = file_entry.get("identity")
                    if isinstance(identity, str) and identity:
                        return export_code, identity
                elif isinstance(file_entry, str) and file_entry:
                    return export_code, file_entry
    return None


def build_export_file_download_url(
    avatar_code: str,
    export_code: str,
    export_file: str,
    base_url: str = DEFAULT_BASE_URL,
    download_path_template: str = "/avatars/{code}/exports/{export_code}/files/{export_file}/file/",
) -> str:
    if not base_url:
        base_url = DEFAULT_BASE_URL
    return base_url.rstrip("/") + download_path_template.format(
        code=avatar_code,
        export_code=export_code,
        export_file=export_file,
    )


def download_asset(url: str, dest_path: str, api_key: t.Optional[str] = None, player_uid: str | None = DEFAULT_PLAYER_UID) -> None:
    headers = _auth_headers(api_key, player_uid)
    with requests.get(url, headers=headers, stream=True, timeout=120) as r:
        r.raise_for_status()
        os.makedirs(os.path.dirname(dest_path) or ".", exist_ok=True)
        with open(dest_path, "wb") as f:
            for chunk in r.iter_content(chunk_size=8192):
                if chunk:
                    f.write(chunk)


if __name__ == "__main__":
    print("This module is a library. Use avatar_sdk_cli.py for CLI usage.")
