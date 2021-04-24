#include "hw_gpio.h"

struct PIN sda = {
    .hwid = 1,
    .pid = I2C,
    .pfunc = SDA,
    .state = 0,
    .init = RP_Init,
    .listen = RP_Listen,
    .read = RP_Read,
    .write = RP_Write
};

void i2cAnalyse()
{
    ;
}


int main( int *argv, char **argc)
{
    sda.init(&sda);
    sda.read(&sda);
    sda.write(&sda, 1);
    return 0;
}
