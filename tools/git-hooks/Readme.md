# Git Hooks for iceoryx

When developing on iceoryx, git-hooks are doing some sanity checks and can execute linters/formatter when you commit code to iceoryx.
In the current state the git hooks do the following job:

- checking if commit messages follows the format `[iox-#<Github issue number>]` and auto-format them if necessary
- check if commit message subject and body follow six of the [seven rules of a great commit message](https://chris.beams.io/posts/git-commit/)
- remove trailing whitespace from all modified files
- run `clang-format` on all modified files

For running `clang-format` it checks if the version 10 is used, otherwise it will print a warning,
you can install it by using the following command:

```bash
sudo apt install clang-format-10
```

## Installation

The hooks are active when you copy them into the `.git/hooks` folder of your iceoryx project:

```bash
cd iceoryx
cp -rv ./tools/git-hooks/pre* .git/hooks/
```

We recommend to do this in every new clone you did on iceoryx.

## Open topics

- clang-tidy check
- check and clean up line endings in files
- ...
