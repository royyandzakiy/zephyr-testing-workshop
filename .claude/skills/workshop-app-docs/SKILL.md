---
name: workshop-app-docs
description: Write or refresh the README.md and EXERCISE.md that sit inside a workshop app folder under apps/. Use this whenever a new app folder is added, an existing app's code changes enough that its docs are stale, or the user asks for a README, exercises, extra practice, or "something for people who finish early" for a folder in this repo. Also use it to check existing app docs against the house format.
---

# Workshop app docs

Every folder under `apps/` carries two files that are not code:

- `README.md` - what this app is, what changed since the previous one, what to
  learn from it, how to run it, and how you know it worked.
- `EXERCISE.md` - what to do next if you finished early, graded, self-serve,
  answers deliberately not in this repo.

They serve different people at the same moment. The README serves someone who
is lost. EXERCISE.md serves someone who is bored. Do not merge them and do not
let one drift into the other.

## Before writing anything

Read the app first. You cannot write either file from the folder name.

1. `src/` and `tests/` in full. The README's "what to learn" comes from what
   the code actually does, not from what the topic is called.
2. The previous app in the sequence, so "what changed" is a real diff and not
   a guess. `diff -r apps/0N-x apps/0M-y` is usually faster than reading both.
3. The root `README.md` for the app table and the session mapping.
4. Any `NOTES.md` in the folder. It often holds the real board commands and
   known findings, and those belong in the README rather than in a second
   file nobody opens.

If a claim in either document cannot be checked against the tree, cut it.

## Voice

Follow the repo's existing voice, which is the same one the root `README.md`
uses. Load the `royyan-voice` skill if it is available in the session.

Hard rules, because they are the ones that get broken:

- **No em dashes.** Not `-` doubled, not the unicode one. Use a comma, a
  spaced hyphen, parentheses, or two sentences.
- No triadic repetition for effect. "Same source. Same assertions. Same
  pytest." is exactly the shape to avoid.
- No colon-then-reveal, no "not X, it's Y", no dramatic framing of ordinary
  engineering.
- Second person for instructions, first person only where the repo already
  uses it.
- State the constraint, state what was done, move on.

## README.md

Target 60 to 120 lines. Longer than that and nobody reads it during a
workshop.

Sections, in this order. Skip one only when the app genuinely has nothing to
put in it.

```markdown
# NN-name

<One or two sentences. What this app is, and the single idea it exists to
carry. No preamble.>

**What changed since `MM-previous`:** <one sentence, or a three-line bullet
list if it is structural. If the app is the first of a new thread, say what
thread it opens instead.>

## What to learn here

<Three to six bullets. Each one is a thing the reader should be able to do or
explain afterwards, not a topic name. "Why a test-only overlay has to move
both aliases" beats "devicetree overlays".>

## Layout

<A fenced tree of the folder, annotated. Only the files that matter. Leave out
boards/ contents unless the app is about board portability.>

## Run it

<Fenced bash blocks, one command per block so the desktop app can put a Run
button on each. Build, run, test, in that order. Include the real board
command if the app has one.>

## Expected outcome

<What you should see. Paste the actual output, trimmed. For test suites, the
scenario names and the pass count. This is the section people scroll to when
something is wrong, so it has to be literal rather than described.>

## References

<Links with a reason attached. "zephyr.org/.../ztest.html - the ZTEST_F
fixture naming rule is halfway down" beats a bare URL. Prefer upstream Zephyr
and framework docs over anything in this repo.>
```

Two things that make a README earn its place, both easy to skip:

- **The expected outcome is literal.** Copy real output from a real run. If
  you cannot run it, say so in one line rather than inventing plausible
  output.
- **"What changed" is honest about zero.** If nothing in `src/` changed and
  the whole lesson is a new directory, that sentence is the lesson. Say it.

## EXERCISE.md

Target 40 to 90 lines. Every exercise is doable with only this repo, the
Zephyr docs and a laptop. Nothing requires hardware unless the app is
explicitly about hardware, and then it is marked.

```markdown
# NN-name - exercises

<One sentence. Who this is for: you finished the guided part and the room has
not caught up yet.>

The answers are in the upstream docs rather than in this repo. Finding them is
the exercise.

## ★ warm-up

<Two to four. Ten minutes each. A parameter to change, a value to predict
before running, one assertion to add. Every one has a check the reader can
apply themselves.>

## ★★ go deeper

<Two to four. Twenty to forty minutes. Add a real test, break something on
purpose and read the failure, port the pattern to a second board or a second
module.>

## ★★★ off the map

<One to three. Open-ended, possibly unfinishable in the session. These are
allowed to have no single right answer. Something that touches a real Zephyr
subsystem, a sanitizer finding, an upstream driver, or a genuine trade-off
with no clean answer.>

## If you want to go further

<Two to four links, with a sentence each on what is actually in them.>
```

Rules for the exercises themselves:

- **Each one names its own success check.** "Add a test for X" is not an
  exercise. "Add a test for X that fails before you fix Y and passes after" is.
- **Stars mean time and open-endedness, not difficulty of syntax.** A ★★★ is
  not a harder API call, it is a question with more than one defensible
  answer.
- **At least one exercise per app should involve making something fail.**
  Reading a real failure message is the skill, and a workshop where everything
  stays green teaches people to trust green.
- Do not restate what the README already covered. If an exercise needs three
  paragraphs of setup, that setup belonged in the README.
- No answer keys, no "solution" section, no hint that gives it away. A pointer
  to the right page of the Zephyr docs is fine and is usually the right amount.

## Cross-app consistency

When you add or change docs for one app, check these:

- The root `README.md` app table has a row for it, with the right session.
- The next app's README says what changed since this one, if this one moved.
- Star ratings mean the same thing across folders. Skim two neighbours before
  settling on a grade.
- Every app has both files. A folder with a README and no EXERCISE.md is the
  one the fast people get stuck on.

## Checklist before you call it done

1. Search for the em dash character and for a doubled hyphen used as one.
   Remove them.
2. Every command in "Run it" was actually run, or is flagged as unverified.
3. Every file named in "Layout" exists. Every file that matters is named.
4. Every link resolves and every link has a reason next to it.
5. The exercises reference things that exist in this app's tree.
6. Read the README out loud. If a sentence sounds like a product page, rewrite
   it as the plainer thing it is trying to say.
