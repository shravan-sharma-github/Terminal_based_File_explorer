
# File Explorer

File Explorer is ```C++``` application, which provide basic file explorer feature set.

## Guide

Some assumptions are taken so that it would be easy to use and make it more secure.

* **Root** and **Home** of the application is **same** and that will be the directory at which application start.
* There are more then two ```.cpp``` files, and the main file that you have to run is **main.cpp** .
* All files should remain in same folder, i.e. jus extract .zip file wherever you want to start and go ahead.

## Run Application

Open the terminal and go to the directory where you extracted .zip file and run these command and your application will start 
```bash
g++ main.cpp
./a.out
```


## General Instructions

### Normal mode

**Left-Arrow** : Previously visited directory

**Right-Arrow** : Next directory (It only work if Left-arrow used)

**Up-Arrow** : Navigate up in the file list

**Down-Arrow** : Navigate down in the file list

**h -key** : press h to go to home directory

**Backspace** : Up one level in directory

**Enter** : To open file/directory

**q -key** : To quit application

**: (colon)** : To switch to command mode  

### Command mode
Used to enter shell commands

**Copy** : `copy <source_file(s)> <destination_directory>`

**Move** : `move <source_file(s)> <destination_directory>`

**Rename** : `rename <old_filename> <new_filename>`

**Create Directory** : `create_dir <dir_name> <destination_path>`

**Create File** : `create_file <file_name> <destination_path>`

**Delete File** : `delete_file <file_path>`

**Delete Directory** : `delete_dir <dir_path>`

**Goto** : `goto <location>`

**Search** : `search <file_name>` or `search <directory_name>` 

**ESC key** : To go back to Normal Mode

