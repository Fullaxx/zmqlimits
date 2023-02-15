# zmqlimits

## Code Layout
| File                     | Purpose                   |
|--------------------------|---------------------------|
| [pub.c](pub.c)           | Shared Publisher code     |
| [sub.c](sub.c)           | Shared Subscriber code    |
| [mpm_fmt.h](mpm_fmt.h)   | Multi-Part Message format |
| [json_fmt.h](json_fmt.h) | JSON Message format       |

## Compile the code
```
./compile.sh
```

## MPM to 1 destination
Push 100000 msgs/sec to 1 destination using multi-part messages with binary payload
```
./pub.exe -m -Z tcp://*:9999 -r 100000
./sub.exe -m -Z tcp://localhost:9999
```

## MPM to 8 destinations
Push 100000 msgs/sec to 8 destinations using multi-part messages with binary payload
```
./pub.exe -m -d 8 -Z tcp://*:9999 -r 100000
./sub.exe -m -d 8 -Z tcp://localhost:9999
```

## Hex JSON to 1 destination
Attempt 100000 msgs/sec to 1 destination using JSON messages with hex encoded payload
```
./pub.exe -j -Z tcp://*:9999 -r 100000
./sub.exe -j -Z tcp://localhost:9999
```

## Hex JSON to 8 destinations
Attempt 100000 msgs/sec to 8 destinations using JSON messages with hex encoded payload
```
./pub.exe -j -d 8 -Z tcp://*:9999 -r 100000
./sub.exe -j -d 8 -Z tcp://localhost:9999
```

## Base64 JSON to 1 destination
Attempt 100000 msgs/sec to 1 destination using JSON messages with base64 encoded payload
```
./pub.exe -j -Z tcp://*:9999 -r 100000 --b64
./sub.exe -j -Z tcp://localhost:9999
```

## Base64 JSON to 8 destinations
Attempt 100000 msgs/sec to 8 destinations using JSON messages with base64 encoded payload
```
./pub.exe -j -d 8 -Z tcp://*:9999 -r 100000 --b64
./sub.exe -j -d 8 -Z tcp://localhost:9999
```
