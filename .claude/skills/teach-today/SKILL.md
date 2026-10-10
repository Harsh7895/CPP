---
name: teach-today
description: Teach the next day of the 90-day C++ Quant/HFT plan. Use when the user says "teach today's topic", "teach today", "next day", "start day N", or similar. Reads PROGRESS.md to know what has already been taught, generates today's lesson folder in the 9-section format, updates PROGRESS.md, then commits and pushes to GitHub.
---

# Teach Today

## 1. Load context (don't ask the user anything you can read)
1. Read `PROGRESS.md` to get **Next day to teach** (N), the completed days and the weak areas.
   - If the user named a specific day ("start day 12"), use that one instead.
2. Read the row for Day N in `PLAN.md` to get the topic, the build target and the phase folder.
3. Check the relevant section of `skill.md` for the full sub-topic list of that area.
4. Look at the previous day's folder. If the user changed `impl.cpp` (check `git status`/`git diff`), review it briefly: 3–6 bullets on bugs, UB, missed edge cases and performance. Then commit it:
   `git add <previous day folder> && git commit -m "Day <N-1>: my implementation"` (no attribution trailer) (skip this if nothing changed).

## 2. Generate the lesson
Create `<phase folder>/dayNN_<short_snake_topic>/` (zero-padded NN) with:

### README.md: these exact sections
```
# Day NN — <Topic>
> Phase X · Est. time 60–90 min · Builds on: Day a, Day b

## 0. Warm-up (from earlier days / weak areas): 2–3 quick questions, answers hidden in <details>
## 1. Theory: 5–10 concepts, crisp, with small code snippets
## 2. Internals: how it actually works (memory layout, ASCII diagrams, what the compiler/library/CPU does)
## 3. Implementation: what to build in impl.cpp: interface, requirements, edge cases, hints
## 4. Output Questions: 4–6 "predict the output" snippets (answers in <details>)
## 5. Debugging: 3–5 buggy/UB snippets to fix (answers in <details>)
## 6. Performance: complexity, cache behaviour, allocations, latency implications (HFT angle)
## 7. Interview Questions: 10–20 (short model answers in <details>)
## 8. Hard Questions: 3–5 senior-level (model answers in <details>)
## 9. Mini Project: apply the concept, ideally tied to trading/backtesting
## Checklist: [ ] read theory [ ] impl.cpp passes tests [ ] exercises done [ ] mini project
```

### impl.cpp
- A compilable skeleton: class/function signatures with `// TODO` bodies, plus a `main()` of `assert`-based tests that fail until the user implements things.
- **Do not write the solution.** The user implements it from scratch.
- Put the build command at the top: `// g++ -std=c++20 -O2 -Wall -Wextra -pedantic impl.cpp -o impl`

### exercises.cpp
- The output-prediction and debugging snippets from sections 4–5 as separate functions, so the user can run them.

Fit the scope to 60–90 minutes. Depth beats breadth. Use the user's Quant/HFT goal and their backtester for examples.

## 3. Update PROGRESS.md
- Increment **Next day to teach** to N+1, set **Last day taught** to N and **Last session date** to today, and update **Current phase**.
- Add a row to "Completed days": `| NN | YYYY-MM-DD | topic | build target | taught |`.
- When the user later says they finished or struggled with something, update "Implementations done" or "Weak areas".

## 4. Commit & push (pre-approved by the user)
```
git add <phase folder>/dayNN_<topic>/ .gitignore
git commit -m "Day NN: <topic>"
git push
```
- If there is no remote or the push fails, report it clearly with the error and keep the local commit. Never force-push.
- The commit message is ONLY `Day NN: <topic>`. **Never add a `Co-Authored-By: Claude` trailer or any other attribution.** The user wants to be the sole contributor.
- Stage only the day folder (and `.gitignore` if changed). CLAUDE.md, PLAN.md, PROGRESS.md, skill.md and .claude/ are git-ignored on purpose.

## 5. Reply in chat
Keep it short: the day number and topic, the folder link, what to implement, the build command and the push status. Don't paste the whole README into chat.

## Other commands the user may give
- **"show solution"**: write `solution.cpp` into today's folder, then commit and push.
- **"done" / "finished day N"**: review their `impl.cpp`, mark it done in PROGRESS.md, then commit and push.
- **"I'm weak at X"**: add X to Weak areas.
