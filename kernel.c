#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__linux__)
#error "You are not using a cross compiler, this will almost certainly lead to issues"
#endif

#if !defined(__i386__) && !defined(__i686__)
#error "You need to compile this project with a ix86-elf compiler"
#endif

enum vga_color {
	vga_color_black = 0,                  //böse
	vga_color_blue = 1,
	vga_color_greem = 2,
	vga_color_cyan = 3,
	vga_color_red = 4,
	vga_color_magenta = 5,
	vga_color_brown = 6, 	              //böse 2
	vga_color_light_grey = 7,
	vga_color_dark_grey = 8,
	vga_color_light_blue = 9,
	vga_color_light_green = 10,
	vga_color_light_cyan = 11,
	vga_color_light_red = 12,
	vga_color_light_magenta = 13,
	vga_color_light_brown = 14,           //Ein bischen weniger böse
	vga_color_white = 15,                 //Nicht böse
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str)
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

#define vga_width 80
#define vga_height 25
#define vga_memory 0xB8000

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer = (uint16_t*)vga_memory;

void terminal_initialise(void)
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(vga_color_light_grey, vga_color_black);

	for (size_t y = 0; y < vga_height; y++) {
		for (size_t x = 0; x < vga_width; x++) {
			const size_t index = y * vga_width + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color)
{
	terminal_color = color;
}

void terminal_put_entry_at(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * vga_width + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_put_char(char c)
{
	terminal_put_entry_at(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == vga_width) {
		terminal_column = 0;
		if (++terminal_row == vga_height) {
			terminal_row == 0;
		}
	}
}

void terminal_write(const char* data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		terminal_put_char(data[i]);
}

void terminal_write_string(const char* data)
{
	terminal_write(data, strlen(data));
}

void kernel_main(void)
{
	terminal_initialise();

	terminal_write_string("Hello world!\n");
}