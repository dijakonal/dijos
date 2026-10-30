#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__linux__)
#error "You are not using a cross compiler, this will almost certainly lead to issues"
#endif

#if !defined(__i386__) && !defined(__i686__)
#error "You need to compile this project with a ix86-elf compiler"
#endif

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

enum vga_color {
	vga_color_black = 0,                  //böse
	vga_color_blue = 1,
	vga_color_green = 2,
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

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}


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

bool is_color_code;
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint8_t text_color;
uint8_t background_color;
uint16_t* terminal_buffer = (uint16_t*)vga_memory;

void terminal_update_cursor(void)
{
   
    uint16_t pos = terminal_row * vga_width + terminal_column;

   
    outb(0x3D4, 0x0F);          
    outb(0x3D5, (uint8_t)(pos & 0xFF));

    
    outb(0x3D4, 0x0E);          
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void terminal_scroll(void)
{
    for (size_t y = 1; y < vga_height; y++) {
        for (size_t x = 0; x < vga_width; x++) {
            size_t src_index = y * vga_width + x;
            size_t dest_index = (y - 1) * vga_width + x;
            terminal_buffer[dest_index] = terminal_buffer[src_index];
        }
    }

    size_t last_row = vga_height - 1;
    for (size_t x = 0; x < vga_width; x++) {
        size_t index = last_row * vga_width + x;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
}



void terminal_initialise(void)
{
	terminal_row = 0;
	terminal_column = 0;
	background_color = vga_color_black;
	text_color = vga_color_white;
	terminal_color = vga_entry_color(text_color, background_color);
	terminal_update_cursor();

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
	if (is_color_code == true) {
		if (c == '0')       { text_color = vga_color_black; }
		else if (c == '1')  { text_color = vga_color_blue; }
		else if (c == '2')  { text_color = vga_color_green; }
		else if (c == '3')  { text_color = vga_color_cyan; }
		else if (c == '4')  { text_color = vga_color_red; }
		else if (c == '5')  { text_color = vga_color_magenta; }
		else if (c == '6')  { text_color = vga_color_brown; }
		else if (c == '7')  { text_color = vga_color_light_grey; }
		else if (c == '8')  { text_color = vga_color_dark_grey; }
		else if (c == '9')  { text_color = vga_color_light_blue; }
		else if (c == 'a')  { text_color = vga_color_light_green; }
		else if (c == 'b')  { text_color = vga_color_light_cyan; }
		else if (c == 'c')  { text_color = vga_color_light_red; }
		else if (c == 'd')  { text_color = vga_color_light_magenta; }
		else if (c == 'e')  { text_color = vga_color_light_brown; }
		else if (c == 'f')  { text_color = vga_color_white; }

		terminal_color = vga_entry_color(text_color, background_color);
		is_color_code = false;
		return;
	}

	if (c == '$') {
		is_color_code = true;
		return;
	}

	if (c == '\b') {
		if (terminal_column > 0) {
			terminal_column--;
		} else if (terminal_row > 0) {
			terminal_row--;
			terminal_column = vga_width - 1;
		} else {
			return;
		}
		terminal_put_entry_at(' ', terminal_color, terminal_column, terminal_row);
		terminal_update_cursor();
		return;
	}

	if (c == '\n') {
		terminal_column = 0;
		terminal_row++;
	} else {
		terminal_put_entry_at(c, terminal_color, terminal_column, terminal_row);
		if (++terminal_column == vga_width) {
			terminal_column = 0;
			terminal_row++;
		}
	}

	if (terminal_row >= vga_height) {
		terminal_scroll();
		terminal_row = vga_height - 1;
	}

	terminal_update_cursor();
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

void process_keyboard_input(uint8_t scancode) {
    
if (scancode == 0x01) { terminal_write_string("[ESC]"); }
else if (scancode == 0x02) { terminal_write_string("1"); }
else if (scancode == 0x03) { terminal_write_string("2"); }
else if (scancode == 0x04) { terminal_write_string("3"); }
else if (scancode == 0x05) { terminal_write_string("4"); }
else if (scancode == 0x06) { terminal_write_string("5"); }
else if (scancode == 0x07) { terminal_write_string("6"); }
else if (scancode == 0x08) { terminal_write_string("7"); }
else if (scancode == 0x09) { terminal_write_string("8"); }
else if (scancode == 0x0A) { terminal_write_string("9"); }
else if (scancode == 0x0B) { terminal_write_string("0"); }
else if (scancode == 0x0C) { terminal_write_string("ß"); }
else if (scancode == 0x0D) { terminal_write_string("`"); }
else if (scancode == 0x0E) { terminal_write_string("\b"); }
else if (scancode == 0x0F) { terminal_write_string("$"); }
else if (scancode == 0x10) { terminal_write_string("q"); }
else if (scancode == 0x11) { terminal_write_string("w"); }
else if (scancode == 0x12) { terminal_write_string("e"); }
else if (scancode == 0x13) { terminal_write_string("r"); }
else if (scancode == 0x14) { terminal_write_string("t"); }
else if (scancode == 0x15) { terminal_write_string("z"); }
else if (scancode == 0x16) { terminal_write_string("u"); }
else if (scancode == 0x17) { terminal_write_string("i"); }
else if (scancode == 0x18) { terminal_write_string("o"); }
else if (scancode == 0x19) { terminal_write_string("p"); }
else if (scancode == 0x1A) { terminal_write_string("ü"); }
else if (scancode == 0x1B) { terminal_write_string("+"); }
else if (scancode == 0x1C) { terminal_write_string("\n"); }
else if (scancode == 0x1E) { terminal_write_string("a"); }
else if (scancode == 0x1F) { terminal_write_string("s"); }
else if (scancode == 0x20) { terminal_write_string("d"); }
else if (scancode == 0x21) { terminal_write_string("f"); }
else if (scancode == 0x22) { terminal_write_string("g"); }
else if (scancode == 0x23) { terminal_write_string("h"); }
else if (scancode == 0x24) { terminal_write_string("j"); }
else if (scancode == 0x25) { terminal_write_string("k"); }
else if (scancode == 0x26) { terminal_write_string("l"); }
else if (scancode == 0x27) { terminal_write_string("ö"); }
else if (scancode == 0x28) { terminal_write_string("ä"); }
else if (scancode == 0x29) { terminal_write_string("#"); }
else if (scancode == 0x2B) { terminal_write_string("-"); }
else if (scancode == 0x2C) { terminal_write_string("y"); }
else if (scancode == 0x2D) { terminal_write_string("x"); }
else if (scancode == 0x2E) { terminal_write_string("c"); }
else if (scancode == 0x2F) { terminal_write_string("v"); }
else if (scancode == 0x30) { terminal_write_string("b"); }
else if (scancode == 0x31) { terminal_write_string("n"); }
else if (scancode == 0x32) { terminal_write_string("m"); }
else if (scancode == 0x33) { terminal_write_string(","); }
else if (scancode == 0x34) { terminal_write_string("."); }
else if (scancode == 0x35) { terminal_write_string("-"); }
else if (scancode == 0x39) { terminal_write_string(" "); }
else if (scancode == 0x56) { terminal_write_string("<"); } 
}


void kernel_main(void)
{
    terminal_initialise();

    for (int i = 0; i < 256; i++) {
        if ((inb(0x64) & 0x01) == 0) {
            break;
        }
        inb(0x60);
	}

    while (1) {
        uint8_t status = inb(0x64);
        
        if (status & 0x01) {
            uint8_t scancode = inb(0x60);
            process_keyboard_input(scancode);
        }
    }
}
