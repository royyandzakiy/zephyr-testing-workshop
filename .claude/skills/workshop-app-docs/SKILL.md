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

### The governing principle

> "the nuance is more neutral, not trying to make anyone surprised (which ends up
> being cringe)"

Write to inform, never to land a point. A sentence built so the reader goes "oh!" is
the thing to remove, even when what it says is true.

Five habits that break it, with the actual line Royyan rewrote in each case:

| Habit | Written | Rewritten |
|---|---|---|
| **Novelty claim.** Saying a thing is the first, the only, or a milestone. | "The test process and the device are two separate programs here, which is the first time that is true in this repo." | "The test process and the device are two separate programs here." |
| **Trailing justification.** A clause after the fact explaining why the fact is good. | "The raw suite skips itself with a message naming the missing binary if you have not built the image, which is what you want from a test that depends on something outside its control." | "The raw suite will give a message naming the missing binary if you have not built the image yet." |
| **Two-part setup.** "What X gives you, and why Y" invites a reveal. One idea, stated flat. | "What the `shell` fixture gives you, and why the same test file works whether the device is a native_sim process or a board on a runner." | "Using a `shell` fixture that lets the same test file run whether the device is a native_sim process or a board on a runner." |
| **Vague emphasis and colour jargon.** "almost every time", "far more often", "goes red". | "When it goes red, that log is the answer almost every time." | "When the test fails, you can find out what the exact failure was by looking there." |
| **"X rather than Y".** Contrast framing where a plain statement works. | "One test reports as **xfail** rather than as a pass." | "One test fails and reports as **xfail**." |

Related patterns to strike on sight, all of which are the same instinct: "that is the
whole trick", "which is the point of having it there", "is the whole value of", "does
not survive contact with", "the naming rule that bites", "stays green while the
product does not work", "walks straight past it".

**Link descriptions say what is in the page, not how valuable it is.** "worth a skim",
"the three that pay off fastest", "worth reading properly", "the page to keep open"
are all value judgements the reader did not ask for.

| Written | Rewritten |
|---|---|
| "the whole index is worth a skim; `conftest.py`, marks and fixtures are the three that pay off fastest here" | "you can find about `conftest.py`, marks and fixtures in more detail" |
| "the page to keep open while writing" | "the one-page summary of matchers, actions and cardinalities" |
| "the vendored copy, worth opening once to see what the macros expand to" | "the vendored copy. Open it to see what the macros expand to." |

"Worth knowing by name" survives in `00-hello`, so the test is not the word "worth",
it is whether you are rating the page or describing it.

### Keep

Second person, practical, slightly loose grammar, "just" as a softener. His own
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

**Every app README has the same sections in the same order, including Trivia, and
Trivia always contains at least one mermaid diagram.** `00-hello` is the reference for
all of it.

### The opener

Three plain sentences, in this order. Full sentences, never stacked fragments.

1. **What this project is.** Starts "This project is...".
2. **What the reader will do.** "You will build it and run the executable."
3. **What it does not do**, pre-empting the obvious confusion. "It does not produce
   any firmware to flash."

Royyan's own:

> This project is just to test and ensure your toolchain works. You will build it and
> run the executable. It does not produce any firmware to flash.

Openers he rejected, and the shape to avoid:

| Rejected | What is wrong with it |
|---|---|
| "One flat `main.c`, no seam, no tests. This is roughly the code you would inherit on a real project." | stacked fragments, then a knowing aside. *"i really hate this style of talking"* |
| "The odd one out. No Zephyr, no devicetree, no board, no twister." | same, opens on a fragment |
| "Everything `04-shell-pytest` did, plus the pytest features that..." | noun phrase, never says what you will do |

A point like "this is the untestable starting point" is real and belongs in a **What
to learn here** bullet, not smuggled into the opener as an aside.

### The sections

```markdown
# NN-name

<The three-sentence opener above.>

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

<REQUIRED. Two or three `###` subsections of background the app itself cannot show
you, and at least one brief mermaid diagram. This is where "briefly explain X" notes
go, so "What to learn here" can stay a list of one-liners that point here.>

## References

<Table. Links with a reason attached. "zephyr.org/.../ztest.html - the ZTEST_F fixture
naming rule is halfway down" beats a bare URL. Prefer upstream docs over this repo.>
```

### Trivia, in more detail

Each app's Trivia answers "what would I have to already know for the code in this
folder to make sense?" Three shapes that keep working:

- **A table of the things that are easy to mix up.** `00-hello` does west / Kconfig /
  devicetree / SDK. `01-blinky` does `.dts` / `.dtsi` / `.overlay` / binding / alias.
- **A diagram of where the piece under discussion sits.** Usually a stack, a build
  graph, or a lifecycle. Keep it brief; eight to twelve nodes is plenty.
- **What you get and what you do not get**, as two short lists, when the app is
  introducing a technique with real limits.

Name generated files by their full path when you mention them. `.config`,
`autoconf.h`, `devicetree_generated.h`, `twister-out/handler.log`. Being able to find
them is half of what the reader is here for.

**Validate every diagram before committing.** Extract the fenced blocks and run them
through the real parser, do not eyeball them:

```bash
npm install --prefix <scratch> mermaid@11 jsdom && node <scratch>/check.mjs
```

`mermaid.parse()` needs a DOM, so jsdom globals have to be installed first. The
diagrams currently in the repo all use `flowchart TD` with quoted labels, `<br/>` for
line breaks, `subgraph`, dotted `-.->` and labelled `-->|like this|` edges, all of
which parse cleanly on mermaid 11.

## EXERCISE.md

No preamble of any kind. The file opens on `## ★ warm-up`, including setup that every
exercise needs, which goes inside the first exercise instead.

### The shape of one exercise

00-hello's are the model. Four parts, in this order:

1. **A bold title that is a full imperative sentence**, ending in a period inside the
   bold. "**Make the build fail on purpose.**"
2. **Plain prose** giving the context and saying what to look for.
3. **The command, in a fenced `bash` block.** Never inline in the prose, never
   described as "run twister". If the exercise needs two steps, use two fences with a
   sentence between them, the way `00-hello` warm-up 2 walks from `.config` to
   `grep -rn "select PRINTK"`.
4. **A blank line, then `*Check:*`** as its own paragraph. `★★★` uses
   `*Why it is interesting:*` instead.

Roughly three quarters of the exercises in a file should carry a fenced command.
`00-hello` has 8 across 10. A file with 13 exercises and no fences is the failure mode
to watch for: it means the exercises are being described rather than handed over.

Commands are repo-relative and must actually run. Test the ones that do not need the
devcontainer before committing.

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
3. Run the neutral-nuance grep. Every hit is a sentence written to land a point:

   ```bash
   grep -nEi 'goes red|go red|green run|stays green|is the answer|almost every time|far more often|which is the point|that is the whole|which is what you want|rather than as a|pay off|worth a skim|survive contact|walks straight past|that bites|the first (time|app|suite) in this repo' apps/*/README.md apps/*/EXERCISE.md
   ```
4. The opener is three full sentences: what it is, what you will do, what it does not
   do. No stacked fragments, no knowing asides.
5. The README has a `## Trivia` section, and that section has a mermaid diagram that
   you parsed rather than eyeballed.
6. `EXERCISE.md` opens on `## ★ warm-up` with nothing above it, and most exercises
   carry a fenced `bash` command rather than describing one. Run the ones that do not
   need the devcontainer.
7. Every `*Check:*` names something concrete the reader can do or say.
8. Every command in "Run it" was actually run, or is flagged as unverified.
9. Every file named in "Layout" exists. Every file that matters is named.
10. Every link resolves and has a reason next to it.
11. Read it back. If a sentence sounds like a product page or like a narrator, rewrite
   it as the plainer thing it is trying to say.
