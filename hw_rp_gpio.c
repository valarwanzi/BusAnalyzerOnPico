#include <stdio.h>
// #include <bcm2835.h>
#include "hw_gpio.h"

int initALL( void )
{
    // if (!bcm2835_init())
	//     return 1;
    return 0;
}

void deinitALL( void )
{
    // bcm2835_close();
    ;
}

int RP_Init(struct PIN *pin )
{
    switch(pin->pid)
    {
        case I2C:
            printf("Register Protocal[%d].\n", pin->pid);
            // bcm2835_gpio_fsel(pin->hwid, BCM2835_GPIO_FSEL_INPT);
            // bcm2835_gpio_set_pud(pin->hwid, BCM2835_GPIO_PUD_UP);
            // bcm2835_gpio_fen(pin->hwid);
            break;

        default:
            break;
    }
    return 0;
}

int RP_Listen(struct PIN *pin)
{
    // bcm2835_gpio_fsel(pin->state, BCM2835_GPIO_FSEL_INPT);
    switch( pin->state & 0x03 )
    {
        case 1: // Low->Rise
            // if (bcm2835_gpio_eds(pin->hwid))
            // {
            //     bcm2835_gpio_set_eds(pin->hwid);
            //     bcm2835_gpio_fen(pin->hwid);
            //     pin->state &= 0xf3; // clear:1111 0011
            //     pin->state |= 0x08; // fill: 0000 1000
            // }
            break;
        
        case 2: // High->Fall
            // if (bcm2835_gpio_eds(pin->hwid))
            // {
            //     bcm2835_gpio_set_eds(pin->hwid);
            //     bcm2835_gpio_ren(pin->hwid);
            //     pin->state &= 0xf3; // clear:1111 0011
            //     pin->state |= 0x04; // fill: 0000 0100
            // }
            break;

        default:
            break;
    }
    return 0;
}

int RP_Read(struct PIN *pin)
{
    printf("Protocol[%d], Signal[%d], Read.\n", pin->pid, pin->pfunc);
    pin->state &= 0xfc; // Clear: 1111 1100
    // pin->state |= (bcm2835_gpio_lev(pin->hwid)? 0x02: 0x01);
    return 0;
}

int RP_Write(struct PIN *pin, int value)
{
    printf("Protocol[%d], Signal[%d], Write[%d].\n", pin->pid, pin->pfunc, value);
    // bcm2835_gpio_fsel(pin->hwid, BCM2835_GPIO_FSEL_OUTP);
	// bcm2835_gpio_write(pin->hwid, (value ? HIGH : LOW));
    return 0;
}
