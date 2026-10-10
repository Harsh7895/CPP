# learn_cpp — Context for Claude

## What this repo is
A 90-day C++ study plan. The goal is a **Quant Developer / HFT C++** role.
The user can give **1–1.5 hrs/day** and covers 1–2 topics per day. They already practise DSA and are building a C++ backtester, so use trading/backtester examples where they fit naturally.

## Key files (read these first in every session)
- [PROGRESS.md](PROGRESS.md): **source of truth for where we are.** It records the next day to teach, the completed days, the finished implementations and the weak areas.
- [PLAN.md](PLAN.md): the 90-day plan. It maps each day to its topic, the thing to build and the phase folder.
- [skill.md](skill.md): the full syllabus (57 sections), the priority tiers and the required daily format.
- `.claude/skills/teach-today/SKILL.md`: the daily teaching workflow.

## Trigger phrases
When the user says **"teach today's topic"**, "teach today", "next day", "start day N" or something similar, run the `teach-today` skill. Never ask which day it is. Read PROGRESS.md and continue from there. Everything in the Completed days table has already been taught, so build on it and don't re-teach it.

## Folder layout (keep it simple, no extra folders)
```
learn_cpp/
├── CLAUDE.md  PLAN.md  PROGRESS.md  skill.md
└── phaseN_<name>/
    └── dayNN_<short_topic>/
        ├── README.md        # the full 9-section lesson
        ├── impl.cpp         # skeleton + TODOs + tests: the user implements from scratch
        ├── exercises.cpp    # output-prediction + debugging snippets
        └── solution.cpp     # ONLY created when the user asks for the solution
```
Phase folder names come from PLAN.md. Create a day folder only on the day it is taught.

## Teaching rules
- Every day uses the 9-section format: Theory → Internals → Implementation → Output Questions → Debugging → Performance → Interview Questions (10–20) → Hard Questions (3–5) → Mini Project.
- **Implement from scratch is mandatory.** Give skeletons, interfaces, TODOs and tests, not finished code. Full solutions only on request.
- Prefer depth and "how it actually works" over definitions. Always cover complexity, cache, allocation and latency.
- Start each day with a 2–3 question warm-up on earlier days, especially the "Weak areas" in PROGRESS.md.
- Target C++20. Code must compile with `g++ -std=c++20 -O2 -Wall -Wextra -pedantic`.

## Git
- After each day's material is generated (and PROGRESS.md updated), **automatically commit and push** to GitHub with the message `Day NN: <topic>`. The user has pre-approved this push.
- Before generating a new day, also commit any uncommitted work the user did on earlier days (`Day NN: my implementation`).
- Never force-push. Never commit build outputs (see .gitignore).
- **No attribution of any kind.** Never add `Co-Authored-By: Claude` or other Claude/AI trailers to commit messages. The user must be the only contributor.
- CLAUDE.md, PLAN.md, PROGRESS.md, skill.md and .claude/ are git-ignored on purpose (local only). Don't stage them.
