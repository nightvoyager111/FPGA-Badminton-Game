#include <stdlib.h>
#include <stdbool.h>

short int create_color(int r, int g, int b);
void plot_pixel(int x, int y, short int pixel_color);
void clear_screen();

/*short int light_blue = create_color(10, 30, 31); // Light Blue
short int orange = create_color(31, 20, 0);      // Orange
short int pink = create_color(31, 10, 15);       // Pink
short int gray = create_color(15, 30, 15);       // Gray 0xf1ef
short int dark_green = create_color(0, 20, 0);  // Dark Green
short int sky_blue = create_color(15, 40, 31);  // Sky Blue
short int brown = create_color(20, 10, 5);      // Brown
short int dark_brown = create_color(10, 10, 5); // Dark Brown 0xaa05
*/
volatile int pixel_buffer_start;

int main(void) 
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;   // pointer to the base register of controller
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();

    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 240; y++) {
            short int brown = create_color(20, 10, 5);
            plot_pixel(x, y, brown);
        }
    }
    
}

short int create_color(int r, int g, int b) {
    return ((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F);
}
void plot_pixel(int x, int y, short int pixel_color)
{
    volatile short int *one_pixel_address;
    one_pixel_address = (volatile short int *)(pixel_buffer_start + (y * 512 + x) * 2);

    *one_pixel_address = pixel_color;
}
void clear_screen() 
{
    int y, x;
    for(x = 0; x < 320; x++)
        for(y = 0; y < 240; y++)
            plot_pixel(x,y,0);
        
}