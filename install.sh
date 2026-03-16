#!/usr/bin/env bash
set -e

REPO="$(cd "$(dirname "$(realpath "$0")")" && pwd)"

# Wrapper script
mkdir -p ~/.local/bin
ln -sf "$REPO/bin/huh" ~/.local/bin/huh
echo "Linked: ~/.local/bin/huh → $REPO/bin/huh"

# Bash completion
mkdir -p ~/.local/share/bash-completion/completions
cp "$REPO/completions/huh.bash" ~/.local/share/bash-completion/completions/huh
echo "Installed bash completion"

# Zsh completion (only if zsh is present)
if command -v zsh &>/dev/null; then
    mkdir -p ~/.local/share/zsh/site-functions
    cp "$REPO/completions/_huh" ~/.local/share/zsh/site-functions/_huh
    echo "Installed zsh completion"
fi

# Fish completion (only if fish config dir exists)
if [[ -d ~/.config/fish ]]; then
    mkdir -p ~/.config/fish/completions
    cp "$REPO/completions/huh.fish" ~/.config/fish/completions/huh.fish
    echo "Installed fish completion"
fi

echo ""
echo "Done. Ensure ~/.local/bin is in your PATH."
echo "For bash completion to activate, open a new shell or run:"
echo "  source ~/.local/share/bash-completion/completions/huh"
