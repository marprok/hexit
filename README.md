# hexit
A minimalistic hex editor.

HEX mode                   |  ASCII mode
:-------------------------:|:-------------------------:
![hex mode](hex_mode.png)  |  ![ascii mode](ascii_mode.png)


### Controls

| Key                           | Function        |
|-------------------------------|-----------------|
| ctrl-s                        | Save the file.  |
| ctrl-x                        | Toggle HEX mode.|
| ctrl-a                        | Toggle ASCII mode.|
| ctrl-q                        | Exit the editor.|
| Arrows(up,down,left,right)    | Move the cursor.|
| Hexadecimal digits(HEX mode)  | Change the value of the nibble that the cursor points to.|
| Printable ASCII characters(ASCII mode) | Change the value of the byte that the cursor points to.|

### Prerequisites

Make sure you have all of the following packages installed.
* g++-9 or greater(c++-17 support is needed)
* make
* libncurses-dev

### Compile
1. git clone https://github.com/marprok/hexit.git
2. cd hexit
3. make

### Limitations

* As it is now, hexit will attempt to load the entire file in memory.
* You can not jump to different parts of the file.
* With hexit you can edit only one file at a time.
