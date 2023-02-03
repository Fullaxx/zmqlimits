# ZMQ Limits

## MPM to 1 destination
RATE=50000  : ~30% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz \
RATE=100000 : ~50% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz \
RATE=140000 : ~90% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz
```
./pub.exe -m -Z tcp://*:9999 -r RATE
./sub.exe -m -Z tcp://localhost:9999
```

## MPM to 8 destinations
RATE=50000  : ~30% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz \
RATE=100000 : ~50% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz \
RATE=140000 : ~75% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz
```
./pub.exe -m -d 8 -Z tcp://*:9999 -r RATE
./sub.exe -m -d 8 -Z tcp://localhost:9999
```

## JSON to 1 destination
RATE=7500   : PUB pegged & SUB backlogged on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz
```
./pub.exe -j -Z tcp://*:9999 -r RATE
./sub.exe -j -Z tcp://localhost:9999
```

## JSON to 8 destinations
RATE=4500   : PUB pegged on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz
```
./pub.exe -j -d 8 -Z tcp://*:9999 -r RATE
./sub.exe -j -d 8 -Z tcp://localhost:9999
```
