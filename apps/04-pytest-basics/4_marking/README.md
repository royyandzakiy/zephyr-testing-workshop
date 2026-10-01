```bash
pytest -v                       # everything
pytest -v -m custom_slow        # only slow tests
pytest -v -m "not custom_slow"  # skip slow
pytest -v -m "not custom_slow and not xfail"
```