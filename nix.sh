#!/usr/bin/env bash
set -e
nix $@ --extra-experimental-features nix-command --extra-experimental-features flakes
