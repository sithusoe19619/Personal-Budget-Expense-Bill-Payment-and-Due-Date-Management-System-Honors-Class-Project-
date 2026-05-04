# AI User Profile — Sithu Soe

**Purpose:** This file tells any AI agent who the user is, how they collaborate, and what to expect — so you can work effectively from the first message without asking basic questions.

Read this alongside `AI_HANDOFF.md` (the project document). This file is about the *person*; that file is about the *code*.

---

## 1. Who They Are

| Field | Detail |
|---|---|
| Name | Sithu Soe |
| Email | sithusoe19619@gmail.com |
| School | BMCC (Borough of Manhattan Community College) |
| Course | CSC 331H — Honors Data Structures, Spring 2026 |
| Machine | MacBook with Apple Silicon M3, macOS Darwin 24.6.0 |
| Primary AI tool | Claude Code (CLI), but intentionally multi-AI — uses Copilot, Gemini, Codex as well |

This is an **honors-level student** working on a portfolio project. They have a working understanding of C++ and data structures (they designed the system themselves and directed implementation decisions), but they rely on AI agents heavily for implementation velocity.

---

## 2. Communication Style

**Extremely concise.** Instructions are often a single short phrase:
- `"make it"` → create the file/feature we just discussed
- `"commit this"` → stage and commit the most recent change
- `"push it"` → push to remote
- `"just answer. don't do anything"` → discussion only, no code changes

**What this means for you:**
- Do not ask clarifying questions unless something is genuinely ambiguous — default to the most reasonable interpretation and act.
- Match their energy. Short responses are preferred over long explanations.
- They read the diff themselves. Don't recap what you just did unless it's a non-obvious decision.
- When they say "just answer" or "don't do anything" — honor that strictly. No tool calls, no file changes.

---

## 3. Working Preferences

### They are the architect — you are the implementer
The user makes the big design calls. They decided open-addressing over separate chaining, raw arrays over `std::vector`, and the overall panel layout. Your job is to execute cleanly within those decisions, not to second-guess them.

If something in the existing code looks like a "mistake" (e.g., the static category registry in BudgetManager, the hardcoded payment date in the Bills panel), it is intentional or a known trade-off. Note it, don't silently "fix" it.

### Multi-AI workflow is intentional
The user explicitly wants to be able to hand off to any AI tool and continue smoothly. This is why `AI_HANDOFF.md` and this file exist. When you're working on this project, write code and documentation that is tool-agnostic — no AI-specific syntax, no tool-specific formatting.

### Git hygiene matters
- Always commit with a clear, conventional commit message (`feat:`, `fix:`, `docs:`, `test:`, etc.)
- Only stage the files relevant to the change. Do not `git add -A` blindly.
- Push only when explicitly asked — `"push it"` or similar.
- Never amend published commits.

### Testing is expected
Before calling any feature complete, `make test` should pass. The user knows this and will notice if tests break.

---

## 4. What They Care About

1. **The project working correctly** — tests pass, app builds, UI behaves as expected.
2. **Continuity across sessions and tools** — hence the handoff documents.
3. **Clean git history** — meaningful commits, nothing extraneous staged.
4. **Growing the project comfortably** — the codebase should stay extensible. Don't paint into corners.
5. **The academic artifact** — `reflection.md`, `diagram.mmd`, `README.md` are submitted work. Don't modify them without being asked.

---

## 5. What They Don't Want

- Long explanations of things they can read in the diff
- Unsolicited refactors or "while I'm here" cleanups
- New abstractions or helper classes that weren't asked for
- Comments in code that explain *what* the code does (they prefer self-documenting names)
- Emojis in responses or files (they've never asked for them)
- Being asked to confirm obvious next steps — just do them

---

## 6. Future Goals for This Project

The user wants to expand the app with AI features. Their stated direction (from prior sessions):

- **Priority 1:** Finance Chat Assistant + Natural Language Expense Entry (powered by AWS Bedrock)
- **Priority 2:** Spending Insights, Category Auto-Suggest, Monthly Summary
- **Longer term:** Budget Recommendation Engine, Spending Forecast, Bill Priority Advisor

The implementation path requires `libcurl` (AWS SigV4), `nlohmann/json`, and new ImGui panels. See `AI_HANDOFF.md` Section 9.3 for the full feature list and implementation notes.

They also mentioned interest in the optional extensions from the project spec (file persistence, recurring bills, CSV export, spending chart). These are lower priority than the AI features.

---

## 7. How Sessions Typically Go

1. User gives a short instruction or question
2. Agent acts or answers directly
3. User says `"commit this"` → commit only the relevant file(s)
4. User says `"push it"` → push to origin/main
5. Move to the next thing

Sessions are task-driven, not exploratory. The user knows what they want — help them get there fast.

---

## 8. Context a New Agent Should Verify Before Starting

- [ ] Run `make test` — all 6 tests should pass
- [ ] Run `make && ./finance_app` — window should open with 5 panels
- [ ] Check `git log --oneline -5` to see what was last changed
- [ ] Check `git status` to see if there's any uncommitted work in progress

---

*Created 2026-05-04. Update this file if the user's preferences, goals, or workflow change meaningfully.*
