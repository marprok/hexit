# Hexit, a minimalistic hex editor

Edit files or display the hex dump of stdin.

![demo](https://github.com/marprok/hexit/assets/18293204/73c7c699-9d60-40c1-8f6a-911c67d5514f)

### Controls

| Key                           | Function        |
|-------------------------------|-----------------|
| ctrl-s                        | Save the file.  |
| ctrl-x                        | Toggle HEX mode.|
| ctrl-a                        | Toggle ASCII mode.|
| ctrl-q                        | Exit the editor.|
| ctrl-g                        | Go to byte.|
| Arrows keys                   | Move the cursor.|
| Page-up/Page-down             | Move the page up/down.|

### Prerequisites

Make sure you have all of the following packages installed.
* gcc(c++-20 support is needed)
* Make
* CMake(3.23 or greater)
* libncurses-dev

### Compile
1. git clone https://github.com/marprok/hexit.git
2. cd hexit
3. mkdir build && cd build
4. cmake -DCMAKE_BUILD_TYPE=Release ..
5. make
6. ./hexit -h

### Limitations
* With hexit you can edit only one file at a time.
