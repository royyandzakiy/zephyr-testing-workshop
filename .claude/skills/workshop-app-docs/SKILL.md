---
name: workshop-app-docs
description: Write or refresh the README.md and EXERCISE.md that sit inside an app folder under apps/. Use this whenever a new app folder is added, an existing app's code changes enough that its docs are stale, or the user asks for a README, exercises, extra practice, or "something for people who finish early" for a folder in this repo. Also use it to check existing app docs against the house format.
---

# App docs

Every folder under `apps/` carries two files that are not code:

- `README.md` - what this app is, what to learn from it, how to run it, and how you
  know it worked.
- `EXERCISE.md` - what to do next once the guided part is done. Graded, self-serve,
  answers deliberately not in this repo.

Do not merge them and do not let one drift into the other.

**`apps/00-hello/README.md` and `apps/00-hello/EXERCISE.md` are the reference.**
Royyan rewrote that pair by hand to set the standard. When this file and those files
disagree, those files win. Read them before writing anything.

## The two rules that get broken most

### 1. No workshop or business context

These READMEs live in a project structure that people should be able to understand on
their own, with none of the surrounding commercial context. So none of this, anywhere
under `apps/`:

- "session 2", "the gate for session 4", "S3"
- "before the break", "the room is still on step 3", "everyone in the room"
- "all day", "on the clock", "during the workshop", "your attendees"
- Scene-setting openers of any kind. `EXERCISE.md` starts at `## ★ warm-up`.

Cross-references to sibling apps are fine and wanted: "app 01 onwards does", "compare
against `apps/02-ztest/tests/unit/testcase.yaml`", "how the pytest suite in app 04
talks to the device". Session mapping and the day's timetable belong in the root
`README.md` only.

### 2. Every check must be something the reader can literally do

A `*Check:*` line is rejected when it restates the task, or when the reader cannot
tell what is being asked. Two real rejections from `00-hello`:

| Rejected | Why |
|---|---|
| "you can name the generated file it comes from" | the `grep` in the task already printed the filename. Too trivial. |
| "you can say which other Kconfig symbol is selecting it" | *"i dont understand what this check wants me to do"* |

So: spell out the command, and say what the answer looks like. The fixed version of
the second one walks the reader through `grep CONFIG_PRINTK build/zephyr/.config`,
then `grep -rn "select PRINTK" $ZEPHYR_BASE`, and only then asks them to name the
symbol.

## Voice

Plainer than the general `royyan-voice` skill. These are read by people learning, not
by people being sold to. Explain the mechanism, not the significance.

Cut on sight:

| Cut | Actual example, removed by Royyan |
|---|---|
| Atmosphere, scene setting | "For when the build finished in twenty seconds and the room is still on step 3." |
| Meta-narrative about the arc | "**What this opens:** the whole sequence." / "the first small demonstration of the thing the whole day is about" |
| Clever reframings | "the same command with a different argument" |
| Punchy fragments for rhythm | "Three files, and two of them are nearly empty." |
| "not X, it's Y" | "That is correct, not a failure." became "will fail as expected" |
| Edgy asides | "without scrolling up", which drew *"i dont like this way of speaking. just make it clear without trying to be too edgy"* |
| Triadic repetition | "Same source. Same assertions. Same pytest." |
| Colon-then-reveal | "There is one thing that changes everything: X." |
| **Em dashes** | not the unicode one, not a doubled hyphen. Comma, spaced hyphen, parentheses, or two sentences. |

Keep: second person, practical, slightly loose grammar, "just" as a softener. His own
opening line is the model, so do not over-polish it away:

> This project is just to test and ensure your toolchain works. You will build it and
> run the executable. It does not produce any firmware to flash.

Say what the project is **for** and what the reader will **do**. Pre-empt the obvious
confusion ("it does not produce any firmware to flash").

## Before writing anything

Read the app first. You cannot write either file from the folder name.

1. `src/` and `tests/` in full. "What to learn here" comes from what the code does.
2. The previous app, so "what changed" is a real diff. `diff -r apps/0N-x apps/0M-y`
   is faster than reading both.
3. Any `NOTES.md` in the folder. It holds real board commands and known findings, and
   those belong in the README rather than in a second file nobody opens.
4. `apps/00-hello/README.md`, for the register.

If a claim cannot be checked against the tree, cut it.

## README.md

```markdown
# NN-name

<One or two sentences. What this app is for, and what the reader will do with it.>

**What changed since `MM-previous`:** <one sentence, or a short bullet list if it is
structural. Skip for the first app in a thread. Honest about zero: if nothing in
`src/` changed and the lesson is one new directory, that sentence is the lesson.>

## What to learn here

<Four to six bullets, in instructor-agenda voice rather than "you will be able to".
"Familiarizing with Kconfig: how `CONFIG_BOARD_TARGET` comes to be" is the shape.
Point at the Trivia section when a bullet needs real explaining.>

## Layout

<Fenced tree, annotated, only the files that matter. Leave out boards/ contents
unless the app is about board portability.>

## Run it

<Fenced bash blocks, ONE command per block so the desktop app puts a Run button on
each. Build, run, test, in that order. Include the real board command if there is one.>

## Expected outcome

<Literal output, trimmed, copied from a real run. For test suites, the scenario names
and the pass count. This is what people scroll to when something is wrong, so it
cannot be described, it has to be pasted. If you could not run it, say so in one line
rather than inventing plausible output.>

## Trivia

<Optional, and only where the reader genuinely needs background the app itself does
not supply. Short explainers, a table or a mermaid diagram where it helps. This is
where "briefly explain X" notes go, so that "What to learn here" stays a list.
Verify any mermaid actually renders before committing it.>

## References

<Table. Links with a reason attached. "zephyr.org/.../ztest.html - the ZTEST_F fixture
naming rule is halfway down" beats a bare URL. Prefer upstream docs over this repo.>
```

## EXERCISE.md

No preamble. The file opens on `## ★ warm-up`.

```markdown
# NN-name - exercises

## ★ warm-up

<Two to four, ten minutes each. A parameter to change, a value to predict before
running, one assertion to add. Give the command in a fenced block when there is one.>

## ★★ go deeper

<Two to four, twenty to forty minutes. Add a real test, break something on purpose and
read the failure, port the pattern to a second board or a second module.>

## ★★★ off the map

<One to three, open-ended, possibly unfinishable. Allowed to have no single right
answer. These use *Why it is interesting:* instead of *Check:*.>

## If you want to go further

<Two to four links, with a sentence each on what is actually in them.>
```

Rules for the exercises:

- **Stars mean time and open-endedness, not syntax difficulty.** A ★★★ is a question
  with more than one defensible answer, not a harder API call.
- **At least one per app makes something fail.** Reading a real failure message is the
  skill, and a set of docs where everything stays green teaches people to trust green.
- **Tell them to put it back.** Any exercise that edits a tracked file ends with "Put
  the line back when you are done."
- Do not restate the README. If an exercise needs three paragraphs of setup, that
  setup belonged in the README.
- No answer keys and no solution section. A pointer to the right page of the Zephyr
  docs is usually the right amount.

## Cross-app consistency

- The root `README.md` app table has a row for it.
- The next app's README says what changed since this one, if this one moved.
- Star ratings mean the same thing across folders. Skim two neighbours first.
- Every app has both files.

## Checklist before you call it done

1. `grep -nEi 'session|workshop|the room|all day|before the break|on the clock'` over
   both files. The only legitimate hit is literal program output.
2. Search for the em dash character and for a doubled hyphen used as one.
3. Every `*Check:*` names something concrete the reader can do or say.
4. Every command in "Run it" was actually run, or is flagged as unverified.
5. Every file named in "Layout" exists. Every file that matters is named.
6. Every link resolves and has a reason next to it.
7. Read it back. If a sentence sounds like a product page or like a narrator, rewrite
   it as the plainer thing it is trying to say.
