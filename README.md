# embedded_gcov
This code and scripts allow for GCC code coverage to be performed on an embedded platform and viewing results.


# depenencies
- This project assumes that you have ELM-Chans FatFS file system http://elm-chan.org/fsw/ff/00index_e.html available for writing .gcda files to a device. 
  - FatFS needs to be configured using `USE_LFN` to suport the potentially long file names of the instrumented source code.

# host computer setup
- Install lcov
  - For Ubuntu: `sudo apt install lcov`
- (Optional) Install gcovr
  - For Ubuntu: `sudo apt install gcovr`

# project setup
- Add the source code provided under `src` to the souce code location for your project. 
- Add `--coverage` to your gcc compile options 
- Add `--coverage` to your gcc linker options
- Build your applicaion. As a part of the compilation, you should see .gcno files alongside the .o files. 

# storing results
The application has finished and runs `_exit()` after `main()`, each source file insturmented will have a .gcda file written to the specifed FAT filesystem. 

# post processing
Using available tools, you can create an HTML report of the code coverage from application execution. Here's how:
- Copy the .gcda files from the SD card or other medium to the same directory as the .gcno files on your host computer.
- Using lcov
  - Move to the directory where the .gcda and .gcno files reside
  - `lcov --capture --directory ./ --output-file /tmp/embedded_gcov.info` Note: output path is arbitrary
  - `genhtml --prefix ./ --ignore-errors /tmp/embedded_gcov.info --legend --title "You're Title Here" --output-directory /tmp/embedded_gcov`
- Using gcov ( simpler ) 
  - Move to the directory where the .gcda and .gcno files reside
  - `gcov -r .`
  - `gcov -r . --branches`


# attribution
This project was inspired by the great work by Erich Styger over at https://mcuoneclipse.com/2014/12/26/code-coverage-for-embedded-target-with-eclipse-gcc-and-gcov/ 

This also somewhat follows, but it much simpler than https://github.com/reeteshranjan/libgcov-embedded 
