"""CLI wrapper around avatar_sdk_client.py (itSeez3D AvatarSDK Cloud Web API).

Auth:
- Prefer providing an OAuth2 access token via `--access-token` or AVATAR_SDK_ACCESS_TOKEN.
- Or provide AVATAR_SDK_CLIENT_ID + AVATAR_SDK_CLIENT_SECRET and this CLI will fetch a token.

Example (PowerShell):
  $env:AVATAR_SDK_ACCESS_TOKEN = "<token>"
  python Tools/avatar_sdk_cli.py --image ./photo.jpg --outdir ./Generated
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path

import requests

from avatar_sdk_client import (
    DEFAULT_BASE_URL,
    get_access_token_client_credentials,
    submit_photo,
    poll_job,
    extract_download_url,
    list_exports,
    select_first_export_file,
    download_asset,
)


def _print_http_error(prefix: str, err: requests.HTTPError) -> None:
    resp = getattr(err, "response", None)
    status = getattr(resp, "status_code", None)
    body = getattr(resp, "text", None)
    print(prefix)
    if status is not None:
        print(f"HTTP {status}")
    if body:
        print(body)


def main(argv=None):
    p = argparse.ArgumentParser()
    p.add_argument("--image", required=True, help="Local photo file to upload")
    p.add_argument("--outdir", required=True, help="Output directory for downloaded asset")
    p.add_argument("--name", default=None, help="Avatar name (defaults to image filename)")
    p.add_argument("--access-token", help="OAuth2 access token (or set AVATAR_SDK_ACCESS_TOKEN env var)")
    p.add_argument(
        "--api-key",
        help="(Deprecated) Treated as OAuth2 access token for backward-compat. Prefer --access-token.",
    )
    p.add_argument("--client-id", help="OAuth2 client_id (or set AVATAR_SDK_CLIENT_ID env var)")
    p.add_argument("--client-secret", help="OAuth2 client_secret (or set AVATAR_SDK_CLIENT_SECRET env var)")
    p.add_argument("--base-url", default=DEFAULT_BASE_URL, help="Base URL for Avatar SDK API")
    p.add_argument("--submit-path", default="/avatars/", help="Submit endpoint path")
    p.add_argument("--status-path", default="/avatars/{id}/", help="Status endpoint path template")
    p.add_argument("--pipeline", default="metaperson_2.0", help="Pipeline to use (itSeez3D Cloud)")
    p.add_argument("--pipeline-subtype", default=None, help="Optional pipeline subtype")
    p.add_argument("--poll-interval", type=float, default=3.0)
    p.add_argument("--timeout", type=float, default=600.0)
    args = p.parse_args(argv)

    base_url = args.base_url or DEFAULT_BASE_URL

    access_token = args.access_token or os.environ.get("AVATAR_SDK_ACCESS_TOKEN")
    if not access_token and args.api_key:
        access_token = args.api_key

    if not access_token:
        client_id = args.client_id or os.environ.get("AVATAR_SDK_CLIENT_ID")
        client_secret = args.client_secret or os.environ.get("AVATAR_SDK_CLIENT_SECRET")
        if not (client_id and client_secret):
            print(
                "Error: no access token. Provide --access-token (recommended), or set AVATAR_SDK_ACCESS_TOKEN,\n"
                "or provide --client-id/--client-secret (or AVATAR_SDK_CLIENT_ID/AVATAR_SDK_CLIENT_SECRET) to fetch one."
            )
            return 2

        print("Fetching OAuth access token (client_credentials)...")
        try:
            access_token = get_access_token_client_credentials(client_id, client_secret, base_url=base_url)
        except Exception as e:
            print("Error: failed to fetch OAuth token.")
            print(str(e))
            print(
                "Hint: HTTP 401 {\"error\":\"invalid_client\"} usually means the client_id/client_secret pair is wrong,\n"
                "or you're using the wrong credentials type (it must be an OAuth app's client_id + client_secret from accounts.avatarsdk.com)."
            )
            return 2

    image = Path(args.image)
    if not image.exists():
        print(f"Error: image not found: {image}")
        return 2

    outdir = Path(args.outdir)
    outdir.mkdir(parents=True, exist_ok=True)

    resolved_pipeline_subtype = args.pipeline_subtype
    if args.pipeline == "metaperson_2.0" and not resolved_pipeline_subtype:
        resolved_pipeline_subtype = "female"
        print("Note: metaperson_2.0 requires --pipeline-subtype (female|male); defaulting to female")

    print("Submitting photo...")
    try:
        avatar_code = submit_photo(
            access_token,
            str(image),
            base_url,
            submit_path=args.submit_path,
            name=args.name,
            pipeline=args.pipeline,
            pipeline_subtype=resolved_pipeline_subtype,
        )
    except requests.HTTPError as e:
        _print_http_error("Error: avatar create request failed.", e)
        print(f"URL: {base_url.rstrip('/')}{args.submit_path}")
        resp = getattr(e, "response", None)
        status = getattr(resp, "status_code", None)
        body = getattr(resp, "text", "") or ""
        if status == 401:
            print(
                "Hint: 401 here means the Bearer token is missing/invalid/expired, or was issued for a different environment.\n"
                "Use AVATAR_SDK_ACCESS_TOKEN from the correct OAuth flow, or let the CLI fetch one via client_id/client_secret."
            )
        elif status == 403:
            print(
                "Hint: 403 often means your subscription plan doesn't allow this pipeline (metaperson_2.0 REST is Enterprise-only per docs).\n"
                "Try rerunning with `--pipeline animated_face` to confirm your auth is valid."
            )
        elif status == 400 and "pipeline_subtype" in body and args.pipeline == "metaperson_2.0":
            print("Hint: metaperson_2.0 requires `--pipeline-subtype female` or `--pipeline-subtype male`.")
        return 3

    print(f"Submitted as avatar: {avatar_code}")

    print("Waiting for job to complete...")
    try:
        avatar_json = poll_job(
            access_token,
            avatar_code,
            base_url,
            status_path_template=args.status_path,
            poll_interval=args.poll_interval,
            timeout=args.timeout,
        )
    except requests.HTTPError as e:
        _print_http_error("Error: polling avatar status failed.", e)
        return 3

    exports_json = None
    selected = None
    try:
        exports_json = list_exports(access_token, avatar_code, base_url)
        selected = select_first_export_file(exports_json)
    except Exception:
        exports_json = None
        selected = None

    if selected is not None:
        export_code, export_file_identity = selected
        dl = None

        if isinstance(exports_json, list):
            for export_item in exports_json:
                if not isinstance(export_item, dict):
                    continue
                if export_item.get("code") != export_code:
                    continue
                files = export_item.get("files")
                if isinstance(files, list):
                    for f in files:
                        if isinstance(f, dict) and f.get("identity") == export_file_identity:
                            file_url = f.get("file")
                            if isinstance(file_url, str) and file_url.startswith("http"):
                                dl = file_url
                                break
                if dl:
                    break

        if not dl:
            print("Could not resolve export file download URL from exports list. Inspect exports JSON:")
            print(exports_json)
            return 3

        filename = f"{export_file_identity}.zip"
        dest = outdir / filename
        print(f"Downloading export file to: {dest}")
        try:
            download_asset(dl, str(dest), api_key=access_token)
        except requests.HTTPError as e:
            _print_http_error("Error: downloading export file failed.", e)
            return 3
    else:
        dl = extract_download_url(avatar_json)
        if dl:
            filename = os.path.basename(dl.split("?")[0])
            dest = outdir / filename
            print(f"Downloading asset to: {dest}")
            try:
                download_asset(dl, str(dest), api_key=access_token)
            except requests.HTTPError as e:
                _print_http_error("Error: downloading asset failed.", e)
                return 3
        else:
            # Pipelines like animated_face provide direct mesh/texture endpoints.
            mesh_url = avatar_json.get("mesh") if isinstance(avatar_json, dict) else None
            texture_url = avatar_json.get("texture") if isinstance(avatar_json, dict) else None

            if isinstance(mesh_url, str) and mesh_url.startswith("http"):
                dest = outdir / f"{avatar_code}_mesh.zip"
                print(f"Downloading mesh to: {dest}")
                try:
                    download_asset(mesh_url, str(dest), api_key=access_token)
                except requests.HTTPError as e:
                    _print_http_error("Error: downloading mesh failed.", e)
                    return 3

            if isinstance(texture_url, str) and texture_url.startswith("http"):
                dest = outdir / f"{avatar_code}_texture.jpg"
                print(f"Downloading texture to: {dest}")
                try:
                    download_asset(texture_url, str(dest), api_key=access_token)
                except requests.HTTPError as e:
                    _print_http_error("Error: downloading texture failed.", e)
                    return 3

            if not (isinstance(mesh_url, str) and mesh_url.startswith("http")) and not (
                isinstance(texture_url, str) and texture_url.startswith("http")
            ):
                print("No export file or downloadable mesh/texture URL found. Inspect the avatar JSON:")
                print(avatar_json)
                if exports_json is not None:
                    print("Exports JSON:")
                    print(exports_json)
                return 3

    print("Done. You can import the downloaded file into Unreal Editor or run the Unreal import helper script.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
