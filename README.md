# Huh — The Better Man Page

> A modernized man page viewer built for tinkerers and developers.

Huh wraps the standard `man` command and renders its output with configurable colors, text effects, and section ordering — all driven by simple TOML config files you write yourself.

---

## Features

- **Per-section styling** — color, bold, italic, and underline for titles, body text, and commands independently
- **Section reordering** — pin any section to the top or bottom of the output
- **Section hiding** — suppress sections you never read (e.g. `COLOPHON`, `REPORTING BUGS`)
- **Config-file driven** — no recompilation needed; drop a `.toml` in your config directory and changes apply immediately

---

## Installation

**Requirements:** CMake 3.31+, a C++20-compatible compiler (GCC or Clang), and `man` installed.
**Optional:** [`bat`](https://github.com/sharkdp/bat) for the best pager experience (falls back to `less -R` if unavailable).

```bash
git clone https://github.com/your-username/Huh-The-Better-man-page
cd Huh-The-Better-man-page
cmake -B build -S .
cmake --build build
bash install.sh
```

`install.sh` symlinks `bin/huh` into `~/.local/bin` and installs completions for whichever shells are present (bash, zsh, fish). Ensure `~/.local/bin` is in your `PATH`.

To update after pulling new changes, just rebuild:

```bash
cmake --build build
```

The symlink continues to point at the repo, so no reinstall is needed.

---

## Usage

```bash
huh <command>
```

Examples:

```bash
huh ls
huh grep
huh curl
```

---

## Configuration

Huh reads all `.toml` files from:

```
~/.config/huhTheBetterManPage/
```

Create that directory and add `.toml` files to customize your output. You can split config across multiple files however you like — they're all loaded.

### Config Schema

```toml
# Defaults applied to any section not explicitly configured
[default]
title   = { color = "CYAN",  bold = true,  italic = false, underline = false }
section = { color = "WHITE", bold = false, italic = false, underline = false }
command = { color = "WHITE", bold = false, italic = false, underline = false }

# Override styling for a specific section (use the uppercase name as it appears in man)
[DESCRIPTION]
title   = { color = "GREEN", bold = true }
section = { color = "WHITE" }
command = { color = "YELLOW" }

# Hide a section entirely
[COLOPHON]
skip = true

# Pin a section to appear before everything else
[SYNOPSIS]
prepend = true

# Pin a section to appear after everything else
[REPORTING BUGS]
postpend = true
```

### Style Options

| Key | Values |
|-----|--------|
| `color` | `RED` `GREEN` `YELLOW` `BLUE` `MAGENTA` `CYAN` `WHITE` |
| `bold` | `true` / `false` |
| `italic` | `true` / `false` |
| `underline` | `true` / `false` |

Each section block has three style targets:
- **`title`** — the section heading line (e.g. `DESCRIPTION`)
- **`section`** — the body text of the section
- **`command`** — command/option strings within the content

Any key omitted from a section block falls back to the `[default]` value.

### Section Ordering

Three flags control where a section appears in the final output:

| Flag | Effect |
|------|--------|
| `prepend = true` | Section renders before all normal sections |
| `postpend = true` | Section renders after all normal sections |
| `skip = true` | Section is omitted entirely |

Sections without any of these flags appear in their original order in the middle.

---

## How It Works

Huh runs `man <command>` internally via `popen()` and processes the output through a pipeline:

```
man output → Parser → Page → Assembler → styled output
```

1. **Parser** splits the raw man text into sections (each section is a title + body pair).
2. **Assembler** loads your TOML config, applies ANSI escape codes to each section's title, body, and command text, then reorders sections according to `prepend`/`postpend`/`skip` flags.
3. The final styled page is printed to stdout.
