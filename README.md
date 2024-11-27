# ffmpeg_wrapper

Welcome to the **ffmpeg_wrapper** project!
This project is a C++17 wrapper around FFmpeg.

## Branching Strategy

This project uses the following branching strategy for version management:

- **`master` branch**  
  This is the default branch of the project, primarily used for providing project information and stable version references. The `master` branch is not directly used for development.

- **`develop` branch**  
  This is our main development branch, where all new features and changes will be made. If you are a developer planning to contribute to the project, please pull the latest code from this branch and create feature branches based on it.

- **Release versions (tags, e.g., `1.0.0`)**  
  Stable release versions of the project are marked with tags. Each version is tested and suitable for production environments.

## How to Choose the Right Branch/Version

- **For developers**  
  If you wish to contribute code or try out the latest features, use the `develop` branch.
  ```bash
  git checkout develop
  ```

- **For users**  
  If you need stable functionality, choose the corresponding release version. For example:
  ```bash
  git checkout tags/1.0.0
  ```

## Quick Start
### Clone the project

1. Clone the repository to your local machine:
    ```bash
    git clone https://github.com/shecannotsee/ffmpeg_wrapper.git
    cd ffmpeg_wrapper
   ```

2. Switch to a branch or tag:

- To use the development branch:
    ```bash
    git checkout develop
    ```

- To use a stable release version:
    ```bash
    git checkout tags/1.0.0
    ```
### Build the project

1. Ensure the following dependencies are installed:
   - FFmpeg 4.3 or higher
   - A C++17 compatible compiler (e.g., GCC 9.0+ or Clang 10.0+)


2. Build the project:
    ```bash
    mkdir build
    cd build
    cmake ..
    make
   ```

## Features
- Demuxing
- Faster and safer encoding/decoding features
- Support for drawing on raw image formats

## Contributing Guidelines

We welcome developers to contribute to the project!

## Issue Reporting

If you encounter any issues during usage, please visit the Issues page to submit a problem. We will respond and resolve it as soon as possible.

## Open Source License

This project is open-source under the MIT License. You are free to use, modify, and distribute the code.

---

Thank you for your support and attention!If you like this project, feel free to give us a star!


