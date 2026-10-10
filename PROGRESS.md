# Progress Tracker

<!-- Claude updates this file every time a day is taught. Keep the "Current state" block machine-simple. -->

## Current state
- **Next day to teach:** 3
- **Last day taught:** 2
- **Last session date:** 2026-10-08
- **Current phase:** Phase 1 — Core Language & Memory
- **Toolchain:** MSYS2 MinGW g++ 16.1 on Windows (`.dll` not `.so`, no `ldd`, sanitizers unavailable → use WSL for Day 79)

## Completed days
| Day | Date | Topic | Implementations | Status |
|-----|------|-------|-----------------|--------|
| 01 | 2026-10-07 | Compilation pipeline, static vs dynamic linking | pricing lib (mid/spread/to_ticks/is_crossed), static+shared lib, linking benchmark | taught |
| 02 | 2026-10-08 | Translation units, declaration vs definition, ODR, headers, guards/#pragma once, forward decls | instrument registry (fixed array), OrderBook::price_to_ticks, break-the-ODR lab, header hygiene on backtester | taught |

## Implementations done
- [ ] Day 01 pricing lib (not started as of 2026-10-08, carried over)
<!-- Tick off as the user finishes them (from PLAN.md "Build from scratch" column) -->

## Weak areas / revisit
<!-- Things the user struggled with; bring them back in review days and as warm-up questions -->
