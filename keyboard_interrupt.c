#include "address_map_niosv.h"
/* function prototypes */
void HEX_PS2(char, char, char);
void PS2_ISR(void);
void set_PS2(void);
void handler(void);

int main(void)
{
    //interrupt setup
    volatile int *mtime_ptr = (int *)MTIME_BASE;

    set_PS2();

    volatile int *PS2_ptr = (int *)PS2_BASE;
    int PS2_data, RVALID;
    char byte1 = 0, byte2 = 0, byte3 = 0;
    // PS/2 mouse needs to be reset (must be already plugged in)
    *(PS2_ptr) = 0xFF; // reset

    //interrupt setup
    int mstatus_value, mtvec_value, mie_value;
    mstatus_value = 0b1000; // interrupt bit mask
    // disable interrupts
    __asm__ volatile("csrc mstatus, %0" ::"r"(mstatus_value));
    mtvec_value = (int)&handler; // set trap address
    __asm__ volatile("csrw mtvec, %0" ::"r"(mtvec_value));
    // disable all interrupts that are currently enabled
    __asm__ volatile("csrr %0, mie" : "=r"(mie_value));
    __asm__ volatile("csrc mie, %0" ::"r"(mie_value));
    mie_value = 0x410088; // PS/2 keyboard, itimer, mtimer interrupts
    // set interrupt enables
    __asm__ volatile("csrs mie, %0" ::"r"(mie_value));
    // enable Nios V interrupts
    __asm__ volatile("csrs mstatus, %0" ::"r"(mstatus_value));

    *(mtime_ptr + 4) = 1; // cause a software interrupt

    while (1)
    {
        // PS2_data = *(PS2_ptr);      // read the Data register in the PS/2 port
        // RVALID = PS2_data & 0x8000; // extract the RVALID field
        // if (RVALID)
        // {
        //     /* shift the next data byte into the display */
        //     byte1 = byte2;
        //     byte2 = byte3;
        //     byte3 = PS2_data & 0xFF;
        //     HEX_PS2(byte1, byte2, byte3);
        //     if ((byte2 == (char)0xAA) && (byte3 == (char)0x00))
        //         // mouse inserted; initialize sending of data
        //         *(PS2_ptr) = 0xF4;
        // }
    }
}

// Interrupt handler
void handler(void)
{
    int mcause_value;
    __asm__ volatile("csrr %0, mcause" : "=r"(mcause_value));

    if (mcause_value == 0x80000016) // PS2 port
        PS2_ISR();
    // else, ignore the trap
}

void HEX_PS2(char b1, char b2, char b3)
{
    volatile int *HEX3_HEX0_ptr = (int *)HEX3_HEX0_BASE;
    volatile int *HEX5_HEX4_ptr = (int *)HEX5_HEX4_BASE;
    /* SEVEN_SEGMENT_DECODE_TABLE gives the on/off settings for all segments in
     * a single 7-seg display in the DE1-SoC Computer, for the hex digits 0 - F
     */
    unsigned char seven_seg_decode_table[] = {
        0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
        0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
    unsigned char hex_segs[] = {0, 0, 0, 0, 0, 0, 0, 0};
    unsigned int shift_buffer, nibble;
    unsigned char code;
    int i;
    shift_buffer = (b1 << 16) | (b2 << 8) | b3;
    for (i = 0; i < 6; ++i)
    {
        nibble = shift_buffer & 0x0000000F; // character is in rightmost nibble
        code = seven_seg_decode_table[nibble];
        hex_segs[i] = code;
        shift_buffer = shift_buffer >> 4;
    }
    /* drive the hex displays */
    *(HEX3_HEX0_ptr) = *(int *)(hex_segs);
    *(HEX5_HEX4_ptr) = *(int *)(hex_segs + 4);
}

// Configure the PS2 port
void set_PS2(void)
{
    volatile int *PS2_ptr = (int *)PS2_BASE;
    //*(PS2_ptr + 3) = 0xF; // clear EdgeCapture register
    *(PS2_ptr + 1) = 0x1; // enable interrupts for PS2 keyboard
}

void PS2_ISR(void)
{
    volatile int *PS2_ptr = (int *)PS2_BASE;
    int PS2_data, RVALID;
    char byte1 = 0, byte2 = 0, byte3 = 0;
    PS2_data = *(PS2_ptr);      // read the Data register in the PS/2 port
    RVALID = PS2_data & 0x8000; // extract the RVALID field
    if (RVALID)
    {
        /* shift the next data byte into the display */
        byte1 = byte2;
        byte2 = byte3;
        byte3 = PS2_data & 0xFF;
        HEX_PS2(byte1, byte2, byte3);
        if ((byte2 == (char)0xAA) && (byte3 == (char)0x00))
            // mouse inserted; initialize sending of data
            *(PS2_ptr) = 0xF4;
    }
}