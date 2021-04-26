#include <stdio.h>
#include "hw_rp_gpio.h"

struct PIN sda = {
    .hwid = I2C_SLAVE_SDA,
    .pid = I2C,
    .pfunc = SDA,
    .state = 0,
    .init = RP_Init,
    .listen = RP_Listen,
    .read = RP_Read,
    .write = RP_Write
};

struct PIN scl = {
    .hwid = I2C_SLAVE_SCL,
    .pid = I2C,
    .pfunc = SCL,
    .state = 0,
    .init = RP_Init,
    .listen = RP_Listen,
    .read = RP_Read,
    .write = RP_Write
};

typedef struct {
    unsigned char type;
    unsigned char state;
    unsigned char signal;
    unsigned char address;
    unsigned char cmd;
    // struct PIN pins[2];
    // unsigned char *regMap;
    // unsigned char *dataMap;
}POTOCOL;

POTOCOL g_protocol = {
    .type = I2C,
    .state = 0,
    .signal = 0,
    .address = 0x25,
    .cmd = 0
    // .pins = {sda, scl},
    // .regMap = {0},
    // .dataMap = {0}
};

void APP_I2cInit(POTOCOL *protocol)
{
    scl.init(&scl);
    sda.init(&sda);
}

void APP_I2cSignalAnalyse(POTOCOL *protocol)
{
    static char state = 0;
    static char cnt = 0;
    static char bs = 0;
    sda.listen(&sda);sda.read(&sda);
    scl.listen(&scl);scl.read(&scl);
    // printf("Get SCL[%d] and SDA[%d].\n", *(g_protocol.pins+1).pfunc, *(g_protocol.pins).pfunc);
    // printf("Get SCL[%d] and SDA[%d].\n", scl.pfunc, sda.pfunc);
    /*
    if ( (sda.state & 12) == 4 && (scl.state & 3) == 2) {
        printf("I2C Signal [S] or [Sr].\n");
    } else if ((sda.state & 12) == 8  && (scl.state & 3) == 1) {
        printf("I2C Signal [P].\n");
    } else if ((scl.state & 12) == 8) {
        // bs *= 2;
        // bs |= (scl.state & 3);
        // cnt--;
        // if( cnt == 0) {
        //     printf("I2C Signal [DR]:[%d].\n", bs);
        //     cnt = 8;
        //     bs = 0;
        // }
        printf("I2C Signal [DR]:[%d].\n", (sda.state & 3) - 1);
    } else if ((scl.state & 12) == 4) {
        printf("I2C Signal [DS].\n");
    } // else if ( (scl.state & 12) || (sda.state & 12) ) { 
    //     printf("I2C Signal [ERR][TRIGGER]:SCL[%d],SDA[%d].\n", ((scl.state & 12) >> 2) - 1, ((sda.state & 12) >> 2) - 1); 
    // }
    */
    printf("Signal [TRIGGER]:SCL[%d].\n", ((scl.state & 12) >> 2) - 1);
    if ( (((scl.state & 12) >> 2) - 1) >= 0 && (((scl.state & 12) >> 2) - 1) <= 1) {
        (((scl.state & 12) >> 2) - 1) ? bs++: cnt++;
        printf("Signal [TRIGGER]:SCL[UP][%d].\n",bs);
        printf("Signal [TRIGGER]:SDA[DN][%d].\n",cnt);
    } else {
        printf("Signal [ERROR].\n");
    }
    sda.state &= 0xf3;
    scl.state &= 0xf3;
}

void i2cAnalyse()
{
    ;
}


int main( int *argv, char **argc)
{
    // initAll();
    if (!bcm2835_init())
	    return 1;

    APP_I2cInit(&g_protocol);
    // sda.init(&sda);
    // sda.read(&sda);
    // sda.write(&sda, 1);

    while(1)
    {
        APP_I2cSignalAnalyse(&g_protocol);
    }
    // deinitAll();
    bcm2835_close();
    return 0;
}
