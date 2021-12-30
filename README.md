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
The GUI uses dear imgui using glfw3, a portable library for OpenGL.

It can be installed on ubuntu with

```sudo apt install libglfw3-dev```

Please access the other subrepos for their dependencies. 

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

```./install/bin/blacklibrary_app -d /mnt/black-library/db/catalog.db -s /mnt/black-library/store/```

## Running the CLI interface
If installed in ./install

```./install/bin/blacklibrary_cli -d /mnt/black-library/db/catalog.db -s /mnt/black-library/store/```

supported commands are listed under the `help` command

## Syntax guidelines

We will (loosely) follow the google cpp style guide for this project

https://google.github.io/styleguide/cppguide.html