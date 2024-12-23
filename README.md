# Ashmanix Move Face OBS Plugin

<p align="center">
<!-- <image height="400px" src="/images/xxx.png"> -->
</p>

## Introduction

This plugin is designed to allow you to alter a 2D image using facetracking software like MeowFace or VTube Studio. The code is based on the OBS template plugin code. For details on how this works please refer to the [OBS Plugin Template Wiki](https://github.com/obsproject/obs-plugintemplate/wiki).

This plugin is designed to be used with `OBS version 31` and above.

## Usage
For usage instruction please refer to the TBC
<!-- [Wiki](https://github.com/ashmanix/obs-plugin-countdown/wiki). -->

## Build Instructions
To build this code use the included scripts that are taken from the [OBS Plugin Template](https://github.com/obsproject/obs-plugintemplate). This repo uses cmake to build the plugin. For example for MacOS to build the plugin carry out the following steps:
1. Run `cmake --preset macos` to compile the plugin first.
2. Then run `cmake --build --preset macos` to then build the plugin into the `build-macos/RelWithDebInfo` folder.

### GitHub Actions & CI
The scripts contained in github/scripts can be used to build and package the plugin and take care of setting up obs-studio as well as its own dependencies. A default workflow for GitHub Actions is also provided and will use these scripts.

#### Retrieving build artifacts
Each build produces installers and packages that you can use for testing and releases. These artifacts can be found on the action result page via the "Actions" tab in your GitHub repository.

#### Building a Release
Simply create and push a tag and GitHub Actions will run the pipeline in Release Mode. This mode uses the tag as its version number instead of the git ref in normal mode.
