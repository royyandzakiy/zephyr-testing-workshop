> Working notes. The Programme Information as it was published, kept for reference.
> The repo has moved on since, so where the two disagree the repo is current.
> [`workshop-runsheet.md`](workshop-runsheet.md) is the plan for the day.

# AI-Assisted Embedded Testing with Zephyr: From Simulation to Hardware & CI

*Build one test suite that runs on your host machine and on your board, with peripheral
emulation, CI, and agent skills that help you write and debug it faster.*

## Overview

Most firmware work still runs on the same loop: flash, watch an LED, add a printk, flash
again. It works fine for a while. It gets expensive once the codebase grows, the board is
stuck in customs, three people are touching the same driver, and the only way to know
whether today's change broke last month's feature is to plug in a board and try it.

Zephyr already ships most of what you need to get out of that loop. Host-based simulation
runs your application logic as a native binary on your laptop. The emulator subsystem
stands in for GPIO, I2C and SPI at the devicetree level. Twister runs your test cases
across boards and configurations. pytest drives a running device end to end over the
shell. All of it is in the tree and all of it is documented. What the documentation does
not tell you is where to start when the code in front of you was never written with any
of this in mind.

That is what this hands-on workshop is about, and it rests on one idea: the seam you need
is usually already at the devicetree, not somewhere inside your application code.

We start with a small and entirely ordinary application, a button interrupt that toggles
an LED, bound to sw0 and led0, written the way you would normally write it. I run its end
to end test on a physical board, then run the same test file on host-based simulation.
Nothing in the test changes. The only difference is which devicetree overlay Twister
picked. Once you have seen that, moving between host and target stops being a rewrite and
becomes an argument you pass on the command line.

From there we build that one seam up into a working pipeline: ztest for the fast loop,
GPIO emulation as your stimulus, test-only overlays, Twister as the canonical runner,
pytest for end to end flows, and GitHub Actions running all of it on every push to your
own fork.

One thing has changed since most teams last looked at their test strategy. There is now
an AI assistant sitting in the editor, and it is good at producing test code, mostly the
boilerplate parts. I cover that once, in a single block near the end, as two agent skills
built for this repository. One scaffolds a test suite following the project's own
conventions. The other drives the build, run and test loop and reads back what actually
failed, so a red build turns into a specific cause, like a CONFIG_ symbol that was never
enabled or an alias that does not resolve on the board you selected. This is not an agent
running unattended. It is meant to be precise and informative and to hand the decision
back to you. I demonstrate it with Claude, though the pattern is not specific to any one
assistant, and the skill files ship in the repository so you can rewrite them for your own
project.

You leave with a working repository, a devcontainer that behaves the same across operating
systems, and a clearer sense of where to cut existing firmware so it becomes testable at
all. The focus is the testing harness itself. You get the principles and patterns that
make a codebase testable, and a concrete template to apply to your own projects, including
the legacy ones.

## By the End You'll Walk Away With

- A working test suite that runs unchanged on both host-based simulation and physical
  hardware
- A reusable devcontainer and project template that pins your SDK version and reproduces
  the same way on different machines
- Hands-on experience with Zephyr's GPIO emulation, driving inputs and observing outputs
  deterministically
- Test-only devicetree overlays that keep your test scaffolding separate from your
  application code
- A configured CI pipeline (GitHub Actions) that builds and runs your tests on every push,
  including on a self-hosted runner
- End to end pytest tests that drive a running Zephyr instance over the shell
- A clear strategy for introducing automated testing into an existing, untested codebase
  incrementally
- The agent skills used in the session, test scaffolding and a build/run/test loop that
  reports precisely what failed, in the repository as a starting point to adapt to your
  own conventions

## Summary

By the end of this workshop you will not just know that Zephyr has a complete testing
environment. You will have run it end to end on your own machine. You will have driven an
interrupt handler from an emulated GPIO, written test-only overlays that keep your test
scaffolding out of your application code, driven a running device from pytest over the
shell, and watched the whole suite go green in CI on your own fork. Every file, script,
container definition and pipeline used in the session is yours to keep and drop into your
own project on Monday.

## Who Should Attend

- Embedded and firmware engineers with hands-on Zephyr or nRF Connect SDK experience who
  have never set up automated testing for it
- Mid to senior firmware developers inheriting or maintaining an existing embedded
  codebase with no testing strategy
- Embedded engineers who write tests for host applications but assume it is impossible on
  firmware
- Firmware tech leads and architects responsible for build reproducibility, code quality
  gates, or CI/CD for embedded teams
- Engineers working under regulatory or safety pressure who need verification evidence,
  not just a working demo

## What You'll Learn

- ✅ How to run your application logic on a host-based simulator and get a first ztest
  suite passing in seconds instead of a flash cycle
- ✅ How to find the point where firmware is bound to its board, and why cutting at the
  devicetree is usually cheaper than refactoring the code
- ✅ How to use Zephyr's emulator subsystem to drive inputs and assert outputs
  deterministically, instead of hand-rolling fakes
- ✅ How to write test-only devicetree overlays and scope them per suite, so your test
  scaffolding never lands in your application source folders
- ✅ How to design tests that earn their keep: AAA structure, test sizing, and choosing
  what to test first
- ✅ How to use Twister as your single canonical runner for filtering, platform matrices,
  artifacts, and coverage
- ✅ How to write end to end blackbox tests in pytest that interact with a running device
  through the shell
- ✅ How to wire a CI pipeline that builds, runs tests, and uploads artifacts, including on
  a self-hosted runner
- ✅ How to package your team's testing conventions into an agent skill that scaffolds
  suites, drives the build and test loop, and turns a red build into a specific cause
  instead of a wall of output

## What You'll Need

- A PC or laptop with at least 8 GB RAM and around 10 GB free disk space
- Docker Desktop or Docker Engine installed and running
- Visual Studio Code with the Dev Containers extension installed
- Git, and a GitHub account (free tier is fine, needed for the CI section)
- Prior experience building and flashing at least one Zephyr or nRF Connect SDK
  application (blinky level or above)
- Familiarity with devicetree concepts (overlays, aliases, bindings) is strongly
  recommended
- Pre-workshop setup (required, around 30 minutes): you receive the repository and a short
  setup guide before the session. You open the project in the devcontainer, build the
  host-based sample, and run the provided test suite once to check everything works

## ✅ Format & Agenda

Live Online workshop, 1 Day (5 hours including breaks)

**First 30 Minutes:** Open Networking & Environment Setup

### Part 1: Proof, Then the Fast Loop

- **The demo (20 min).** A physical board: press the button, the LED toggles, the
  interrupt handler logs to the shell. Then the end to end pytest suite run against the
  board via Twister and a hardware map. Green. Then the same test file run on host-based
  simulation. Green. Two commands, one difference, which is the overlay.
- **Why firmware resists testing (15 min).** The flash-and-printk loop, the real cost of
  late defects, and where the test pyramid does and does not map onto embedded. Also what
  changes when an assistant can produce a hundred lines of test code in a second: the
  volume goes up, and verification becomes the only thing that scales with it.
- **Host-based simulation and ztest (30 min, hands-on).** Running your logic as a native
  binary, and a first passing test on the clock.
- **GPIO emulation as your stimulus (35 min, hands-on).** Driving an emulated button
  press, catching the interrupt, and asserting that the LED pin actually moved.
- **Test-only overlays (20 min, hands-on).** Giving a test suite its own devicetree and
  rerouting an alias onto an emulated controller, without a single change to your
  application source.

**Break (30 minutes)**

### Part 2: Scale It Up

- **Twister as the canonical runner (25 min).** Replacing ad-hoc invocations.
  testcase.yaml, platform_allow, platform and config matrices, filtering, artifacts,
  coverage output.
- **Designing tests worth keeping (10 min).** AAA structure, test size and scope, how much
  testing is enough, and what to test first in a codebase you inherited.
- **End to end testing with pytest (35 min, hands-on).** Registering a shell command as a
  test backdoor, driving it from the twister_harness Shell fixture, asserting on device
  output, and using fixtures and markers to keep it maintainable.
- **CI with GitHub Actions (20 min, hands-on).** Push to your own fork and watch build and
  Twister run on every commit, with test artifacts uploaded. Includes a self-hosted runner
  running host-based simulation.
- **On-target CI (5 min, demo).** The self-hosted runner and hardware map that ran the demo
  you saw earlier, walked through quickly.
- **Agent skills for the test loop (15 min, demo).** Two skills built for this repository.
  One scaffolds a ztest suite to the project's conventions. The other runs build, then
  run, then Twister, and reads the output back, naming the actual cause of a failure
  instead of dumping the log at you. We look at how they are written and not only at what
  they do, because the intended takeaway is that you rewrite them for your own project.
  Demonstrated with Claude. No API key, subscription or particular assistant is needed to
  attend, and everything the skills produce also ships hand-written in the repository.
- **Bringing this to a legacy project (10 min).** The incremental adoption path, and
  wrap-up Q&A.

Throughout the session there are optional exercises encouraging you to try things on your
own board or explore deeper API options. These are optional, and we will not spend
significant time debugging individual setups during the main session, but having a
physical board available will certainly enrich your exploration.

The repository also ships a set of self-serve bonus exercises, tiered by difficulty, for
anyone who finishes a block early or wants to keep going afterwards. Those cover ground the
session itself does not have time for, such as faking a dependency, testing against an
emulated I2C peripheral, and using the sensor emulators Zephyr already provides.

Follow along step by step. Each milestone is designed to keep participants on track, and
all final files are shared after the session.

## Why Now?

Zephyr has moved from interesting alternative to the default RTOS across major silicon
vendors, which means the ecosystem's tooling is now your tooling, whether you adopted it
deliberately or not.

At the same time, four pressures are converging on firmware teams: hardware availability
that no longer matches software schedules, regulatory and cybersecurity regimes that ask
for verification evidence rather than a working demo, codebases that have grown past the
point where one engineer can hold them in their head, and AI assistants now generating
firmware and test code faster than any team can review it by hand.

The tooling to handle all four already exists inside Zephyr, shipped and documented, and it
is dramatically underused. This workshop compresses the trial and error that adoption
normally costs into a single afternoon, with a working repository at the end of it.

## FAQ

**1. Do I need prior experience with Zephyr or RTOS development to attend this workshop?**
Yes. You should have built and flashed at least one Zephyr or nRF Connect SDK application
before. Blinky level is enough. We assume you know what west, a devicetree overlay and a
prj.conf are, and that you have some familiarity with devicetree concepts like aliases and
bindings. General RTOS experience without Zephyr specifically will leave you playing
catch-up. If you are unsure, run through the pre-workshop setup repository when you
receive it. If that builds and makes sense to you, you are ready.

**2. Will I need any specific hardware or development boards to follow along?**
No, and this is deliberate. Every hands-on segment runs inside the container on host-based
simulation and Zephyr's GPIO emulator. The on-target path is demonstrated live at the start
of the session so you can see it working, but nothing in the session depends on you owning
a board, and no exercise is gated on flashing one. That said, optional exercises throughout
the session will encourage you to try things on your own board if you have one.

**3. The title says AI-assisted. How much of this workshop is actually about AI?**
Around ten percent, in one block near the end. This is a Zephyr testing workshop first, and
the agenda reflects that: simulation, emulation, Twister, pytest and CI are the substance of
the day. The AI portion is a single worked example, two agent skills built for this
repository, shown so you can see how they are structured and take them apart. You need no
API key, subscription or particular tool to attend, and everything built during the session
is in the repository regardless.

**4. Is the AI part about automating testing end to end?**
No, and that is a deliberate choice. The skills are built to be informative and precise to
the developer, not to run unattended. The most useful one is diagnostic: when a build or a
Twister run goes red, it reads the output and tells you the specific cause, for example a
Kconfig symbol that is not enabled or a devicetree alias that does not exist on the board
you selected, and proposes a fix. You stay in the loop and make the call.

**5. Will the instructor share the code, templates, or project files used during the
workshop?**
Yes. You get the repository before the session, including the devcontainer, Dockerfile and
setup guide, and the complete final version afterwards: all test suites, overlays, pytest
harnesses, CI workflows, scripts, and the agent skills used in the session along with a few
smaller diagnostic ones. It is structured as a reusable project template, not a throwaway
demo.

**6. Will the project built in the workshop run on real hardware or only in simulation?**
Both, and demonstrating that is the first thing we do. The code you write is real Zephyr
application code, not a simulation-only toy. The same source builds and runs unchanged on
physical hardware. What changes is the devicetree overlay Twister selects. That is the
whole design argument of the day: your application stays portable enough to run on a host
for tests and on target for production.

**7. Will the sessions be recorded?**
Yes, recordings and project files will be provided to all registered participants.

**8. How interactive is this workshop?**
Highly interactive. Participants follow along live, with Q&A and real-time milestones
throughout. There are also optional exercises for those who want to explore deeper or try
things on their own hardware.

**9. Will I get a certificate of completion at the end of the workshop?**
Yes, you will receive an official certificate of completion recognizing your participation
in this workshop.
