# black-library
Black Library main application. 

## Default directory structure
    mnt
    ├── db
    └── store

The `db` directory contains the `catalog.db` database. The `store` directory contains directories with names as the UUID of a particular stored media. The user will need read-write permissions for both directories. 

## Building the project
```mkdir build```

and

```cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=install . -Bbuild```

or

```cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install . -Bbuild```

then

```make install -j`nproc` -Cbuild```

## Running the main application
If installed in ./install

```./install/bin/blacklibrary_app -d /mnt/db/catalog.db -s /mnt/store/```

## Running the CLI interface
If installed in ./install

```./install/bin/blacklibrary_cli -d /mnt/db/catalog.db -s /mnt/store/```

supported commands are listed under the `help` command

## Syntax guidelines

We will (loosely) follow the google cpp style guide for this project

https://google.github.io/styleguide/cppguide.html