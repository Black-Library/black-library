# black-library
Black Library main application. 

## Default directory structure
    mnt
    └── black-library
        ├── db
        ├── log
        ├── output
        └── store

The `db` directory contains the `catalog.db` database. The `store` directory contains directories with names as the UUID of a particular stored media. The user will need read-write permissions for black-library and its child directories. 

## Dependencies for main application
Please use the `install_build_deps.sh` file to install the main application dependencies. 

### GUI dependencies
The GUI uses dear imgui using glfw3, a portable library for OpenGL.

It can be installed on ubuntu with

```sudo apt install libglfw3-dev```

## Building the project
The option `-DBUILD_GUI=ON` can be added to the cmake command to build the GUI. 

```mkdir build```

and

```cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=install . -Bbuild```

or

```cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install . -Bbuild```

then

```make install -j`nproc` -Cbuild```

## Running the main application
If installed in ./install

```./install/bin/blacklibrary_app --config black-library-config.json```

## Running the CLI interface
If installed in ./install

```./install/bin/blacklibrary_cli --config black-library-config.json```

supported commands are listed under the `help` command

## Db migration summary

this will use default export/import locations

```./install/bin/blacklibrary_cli --config black-library-config.json```
```export all```

modify db

```./install/bin/blacklibrary_cli --config black-library-config.json```
```import all```

## Syntax guidelines

We will (loosely) follow the google cpp style guide for this project

https://google.github.io/styleguide/cppguide.html

## Related projects by others
https://github.com/Flameish/Novel-Grabber
https://github.com/krissgjeng/webnovel-dl
