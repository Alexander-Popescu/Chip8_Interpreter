#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>

//SDL screen variables

//CPU register array
char V[16];

//Index register
unsigned short idex;

//program counter
unsigned short pc;

//stack pointer
unsigned short sp;

//emulated stack
unsigned short stack[16];

//emulated memory
uint8_t mem[4096];

//array of pixels for display output
uint8_t display[64 * 32];

//delay timer
char delayTimer;

//sound timer
char soundTimer;

//store key state
bool keys[16];

//required font
const int font[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void initialize()
{
    pc = 0x200; // start of program
    idex = 0;
    sp = 0;
    //initialize arrays used to 0
    for (int i = 0; i < 64 * 32; i++)
    {
        display[i] = 0;
    }
    for (int i = 0; i < 4096; i++)
    {
        mem[i] = 0;
    }
    for (int i = 0; i < 16; i++)
    {
        stack[i] = 0;
        V[i] = 0;
    }
    //load font
    for (int i = 0; i < 80; i++)
    {
        mem[0x50 + i] = font[i];
    }
}

static const SDL_Scancode input_map[16] = {//list of keyboard inputs to shorten input detection
    SDL_SCANCODE_1,
    SDL_SCANCODE_2,
    SDL_SCANCODE_3,
    SDL_SCANCODE_4,
    SDL_SCANCODE_Q,
    SDL_SCANCODE_W,
    SDL_SCANCODE_E,
    SDL_SCANCODE_R,
    SDL_SCANCODE_A,
    SDL_SCANCODE_S,
    SDL_SCANCODE_D,
    SDL_SCANCODE_F,
    SDL_SCANCODE_Z,
    SDL_SCANCODE_X,
    SDL_SCANCODE_C,
    SDL_SCANCODE_V,
};

void handleKey(SDL_KeyboardEvent *event)
{
	if (event->repeat == 0)
	{
		for (int i = 0; i < 16; i++)
        {
            if (event->keysym.scancode == input_map[i])
            {
                if (event->type == SDL_KEYDOWN)
                {
                    keys[i] = 1;
                }
                if (event->type == SDL_KEYUP)
                {
                    keys[i] = 0;
                }
            }
        }
        if (event->keysym.scancode == SDL_SCANCODE_ESCAPE)
        {
            exit(1);
        }
	}
}

void detect_input()
{
    SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
                exit(0);
                break;
            case SDL_KEYDOWN:
			case SDL_KEYUP:
				handleKey(&event.key);
				break;
			default:
				break;
		}
	}
}

void run_emu_cycle()
{
    //get opcode (2 bytes)
    unsigned short opcode = mem[pc] << 8 | mem[pc + 1];
    
    //get variables from opcode
    unsigned short x = (opcode & 0x0F00)>> 8;
    unsigned short y = (opcode & 0x00F0) >> 4;
    unsigned short address;
    uint8_t NN = opcode & 0x00FF;

    //decode opcode by ANDing for digits to compare to list on wikipedia
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x00FF) {
                //00E0
                case 0x00E0:
                    printf("00E0");
                    for (int i = 0; i < 64 * 32; i++)
                    {
                        display[i] = 0;
                    }
                    pc += 2;
                    break;
                //00EE
                case 0x00EE:
                    printf("00EE");
                    pc = stack[sp];
                    sp--;
                    pc += 2;
                    break;
                default:
                    printf("Unkown %X\n", opcode);
                    break;
            }
            break;

        // 1NNN
        case 0x1000:
        printf("1NNN");
            address = opcode & 0x0FFF;
            pc = address;
            break;

        // 2NNN
        case 0x2000:
            printf("2NNN");
            address = opcode & 0x0FFF;
            sp++;
            stack[sp] = pc;
            pc = address;
            break;

        // 3XNN
        case 0x3000:
            printf("3XNN");
            if (V[x] == NN)
            {
                pc += 2;
            }
            pc += 2;
            break;

        // 4XNN
        case 0x4000:
            printf("4XNN");
            if (V[x] != NN)
            {
                pc += 2;
            }
            pc += 2;
            break;

        // 5XY0
        case 0x5000:
            printf("5XY0");
            if (V[x] == V[y])
            {
                pc += 2;
            }
            pc += 2;
            break;

        // 6XNN
        case 0x6000:
            printf("6XNN");
            V[x] = NN;
            pc += 2;
            break;

        // 7XNN
        case 0x7000:
            printf("7XNN");
            V[x] = V[x] + NN;
            pc += 2;
            break;

        // 8XYn
        case 0x8000:
            switch (opcode & 0x000F) {
                // 8XY0
                case 0x0000:
                    printf("8XY0");
                    V[x] = V[y];
                    pc += 2;
                    break;

                // 8XY1
                case 0x0001:
                printf("8XY1");
                    V[x] = (V[x] | V[y]);
                    pc += 2;
                    break;

                // 8XY2
                case 0x0002:
                    printf("8XY2");
                    V[x] = (V[x] & V[y]);
                    pc += 2;
                    break;

                // 8XY3
                case 0x0003:
                    printf("8XY3");
                    V[x] = (V[x] ^ V[y]);
                    pc += 2;
                    break;

                // 8XY4
                case 0x0004:
                    printf("8XY4");
                    if ((V[x] + V[y]) > 0xFF)
                    {
                        V[0xF] = 1;
                    }
                    else
                    {
                        V[0xF] = 0;
                    }
                    V[x] = V[x] + V[y];
                    pc += 2;
                    break;

                // 8XY5
                case 0x0005:
                    printf("8XY5");
                    V[0xF] = 0;
                    if (V[x] > V[y]) 
                    {
                        V[0xF] = 1;
                    }
                    V[x] -= V[y];
                    pc += 2;
                    break;

                // 8XY6
                case 0x0006:
                    printf("8XY6");
                    V[0xF] = V[x] & 1;
                    V[x] = (V[x] >> 1);
                    pc += 2;
                    break;

                // 8XY7
                case 0x0007:
                    printf("8XY7");
                    if (V[y] > V[x])
                    {
                        V[0xF] = 1;
                    }
                    else
                    {
                        V[0xF] = 0;
                    }
                    V[x] = V[y] - V[x];
                    pc += 2;
                    break;

                // 8XYE
                case 0x000E:
                    printf("8XYE");
                    V[0xF] = x >> 7;
                    V[x] <<= 1;
                    pc += 2;
                    break;

                default:
                    printf("Unknown %X\n", opcode);
                    break;
            }
            break;

        // 9XY0
        case 0x9000:
            printf("9XY0");
            if (V[x] != V[y])
            {
                pc += 2;
            }
            pc += 2;
            break;

        // ANNN
        case 0xA000:
            printf("ANNN");
            address = opcode & 0x0FFF;
            idex = address;
            pc += 2;
            break;

        // BNNN
        case 0xB000:
            printf("BNNN");
            address = opcode & 0x0FFF;
            pc = address + V[0];
            break;

        // CXNN
        case 0xC000:
            printf("CXNN");
            V[x] = rand() & NN;
            pc += 2;
            break;

        //DXYN pain
        case 0xD000:
            printf("DXYN");
            V[0xF] = 0;
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            for (int row = 0; row < height; row++)
            {
                pixel = mem[idex + row];
                for (int column = 0; column < 8; column++)
                {
                    if ((pixel & (0x80 >> column)) != 0)
                    {
                        if (display[V[x] + column + ((V[y] + row) * 64)] == 1)
                        {
                            V[0xF] = 1;// if pixel turns off set collision flag
                        }

                        display[V[x] + column + ((V[y] + row) * 64)] ^= 1;
                    }
                }
            }
            pc += 2;
            break;

        case 0xE000:
            switch (opcode & 0x00FF) {
                // EX9E
                case 0x009E:
                    printf("EX9E");
                    if(keys[V[x]] == 1)
                    {
                        pc += 2;
                    }
                    pc += 2;
                    break;

                // EXA1
                case 0x00A1:
                    printf("EXA1");
                    if(keys[V[x]] == 0)
                    {
                        pc += 2;
                    }
                    pc += 2;
                    break;

                default:
                    printf("Unkown %X", opcode);
            }
            break;

        case 0xF000:
            switch (opcode & 0x00FF) {
                // FX07
                case 0x0007:
                    printf("FX07");
                    V[x] = delayTimer;
                    pc += 2;
                    break;

                // FX0A
                case 0x000A:
                    printf("FX0A");
                    for (int i = 0; i < 16; i++)
                    {
                        if (keys[i] == 1)
                        {
                            V[x] = i;
                            pc += 2;
                            break;
                        }
                    }
                    break;

                // FX15
                case 0x0015:
                    printf("FX15");
                    delayTimer = V[x];
                    pc += 2;
                    break;

                // FX18
                case 0x0018:
                    printf("FX18");
                    soundTimer = V[x];
                    pc += 2;
                    break;

                // FX1E
                case 0x001E:
                    printf("FX1E");
                    idex = idex + V[x];
                    pc += 2;
                    break;

                // FX29
                case 0x0029:
                    printf("FX29");
                    idex = V[x] * 5;
                    pc += 2;
                    break;

                //FX33
                case 0x0033:
                    printf("FX33");
                    mem[idex] = V[(x) >> 8] / 100;
                    mem[idex+1] = (V[(x) >> 8] / 10) % 10;
                    mem[idex+2] = V[(x) >> 8] % 10;
                    pc += 2;
                    break;

                // FX55
                case 0x0055:
                    printf("FX55");
                    for (int i = 0; i <= x; i++)
                    {
                        mem[idex + i] = V[i];
                    }
                    pc += 2;
                    break;

                //FX65
                case 0x0065:
                    printf("FX65");
                    for (int i = 0; i <= x; i++)
                    {
                        V[i] = mem[idex + i];
                    }
                    pc += 2;
                    break;

                default:
                    printf("Unknown %X\n", opcode);
                    break;
            }
            break;

        default:
            printf("Unknown %X\n", opcode);
            break;
    }
}


int main(int argc, char *argv[])
{
    long timertracker = 0;
    printf("App is Running\n");
    //setup SDL window
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *surface;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }

    if (SDL_CreateWindowAndRenderer(1200, 600, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return 3;
    }
    SDL_RenderSetLogicalSize(renderer, 64, 32);

    //set variables to zero and prepare for emulation
    initialize();

    //load game into memory
    //open rom
    FILE* fp = fopen(argv[1], "rb");

    if (fp == NULL) 
    {
        printf("fopen failure\n");
        return 1;
    }
    //load rom into chip8 memory
    fread(&mem[0x200], 1, 4096 - 0x200, fp);

    //SDL_Surface * image = SDL_LoadBMP("sample.bmp");
    SDL_Surface *screen = SDL_GetWindowSurface(window);
    SDL_Texture * texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    uint8_t *pixels = malloc(64 * 32 * 4 * sizeof(uint8_t));


    
    //cycle
    while (true)
    {
        //detect keyboard presses and update keys array
        detect_input();
        //run next opcode
        run_emu_cycle();
        timertracker++;
        // update texture
        for (int i = 0; i < 64 * 32; i++)
        {
            pixels[i * 4] = 255;
            pixels[i * 4 + 1] = display[i] ? 255 : 0;
            pixels[i * 4 + 2] = display[i] ? 255 : 0;
            pixels[i * 4 + 3] = display[i] ? 255 : 0;
        }
        SDL_UpdateTexture(texture, NULL, pixels, 64 * 4);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        //decrement timers
        if (delayTimer > 0 && timertracker >= 10)
        {
            delayTimer--;
            timertracker = 0;
        }
        if (soundTimer > 0)
        {
            soundTimer--;
        }
    }
}