#!/usr/bin/env bash
set -e
nix $1 --extra-experimental-features nix-command --extra-experimental-features flakes
