# VIMGCOV
My vim plugin for showing gcov coverage for C/C++ code.

## Dependencies
- recent gcov commandline interface (with json output)
- Boost (Process library)
- pybind11
- pkg-config
- RapidJSON
## Install the plugin
Using your favourite plugin manager install this repo and it's dependencies.
```
" in .vimrc Vundle
:Plugin 'google/vim-maktaba'
:Plugin 'google/vim-coverage'
:Plugin 'some00/vimgcov'
" execute :PluginInstall
```
and install the native python module.
```
cd ~/.vim/bundle/vimgcov
cmake -S . _build \
	-DENABLE_TESTS=OFF \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	-DCMAKE_CXX_COMPILER=clang++ \
	-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON \
	-DCMAKE_BUILD_TYPE=Release
cmake --build _build
```
This will create `_vimgcov.<python-version>.so` inside `python` folder. A nasty little hack but works perfectly.

## Usage
The plugin searches for `.gcno` files recursively in the current work directory and assumes that gcov retuns absolute path for sources, which is the case for CMake based projects. Works great if vim is started in the root of the source tree and a build folder is created there.
```
" Open source file and execute
:CoverageToggle! " command from vim-coverage, for which this repo is a provider
```
