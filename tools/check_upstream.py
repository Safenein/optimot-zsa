#!/usr/bin/env python3
"""Vérifie si le pilote Optimot Ergo publié sur optimot.fr diffère de vendor/optimot/.

Source : la page « Pilotes pour Linux » d'optimot.fr, dont le bundle JavaScript contient
le lien de téléchargement de l'archive Ergo (Optimot_X.Y/linux/Optimot_Linux_Ergo_NN.zip).
Le .xkb et le .XCompose de l'archive sont comparés octet par octet aux fichiers vendorisés.

Codes de sortie : 0 à jour, 1 mise à jour disponible, 2 erreur (site injoignable, format changé).

Options :
  --update        remplace les fichiers de vendor/optimot/ par ceux d'amont
  --report-issue  ouvre une issue Forgejo (variables FORGEJO_API, FORGEJO_REPO, FORGEJO_TOKEN)
"""

import argparse
import hashlib
import io
import json
import os
import re
import sys
import urllib.error
import urllib.request
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
VENDOR = ROOT / "vendor/optimot"
LOCAL_XKB = VENDOR / "Optimot_Ergo_1.8.0.xkb"
LOCAL_XCOMPOSE = VENDOR / "Optimot_Ergo_1.8.0.XCompose"

SITE = "https://optimot.fr"
ISSUE_TITLE = "Mise à jour du pilote Optimot disponible"


def fetch(url):
    req = urllib.request.Request(url, headers={"User-Agent": "optimot-zsa-upstream-check"})
    with urllib.request.urlopen(req, timeout=60) as r:
        return r.read()


def fail(msg):
    print(f"erreur : {msg}", file=sys.stderr)
    sys.exit(2)


def upstream_zip_url():
    page = fetch(f"{SITE}/linux.html").decode()
    assets = sorted(set(re.findall(r'assets/linux\.md\.[\w.-]+\.js', page)))
    if not assets:
        fail("bundle JavaScript de la page Linux introuvable")
    for asset in assets:
        js = fetch(f"{SITE}/{asset}").decode()
        m = re.search(r"(Optimot_[\d.]+/linux/Optimot_Linux_Ergo_\d+\.zip)", js)
        if m:
            return f"{SITE}/downloads/{m.group(1)}"
    fail("lien de l'archive Ergo introuvable dans la page Linux")


def layout_name(xkb_text):
    m = re.search(r'name\[Group1\]\s*=\s*"([^"]+)"', xkb_text)
    return m.group(1) if m else "?"


def sha(data):
    return hashlib.sha256(data).hexdigest()[:16]


def report_issue(body):
    api, repo, token = (os.environ.get(k) for k in ("FORGEJO_API", "FORGEJO_REPO", "FORGEJO_TOKEN"))
    if not (api and repo and token):
        fail("FORGEJO_API, FORGEJO_REPO et FORGEJO_TOKEN sont requis pour --report-issue")
    headers = {"Authorization": f"token {token}", "Content-Type": "application/json"}

    def call(method, path, payload=None):
        data = json.dumps(payload).encode() if payload is not None else None
        req = urllib.request.Request(f"{api}/repos/{repo}{path}", data=data, method=method, headers=headers)
        with urllib.request.urlopen(req, timeout=60) as r:
            return json.loads(r.read() or b"null")

    for issue in call("GET", "/issues?state=open&type=issues&limit=50"):
        if issue["title"].startswith(ISSUE_TITLE):
            print(f"issue déjà ouverte : {issue['html_url']}")
            return
    issue = call("POST", "/issues", {"title": ISSUE_TITLE, "body": body})
    print(f"issue ouverte : {issue['html_url']}")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--update", action="store_true")
    ap.add_argument("--report-issue", action="store_true")
    args = ap.parse_args()

    try:
        url = upstream_zip_url()
        archive = zipfile.ZipFile(io.BytesIO(fetch(url)))
    except (urllib.error.URLError, zipfile.BadZipFile) as e:
        fail(f"téléchargement impossible : {e}")

    def member(suffix):
        names = [n for n in archive.namelist() if n.endswith(suffix) and "__MACOSX" not in n]
        if len(names) != 1:
            fail(f"{suffix} : {len(names)} fichier(s) dans l'archive au lieu de 1")
        return archive.read(names[0])

    up_xkb, up_compose = member(".xkb"), member(".XCompose")
    loc_xkb, loc_compose = LOCAL_XKB.read_bytes(), LOCAL_XCOMPOSE.read_bytes()

    local_name = layout_name(loc_xkb.decode())
    upstream_name = layout_name(up_xkb.decode())
    changed = [
        name
        for name, up, loc in (("xkb", up_xkb, loc_xkb), ("XCompose", up_compose, loc_compose))
        if up != loc
    ]

    lines = [
        f"- Archive amont : {url}",
        f"- Version vendorisée : **{local_name}**",
        f"- Version amont : **{upstream_name}**",
        f"- xkb : local `{sha(loc_xkb)}` / amont `{sha(up_xkb)}`",
        f"- XCompose : local `{sha(loc_compose)}` / amont `{sha(up_compose)}`",
    ]
    summary = "\n".join(lines)
    print(summary)

    step_summary = os.environ.get("GITHUB_STEP_SUMMARY")
    if step_summary:
        state = "Mise à jour disponible" if changed else "À jour"
        Path(step_summary).write_text(f"## Pilote Optimot : {state}\n\n{summary}\n")

    if not changed:
        print("à jour")
        return 0

    print(f"mise à jour disponible ({', '.join(changed)} modifié(s))")
    if args.update:
        LOCAL_XKB.write_bytes(up_xkb)
        LOCAL_XCOMPOSE.write_bytes(up_compose)
        print(f"vendor/optimot/ mis à jour (renommer les fichiers si la version a changé : {upstream_name})")
    if args.report_issue:
        report_issue(
            "Le pilote Linux Optimot Ergo publié sur optimot.fr diffère de `vendor/optimot/`.\n\n"
            f"{summary}\n\n"
            f"Fichiers modifiés : {', '.join(changed)}.\n\n"
            "Pour intégrer la mise à jour :\n\n"
            "```sh\n"
            "python3 tools/check_upstream.py --update\n"
            "devenv shell run-tests\n"
            "devenv shell build\n"
            "```\n"
        )
    return 1


if __name__ == "__main__":
    sys.exit(main())
