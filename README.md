# joinPhotos

Program to make series of images of fluidized beds to show evolution of the beds.

## Install

On CMakeLists.txt fix the opencv path for your computer.

Run to configure the compiler.
```bash
cmake .
```

Run to compile the program. 
```bash
make
```

## Usage

Run the command as:
```bash
./joinImages "Path/to/images/" t0 dt tf h 
```

Where:
- "Path/to/images/" : is the path to the images saved with the pattern "image_*.tif"
- t0 : first frame
- dt : frames to skip between images
- tf : end frame
- h : height of the final joined image (crop extra blank space)
