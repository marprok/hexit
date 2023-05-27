# Hexit, a minimalistic hex editor

Edit files or display the hex dump of stdin.

<img src="https://github.com/marprok/hexit/assets/18293204/da3e1252-a4ff-428a-8e17-3f971419f124" width="520" height="280">

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

### Limitations
* With hexit you can edit only one file at a time.
