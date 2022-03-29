#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <math.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL2_gfxPrimitives.h"
#include "SDL2/SDL_ttf.h"
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

/* Screen size */
#define WIDTH	500
#define HEIGHT	500
#define PI 3.14159265

SDL_Color fgcolor = {255, 255, 255}, bgcolor = {0,0,0};

void drawGauge(int thickness, int radius, double value, SDL_Renderer* rend, TTF_Font* font){
    int cutOut = 60;
    int centerX = WIDTH/2;
    int centerY = HEIGHT/2;
    int arcStart = 90 + cutOut;
    int arcEnd = 90 - cutOut;
    double percent = value / 100.0; // figure out the percentage
    double arcPercent = percent * ((360 - (cutOut*2)) * PI / 180); // multiply the total angle by percentage to find indicator angle
    double pointer = arcPercent + ((90 + cutOut) * PI / 180); // add in offset for coordinate system starting at y+ and cutOut
    double offset = 6 * PI / 180; // offset the triangle 2 degrees either side
    // define triangle points
    int tipX = (radius-6) * cos(pointer) + centerX;
    int tipY = ((radius-6) * sin(pointer)) + centerY;
    int leftX = ((radius - 20) * cos(pointer-offset)) + centerX;
    int leftY = ((radius - 20) * sin(pointer-offset)) + centerY;
    int rightX = ((radius - 20) * cos(pointer+offset)) + centerX;
    int rightY = ((radius - 20) * sin(pointer+offset)) + centerY;
    // percentage code
    char textValue[6];
    sprintf(textValue, "%g%%", value);
    int textWidth;
    int textHeight;
    TTF_SizeText(font, textValue, &textWidth, &textHeight);
    SDL_Rect textRect;
    // draw text in middle
    textRect.h = textHeight;
    textRect.w = textWidth;
    textRect.x =  centerX - textWidth/2;
    textRect.y = textHeight + 30 + centerY;
    // keep drawing arcs out to get desired thickness
    for(int r = radius; r <= radius + thickness; r++){
        arcRGBA(rend, centerX, centerY, r, arcStart, arcEnd, 255, 255, 255, 255);
    }
    // draw triangle showing value of gauge
    filledTrigonRGBA(rend, tipX, tipY, leftX, leftY, rightX, rightY, 255, 255, 255, 255);
    
    SDL_Surface* surfaceMessage = TTF_RenderText_Shaded(font, textValue, fgcolor, bgcolor);
    SDL_Texture* textMesg = SDL_CreateTextureFromSurface(rend, surfaceMessage);
    SDL_RenderCopy(rend, textMesg, NULL, &textRect);
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(textMesg);
}

int main(int argc, char** argv){
    int devHandle;
    if(argc > 2){
        devHandle = open(argv[2], O_RDONLY | O_NONBLOCK);
    }
    else{
        devHandle = open("/dev/ttyACM0", O_RDONLY | O_NONBLOCK);
    }
    if(devHandle == -1){
        std::cout << "Cannot open file" << std::endl;
        exit(-1);
    }
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
       printf("error initializing SDL: %s\n", SDL_GetError());
    }
    
    TTF_Init();
    TTF_Font* lora = TTF_OpenFont("Lora-Regular.ttf", 40);
    if(lora == NULL){
        std::cout << "Font file not included" << std::endl;
        exit(-1);
    }
    
    SDL_Window* win = SDL_CreateWindow("Gauge Test",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);

    //triggers the program that controls
    //your graphics hardware and sets flags
    Uint32 render_flags = SDL_RENDERER_ACCELERATED;

    //creates a renderer to render our images
    SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);

    int close = 0;
    double gaugeVal = 50.0; // default start at 50
    SDL_RenderClear(rend);
    drawGauge(10, 150, gaugeVal, rend, lora);
    SDL_RenderPresent(rend);
    
    struct termios config;
    
    // don't touch the black magic parameters
    config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
    config.c_oflag = 0;
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    config.c_cflag &= ~(CSIZE | PARENB);
    config.c_cflag |= CS8;
    config.c_cc[VMIN]  = 1;
    config.c_cc[VTIME] = 0;
    if(!isatty(devHandle)){
        std::cout << "File is not a term" << std::endl;
    }
    if(cfsetispeed(&config, B9600) < 0 || cfsetospeed(&config, B9600) < 0) {
        std::cout << "Cannot set baud rate" << std::endl;
        exit(-1);
    }
    if(tcsetattr(devHandle, TCSANOW, &config) < 0) {
        std::cout << "Cannot set term attributes" << std::endl;
        exit(-1);
    }

    char result[15];
    int charsRead = 0;
    char position[4];
    while(!close){
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    // handling of close button
                    close = 1;
                    break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE: 
                            close = 1;
                            break;
                    }
                    break;
            }
        }
        charsRead = read(devHandle, &result, 15);
        if(charsRead > 0){
            if(strcmp(result, "BUTTON PRESSED") == 0){
                // handle button press here
                std::cout << "Button pressed" << std::endl;
            }
            else if(strcmp(result, "Position: ")){
                strncpy(position, result + 10, 4);
                SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
                SDL_RenderClear(rend);
                gaugeVal = atoi(position);
                drawGauge(10, 150, gaugeVal, rend, lora);
                SDL_RenderPresent(rend);
            }
            
        }
    
    }
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    return 0;

}
