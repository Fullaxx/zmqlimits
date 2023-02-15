# ZMQ Limits (measured at ZMQbg/IO/0)

## MPM to 1 destination
RATE=25000  : ~15% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz \
RATE=50000  : ~30% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz \
RATE=75000  : ~45% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz \
RATE=100000 : ~60% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz \
RATE=125000 : ~70% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz \
RATE=150000 : ~90% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz
```
./pub.exe -m -Z tcp://*:9999 -r RATE
./sub.exe -m -Z tcp://localhost:9999
```

## MPM to 8 destinations
RATE=25000  : ~15% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz \
RATE=50000  : ~30% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz \
RATE=75000  : ~45% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz \
RATE=100000 : ~60% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz \
RATE=125000 : ~70% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz \
RATE=150000 : ~99% CPU usage on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz
```
./pub.exe -m -d 8 -Z tcp://*:9999 -r RATE
./sub.exe -m -d 8 -Z tcp://localhost:9999
```

## Hex JSON to 1 destination
RATE=7000   : PUB pegged & SUB backlogged on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz
```
./pub.exe -j -Z tcp://*:9999 -r RATE
./sub.exe -j -Z tcp://localhost:9999
```

## Hex JSON to 8 destinations
RATE=4500   : PUB pegged on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz
```
./pub.exe -j -d 8 -Z tcp://*:9999 -r RATE
./sub.exe -j -d 8 -Z tcp://localhost:9999
```

## Base64 JSON to 1 destination
RATE=42000   : PUB pegged on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz
```
./pub.exe -j -Z tcp://*:9999 -r RATE --b64
./sub.exe -j -Z tcp://localhost:9999
```

## Base64 JSON to 8 destinations
RATE=32000   : PUB pegged & SUB backlogged on Intel(R) Core(TM) i7-4610M CPU @ 3.00GHz
```
./pub.exe -j -d 8 -Z tcp://*:9999 -r RATE --b64
./sub.exe -j -d 8 -Z tcp://localhost:9999
```
