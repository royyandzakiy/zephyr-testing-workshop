# Self-hosted runner

For someone putting this repo's tests on their own machine or on a board, so CI can
flash real hardware. The workflows that use it are in
[`../../.github/workflows/`](../../.github/workflows/).

## Registering the runner

Activate runners by opening the settings for self-hosted runner

```bash
https://github.com/YOUR_USERNAME/zephyr-testing-workshop/settings/actions/runners/new?arch=x64&os=linux
```

**Inside the devcontainer you can skip the download.** The runner is already unpacked at
`/actions-runner` by the published devel image ([zephyr-devcontainer](https://github.com/royyandzakiy/zephyr-devcontainer)), and its state lives in a
per-project volume (`${localWorkspaceFolderBasename}-actions-runner`) so a registration survives
container rebuilds. That volume is deliberately *not* shared between projects the way the SDK
volumes are - it holds one runner registration, so a shared name would make two projects fight
over it. Jump to "Run the actions-runner" below.

Two caveats on the baked copy: Docker seeds a named volume from the image only when the volume
is **empty**, so bumping the runner version in the image will not update an existing
volume - delete it first. And the version bundled in the image is the one pinned there, not
necessarily the newest.

The rest of this section is for setting a runner up on a **bare host**, outside the container.

Download the actions runner for linux to `/`

```bash
# Create a folder
cd /
mkdir actions-runner && cd actions-runner

# Download the latest runner package
curl -o actions-runner-linux-x64-2.336.0.tar.gz -L https://github.com/actions/runner/releases/download/v2.336.0/actions-runner-linux-x64-2.336.0.tar.gz

# Optional: Validate the hash
echo "04cf0be1aff4c3ec3554466c39124ca250e3effd8873bb7e8d68535aa9505d5d  actions-runner-linux-x64-2.336.0.tar.gz" | shasum -a 256 -c

# Extract the installer
tar xzf ./actions-runner-linux-x64-2.336.0.tar.gz
```

Run the actions-runner NOT as root (here we create a user called runner)

```bash
cd /actions-runner

# config if not yet
./config.sh --url https://github.com/YOUR_USERNAME/zephyr-testing-workshop --token YOUR_TOKEN_HERE --ephemeral

./run.sh
```

### Troubleshooting

Error: Cannot configure the runner because it is already configured. To reconfigure the runner, run 'config.cmd remove' or './config.sh remove' first.

```bash
./config.sh remove --token YOUR_TOKEN_HERE
```

Error: A runner already exists

```bash
# √ Connected to GitHub
# A session for this runner already exists.
# 2026-08-13 11:57:51Z: Runner connect error: Error: Conflict. Retrying until reconnected.

# Find the running runner processes, then kill
ps aux | grep -i Runner.Listener
pkill -9 -f Runner.Listener || true

# or, wait 1-3 mins until github actions kills it online
# or, replace with a new runner
./config.sh --url https://github.com/YOUR_USERNAME/YOUR_REPO --token YOUR_NEW_RUNNER_TOKEN --replace

./run.sh
```

Error: Not Found

```bash
# Http response code: NotFound from 'POST https://api.github.com/actions/runner-registration' (Request Id: 14ED:2450BC:5DE3A:66F10:6A7E54AF)
# {"message":"Not Found","documentation_url":"https://docs.github.com/rest","status":"404"}
# Response status code does not indicate success: 404 (Not Found).
```

The registration token is wrong or has expired. They are short-lived. Generate a fresh
one from the repository's Settings, Actions, Runners, New self-hosted runner page, and
check the `--url` points at a repository you can administer.

## Checking the runner can flash

Before wiring it into a workflow, confirm the runner's own shell can build and flash:

```bash
west build -b nrf5340dk/nrf5340/cpuapp -d build_nrf -p always
```

```bash
west flash -d build_nrf --runner nrfutil --dev-id 1050073602
```

Full per-board commands are in
[`../reference/boards.md`](../reference/boards.md#per-platform).