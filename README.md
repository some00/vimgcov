![Build Status](https://github.com/some00/vimgcov/actions/workflows/build_and_test.yml/badge.svg) ![License](https://img.shields.io/github/license/some00/vimgcov) ![Codecov](https://codecov.io/gh/some00/vimgcov/branch/main/graph/badge.svg)


# VIMGCOV

VIMGCOV is a Vim plugin designed to display gcov coverage for C/C++ code.

## Dependencies

- Recent gcov command-line interface (with JSON output)
- Boost (Process library)
- pybind11
- pkg-config
- RapidJSON

## Installation

To install the plugin, use your preferred plugin manager and include this repository and its dependencies in your `.vimrc`:

```vim
" in .vimrc for Vundle
:Plugin 'google/vim-maktaba'
:Plugin 'google/vim-coverage'
:Plugin 'some00/vimgcov'
" then execute :PluginInstall
```

Next, install the native Python module:

```bash
cd ~/.vim/bundle/vimgcov
cmake -S . _build \
    -DENABLE_TESTS=OFF \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON \
    -DCMAKE_BUILD_TYPE=Release
cmake --build _build
```

This will create `_vimgcov.<python-version>.so` in the `python` folder. It’s a workaround, but it functions perfectly.

## Usage

The plugin searches for `.gcno` files recursively in the current working directory and assumes that gcov returns absolute paths for sources, which is typical for CMake-based projects. It works best when Vim is started from the root of the source tree with a build folder created there.

To use, open a source file and execute:

```vim
:CoverageToggle! " command from vim-coverage, for which this repo is a provider
```
