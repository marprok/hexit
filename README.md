<a id="Top"></a>

[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]

[contributors-shield]: https://img.shields.io/github/contributors/marprok/hexit?style=for-the-badge
[contributors-url]: https://github.com/marprok/hexit/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/marprok/hexit?style=for-the-badge
[forks-url]: https://github.com/marprok/hexit/network/members
[stars-shield]: https://img.shields.io/github/stars/marprok/hexit.svg?style=for-the-badge
[stars-url]: https://github.com/marprok/hexit/stargazers
[issues-shield]: https://img.shields.io/github/issues/marprok/hexit.svg?style=for-the-badge
[issues-url]: https://github.com/marprok/hexit/issues
[license-shield]: https://img.shields.io/github/license/marprok/hexit.svg?style=for-the-badge
[license-url]: https://github.com/marprok/hexit/blob/master/LICENSE

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

![Product Name Screen Shot][product-demo]

-   <strong>Hexit</strong> is a easy to use program used to view and edit binary files.
-   Hexit allows editing the raw data contents of a file, instead of other programs which attempt to interpret the data for you.
-   Hexit's UI is separated into three visual areas: an address area on the left, a hexadecimal area in the center, and a character area on the right.

### Built With

-   [![C][CLanguage]][ClanguageURL]

[Clanguage]: https://img.shields.io/badge/C++-0769AD?style=for-the-badge&logo=C%2B%2B&logoColor=white
[ClanguageURL]: https://en.wikipedia.org/wiki/C%2B%2B

## Getting Started

### Prerequisites

-   gcc(c++-20 support is needed)
-   Make
-   CMake(3.23 or greater)
-   libncurses-dev

### Installation

1. Clone the repo
    ```sh
    git clone https://github.com/marprok/hexit.git
    ```
1. Cd into the directory
    ```sh
    cd hexit
    ```
1. Make a direcoty named "build" and cd into it
    ```sh
    mkdir build && cd build
    ```
1. Create build files
    ```sh
    cmake -DCMAKE_BUILD_TYPE=Release ..
    ```
1. Build the Project
    ```sh
    make
    ```

## Usage

-   Help Page:
    -   `./hexit -h`
-   Display the hex dump of a file:

    -   `./hexit -f (--file) <file> [options]`

-   Options:

    -   Hexadecimal or decimal byte offset to seek during startup: `-o (--offset) <offset>`

-   If no file is given via the -f flag, then Hexit will read bytes from standard input
    until EOF is reached. When displaying the hex dump of standard input, saving will do nothing.

## Controls

| Key               | Function              |
| ----------------- | --------------------- |
| ctrl + S          | Save the file         |
| ctrl + X          | Toggle HEX mode       |
| ctrl + A          | Toggle ASCII mode     |
| ctrl + Q          | Exit the editor       |
| ctrl + G          | Go to byte            |
| Arrows keys       | Move the cursor       |
| Page-up/Page-down | Move the page up/down |

## License

Distributed under the MIT License. See `LICENSE` for more information.

<p align="right">(<a href="#Top">back to top</a>)</p>
