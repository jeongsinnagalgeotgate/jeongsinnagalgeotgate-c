# Jeongsinnagalgeotgate C interpreter
This interpreter is in beta. Please help improving this by reporting bugs in [issues page](https://github.com/jeongsinnagalgeotgate/jeongsinnagalgeotgate-c/issues).

## Installation
1. Clone this git repository.
```bash
git clone https://github.com/jeongsinnagalgeotgate/jeongsinnagalgeotgate-c.git
```
2. Compile interpreter.c
```bash
cd jeongsinnagalgeotgate
gcc -o interpreter interpreter.c
# For debug mode use:
gcc -o interpreter interpreter.c -DDEBUG
```

## Usage
```bash
./interpreter filename[ -s stack_size][ -q queue_size]
```

## Example
For some examples see [example](https://github.com/jeongsinnagalgeotgate/example).
