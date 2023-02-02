# zmqlimits

## Compile the code
```
./compile.sh
```

## MPM to 1 destination
Push 100000 msgs/sec to 1 destination using multi-part messages
```
./pub.dbg -m -Z tcp://*:9999 -r 100000
./sub.dbg -m -Z tcp://localhost:9999
```

## MPM to 8 destinations
Push 100000 msgs/sec to 8 destinations using multi-part messages
```
./pub.dbg -m -d 8 -Z tcp://*:9999 -r 100000
./sub.dbg -m -d 8 -Z tcp://localhost:9999
```

## JSON to 1 destination
Push 100000 msgs/sec to 1 destination using JSON messages
```
./pub.dbg -j -Z tcp://*:9999 -r 100000
./sub.dbg -j -Z tcp://localhost:9999
```

## JSON to 8 destinations
Push 100000 msgs/sec to 8 destinations using JSON messages
```
./pub.dbg -j -d 8 -Z tcp://*:9999 -r 100000
./sub.dbg -j -d 8 -Z tcp://localhost:9999
```
