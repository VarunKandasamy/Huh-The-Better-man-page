# Bash completion for huh
# Place in /etc/bash_completion.d/huh  OR
# ~/.local/share/bash-completion/completions/huh

_huh() {
    local cur="${COMP_WORDS[COMP_CWORD]}"
    # Query the whatis database (pre-built by mandb) — fast and only returns
    # entries that actually have a man page.
    local pages
    pages=$(man -k . 2>/dev/null | awk '{print $1}' | sort -u)
    COMPREPLY=($(compgen -W "$pages" -- "$cur"))
}

complete -F _huh huh
