#ifndef _HA_GPIO_H
#define _HA_GPIO_H

// #define I2C_SLAVE_SDA RPI_V2_GPIO_P1_15
// #define I2C_SLAVE_SDA RPI_V2_GPIO_P1_16

enum {
    PROTOCOL_NONE=0,
    I2C
};

enum {
    SDA = 0,
    SCL
};

struct PIN {
    unsigned char hwid;
    unsigned char pid : 4;      // I2C:1
    unsigned char pfunc : 4;    // SDA:1, SCL:2
    unsigned int state; // 0: low; 2: high; 4: fall; 8: rise
    int(*init)(struct PIN *);
    int(*listen)(struct PIN *);
    int(*read)(struct PIN *);
    int(*write)(struct PIN *, int);
};

int initAll( void );
void deinitAll( void );
int RP_Init(struct PIN *);
int RP_Listen(struct PIN *);
int RP_Read(struct PIN *);
int RP_Write(struct PIN *, int);

#endif
