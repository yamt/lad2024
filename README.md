# Lad 2024

A puzzle game for the [WASM-4] fantasy console.

![screenshot](img/screenshot.png)

[WASM-4]: https://wasm4.org

## How to play

Maybe it's simplest to run it on your browser.

* [On wasm4.org]

* [A development snapshot] for testing

[On wasm4.org]: https://wasm4.org/play/lad2024

[A development snapshot]: https://yamt.github.io/lad2024/snapshot

### Operations

| WASM-4 Gamepad button      | Operation
| -------------------------- | ------------------------------------------------
| `UP` `DOWN` `RIGHT` `LEFT` | Move the current player
| `X`                        | Toggle players
| `Z` + `UP`                 | Give up and reset the stage
| `Z` + `DOWN`               | Undo a move
| `Z` + `RIGHT`              | Move to the next stage (hold to move faster)
| `Z` + `LEFT`               | Move to the previous stage (hold to move faster)

You can use the mouse (or maybe screen tap on phone-like devices)
for some of operations.

| Left click / tap on                  | Operation
| ------------------------------------ | -----------------------------
| an empty location                    | Move to the location if possible
| the current player                   | Switch to the next player
| an object next to you                | Push it
| the other player (not next to you)   | Switch to the player

### Save data

The game progress is automatically saved and restored.

## History

In 1994, I've written [the original version] of this game.
It was in x86 assembler for the NEC PC-9800 series.

In 2024, I found it in one of my MO disks and ported it
to the [WASM-4] fantasy console.

[the original version]: https://github.com/yamt/lad1994

## Messages to myself in 1994

* Please write comments.

* Please document your domain-specific compression format
  for the stage data.

* I couldn't even figure out what "LAD" stands for.
