<a id="Top"></a>

<p align="center">
  <a href="https://github.com/marprok/hexit/network/members">
        <img src="https://img.shields.io/github/forks/marprok/hexit" alt="Forks"></a>
  <a href="https://github.com/marprok/hexit/stargazers">
        <img src="https://img.shields.io/github/stars/marprok/hexit.svg" alt="Stars"></a>
  <a href="https://github.com/marprok/hexit/actions/workflows/cmake.yml/badge.svg">
        <img src="https://github.com/marprok/hexit/actions/workflows/cmake.yml/badge.svg" alt="Build Status"></a>
  <a href="https://github.com/marprok/hexit/graphs/contributors">
        <img src="https://img.shields.io/github/contributors/marprok/hexit" alt="Contributors"></a>
  <a href="https://github.com/marprok/hexit/issues">
        <img src="https://img.shields.io/github/issues/marprok/hexit.svg" alt="Issues"></a>
  <a href="https://github.com/marprok/hexit/blob/master/LICENSE">
        <img src="https://img.shields.io/github/license/marprok/hexit.svg" alt="License"></a>
</p>

<br />
<div align="center">
  <h1 align="center">Hexit</h1>
  <p align="center">
    A minimalistic Hex-Editor!
    <br />
    <br />
    <a href="https://github.com/marprok/hexit">View Demo</a>
    ·
    <a href="https://github.com/marprok/hexit/issues">Report Bug</a>
    ·
    <a href="https://github.com/marprok/hexit/issues">Request Feature</a>
  </p>
  <p>
  </p>
</div>

<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#controls">Controls</a></li>
    <li><a href="#license">License</a></li>
  </ol>
</details>

## About The Project

[product-demo]: https://github.com/marprok/hexit/assets/18293204/73c7c699-9d60-40c1-8f6a-911c67d5514f

Hexit aims to be a simple yet efficient hex editor.

-   It allows you to view and/or edit raw bytes in a file.
-   The UI is separated into three visual areas: the byte offset on the left, the hexadecimal representation in the center, and the ASCII interpretation on the right.
-   Hexit will not load the whole file in memory but it will instead load different 'chunks' of the file on demand. This makes it use as little memory as possible regardless of the actual size of the file.
-   You can easily jump into different absolute byte offsets.
-   Hexit will attempt to display the actual type of the file and display it on the bottom right of the screen. It does that by trying to match the first few bytes of each file against a database of well known file signatures.
-   Hexit can be used as a shell filter. 

![Product Name Screen Shot][product-demo]

## Getting Started

### Prerequisites

-   gcc(c++-20 support is needed)
-   Make
-   CMake(3.23 or greater)
-   libncurses-dev

### Installation

1. clone the repo
    ```sh
    git clone https://github.com/marprok/hexit.git
    ```
2. cd into the directory
    ```sh
    cd hexit
    ```
3. Change the default parameters contained in `src/confing.h` in case the default values are not what you need
    ```sh
    mkdir build && cd build
    ```
4. make a direcoty named `build` and cd into it
    ```sh
    mkdir build && cd build
    ```
5. configure cmake
    ```sh
    cmake -DCMAKE_BUILD_TYPE=Release ..
    ```
6. build the Project
    ```sh
    make -j8
    ```

## Usage

-   help:
    -   `./hexit -h`
-   display the hex dump of a file:

    -   `./hexit -f (--file) <file> [options]`

-   options:

    -   Hexadecimal or decimal byte offset to seek during startup: `-o (--offset) <offset>`

-   If no file is given via the -f flag, then Hexit will read bytes from standard input
    until EOF is reached. When displaying the hex dump of standard input, saving will do nothing.

## Controls

| Key               | Function              |
| ----------------- | --------------------- |
| ctrl + s          | save the file         |
| ctrl + x          | toggle HEX mode       |
| ctrl + a          | toggle ASCII mode     |
| ctrl + q          | exit the editor       |
| ctrl + g          | go to byte            |
| Arrows keys       | move the cursor       |
| Page-up/Page-down | move the page up/down |

## License

Distributed under the MIT License. See `LICENSE` for more information.

<p align="right">(<a href="#Top">back to top</a>)</p>
