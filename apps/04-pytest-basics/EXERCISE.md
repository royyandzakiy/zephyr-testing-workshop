# 04-pytest-basics - exercises

## ★ warm-up

1. **Make the first test fail on purpose.** In `1_bareminimum/test_bareminimum.py`,
   uncomment the `assert 1 + 1 == 3` line, then run the folder again:

   ```bash
   cd apps/04-pytest-basics/1_bareminimum && pytest -v
   ```

   *Check:* the failure report shows the assert with both values filled in, and the
   summary line reads `1 failed, 1 passed`. Put the line back when you are done.

2. **Predict the test count before you run it.** In `2_fixture/test_fixture.py`, add
   `(3, 4, 7)` to the list in `test_add`'s `parametrize`, and add `2` to the `x` list
   of `test_multiply`. Write down how many tests you expect, then collect them:

   ```bash
   cd apps/04-pytest-basics/2_fixture && pytest --collect-only -q
   ```

   *Check:* the count matches your prediction, and you can say why the extra `x` value
   added two tests while the extra `test_add` case added one. Put the lines back when
   you are done.

3. **Run 4_marking three ways and compare.** Run it with the slow test only, without
   it, and with everything:

   ```bash
   cd apps/04-pytest-basics/4_marking && pytest -v -m custom_slow
   ```

   ```bash
   pytest -v -m "not custom_slow"
   ```

   ```bash
   pytest -v
   ```

   *Check:* you can read the `deselected` count in each summary line, and say which run
   took about 5 seconds and why.

## ★★ go deeper

1. **Break the marker registration.** Delete the `markers =` lines from
   `4_marking/pytest.ini`, then run with strict markers:

   ```bash
   cd apps/04-pytest-basics/4_marking && pytest -v --strict-markers
   ```

   *Check:* pytest stops with an error naming `custom_slow` before running any test, and
   without `--strict-markers` the same file only gives a `PytestUnknownMarkWarning`. Put
   the lines back when you are done.

2. **Move 3_foldering_conftest to the config approach.** Delete
   `3_foldering_conftest/tests/conftest.py` and run, so you see the import fail:

   ```bash
   cd apps/04-pytest-basics/3_foldering_conftest && pytest -v
   ```

   Then add a `pyproject.toml` in that folder with the same two lines that
   `3_foldering_project/pyproject.toml` has, and run again.

   *Check:* the first run fails collection with `ModuleNotFoundError: No module named
   'calc'`, and the second run passes with no `conftest.py`. Put the file back and remove
   the `pyproject.toml` when you are done.

3. **Find out what the xfail is hiding.** `test_xfail` is marked as a known bug in
   `wrong_add`. Run it with `--runxfail`, which ignores the marker and reports the real
   result:

   ```bash
   cd apps/04-pytest-basics/4_marking && pytest -v --runxfail -m "not custom_slow"
   ```

   *Check:* the failure is a `NameError` for `wrong_add`, not the wrong sum the reason
   string describes, and you can name the import line in `tests/test_calc.py` that is
   missing it. `xfail` accepts any failure unless you pass it `raises=`.

## ★★★ off the map

1. **Share one fixture between two test files.** Add a second test file to `2_fixture`
   that uses the `numbers` fixture, then move the fixture somewhere both files can see
   it.

   *Why it is interesting:* pytest finds fixtures in `conftest.py` files by directory,
   with no import. Plugins add fixtures the same way: the `shell` and `dut` fixtures in
   app 05 come from the `twister_harness` plugin, which Twister loads with
   `-p twister_harness.plugin`, and your test never imports them.

2. **Decide where the slow line should be drawn.** `custom_slow` is one marker. Think
   about a suite with tests that take 10 ms, 1 s, 30 s and 5 minutes, and how you would
   mark them so a pre-commit run and a nightly run each pick the right set.

   *Why it is interesting:* there is no single right answer, and the same choice comes
   up for the Twister scenarios in app 05.

## If you want to go further

- [How to use fixtures](https://docs.pytest.org/en/stable/how-to/fixtures.html):
  scopes, `yield` teardown and `conftest.py`, which come up as soon as a fixture starts a
  process or opens a port.
- [How to mark test functions](https://docs.pytest.org/en/stable/how-to/mark.html):
  registering markers, `--strict-markers`, and combining `-m` expressions with `and`,
  `or` and `not`.
- [pytest import mechanisms](https://docs.pytest.org/en/stable/explanation/pythonpath.html):
  how `rootdir`, `sys.path` and the `pythonpath` setting decide what a test can import.
