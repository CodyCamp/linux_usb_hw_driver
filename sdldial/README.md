# SDL Dial

## Dependencies

Install SDL2, SDL\_TTF, and SDL\_gfx <br />
`sudo apt-get install -y build-essential g++ libsdl2-dev libsdl2-gfx-dev libsdl2-ttf-dev`

## Running
To run make sure to use sudo as you usually can't read from /dev/ without it. `sudo ./dial`

The default device is /dev/ttyACM0 but you can set a different device with `sudo ./dial /dev/ttyACM1`
