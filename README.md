<<<<<<< HEAD
1. Build
============
build server
```
cd /path/to/torcs
./configure --prefix=/path/to/install
make
make install
make datainstall
```

build client
```
cd /path/to/torcs/src/scr-client-cpp
make
```

2. Run
============
run server
```
cd /path/to/install/bin
./torcs
```

run client
```
cd /path/to/torcs/src/scr-client-cpp
./scr_client
```

---------------------------------------------------------------

3. Build a driver
============
set env
```
export TORCS_BASE=/path/to/torcs
export MAKE_DEFAULT=$TORCS_BASE/Make-default.mk
```

build
```
cd /path/to/torcs/src/drivers/driverExample
make
make install
```

clean
```
make clean
```
=======
Hello torcs_data
>>>>>>> 18cbacbe56b4c923f2ba486da2b5b571d135e0a8
