Software testing can be categorized in several ways. Here's a breakdown of the main types:

## By Level (Granularity)

**Unit Testing**
Tests individual functions, methods, or components in isolation. Usually written by developers. Fast to run, cheap to maintain.

**Integration Testing**
Tests how multiple units/modules work together. Catches issues in interfaces, data flow, and communication between components.

**System Testing**
Tests the complete, integrated application as a whole against requirements.

**Acceptance Testing**
Validates the software meets business needs. Often done by end users or QA.
- *UAT (User Acceptance Testing)* – real users verify it works for their workflow
- *Alpha/Beta testing* – internal then external pre-release testing

## By Purpose

**Functional Testing**
Verifies features work per requirements (input → expected output).

**Non-Functional Testing**
Verifies quality attributes:
- **Performance** – speed, responsiveness, throughput
- **Load** – behavior under expected load
- **Stress** – behavior beyond normal capacity
- **Scalability** – ability to grow
- **Usability** – ease of use
- **Accessibility** – usable by people with disabilities
- **Security** – resistance to attacks
- **Reliability** – consistent behavior over time
- **Compatibility** – works across browsers/OS/devices

## By Approach

**Black Box** – tests behavior without knowledge of internals
**White Box** – tests internal logic/structure (code coverage, paths)
**Gray Box** – partial knowledge of internals

**Manual Testing** – human executes tests
**Automated Testing** – scripts/tools run tests

## By Timing / Strategy

**Smoke Testing** – quick check that core features work before deeper testing

**Sanity Testing** – narrow check that a fix/small change works

**Regression Testing** – confirms new changes didn't break existing features

**Exploratory Testing** – simultaneous learning, test design, and execution

**Ad-hoc Testing** – informal, no planning or documentation

**End-to-End (E2E) Testing** – full user workflow across the whole system

**A/B Testing** – compares two versions to see which performs better

## Common Testing Pyramid

```
      /\
     /E2E\        few, slow, expensive
    /------\
   /Integr. \     moderate
  /----------\
 /    Unit    \   many, fast, cheap
/--------------\
```

## Quick Comparison

| Type | Scope | Who | Speed |
|------|-------|-----|-------|
| Unit | Function | Dev | Fast |
| Integration | Modules | Dev/QA | Medium |
| System | Whole app | QA | Slow |
| Acceptance | Business | User/QA | Slow |
| Regression | Existing features | QA/Auto | Varies |
| Performance | Load/speed | QA/Perf | Slow |

Want me to go deeper on any specific type, or cover testing tools/frameworks for each?