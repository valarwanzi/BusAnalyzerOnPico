#include <bcm2835.h>
#include <stdio.h>

#define SLAVE_SDA RPI_V2_GPIO_P1_15
#define SLAVE_SCL RPI_V2_GPIO_P1_16

typedef enum{
    START = 0,
    ADDRLISTENING,
    REGLISTENING,
    DATALISTENING,
    ACKNAK,
    SENDING,
    BUSY,
    STOP
} I2C_STATE;

static I2C_STATE status = START; // TODO:并入从机结构体

enum{
    I2C_WRITE,
    I2C_READ
};
static uint8_t work = I2C_WRITE; // TODO:并入从机结构体
static uint8_t waitReg = 1;

#define I2C_ID 0x75
#define TEST_DATA 0x48
void switchIO_SM(uint8_t st);
void configIO_SM(uint8_t st);

static int adderessAnalyse(uint8_t indata)
{
    printf("Input address: %d\n", ((indata & 0xFE) >> 1));
    printf("Input mode: %d\n", (indata & 0x01));
    if( ((indata & 0xFE) >> 1) == I2C_ID )
    {
        switch (indata & 0x01)
        {
        case I2C_WRITE:
            work = I2C_WRITE;
            status = SENDING;
            break;

        case I2C_READ:
            work = I2C_READ;
            if (waitReg == 1)
                status = REGLISTENING;
            else
                status = DATALISTENING;
            break;

        default:
            break;
        }
        configIO_SM(ACKNAK);
        switchIO_SM(ACKNAK);
    }
    else{
        status = STOP;
    }
    return 0;
}

static int registerAnalyse(uint8_t indata)
{
    printf("Input register: %d\n", indata);
    if( indata == I2C_ID )
    {
        status = DATALISTENING;
        waitReg = 0;
        configIO_SM(ACKNAK);
        switchIO_SM(ACKNAK);
    }
    return 0;
}

static int dataAnalyse(uint8_t indata)
{
    printf("Input data: %d\n", indata);
    status = STOP;
    waitReg = 1;
    configIO_SM(ACKNAK);
    switchIO_SM(ACKNAK);
    return 0;
}

static int i2c_outb(unsigned char c) {
    int cnt = 7;
    int sb;

    while (1) {
        if ( bcm2835_gpio_eds(SLAVE_SCL) && bcm2835_gpio_lev(SLAVE_SCL))
        {
            bcm2835_gpio_set_eds(SLAVE_SCL);
            bcm2835_gpio_fsel(SLAVE_SCL, BCM2835_GPIO_FSEL_OUTP);
            bcm2835_gpio_write(SLAVE_SCL, LOW);
            sb = ( c >> cnt ) & 1;
            bcm2835_gpio_write(SLAVE_SDA, (sb ? HIGH : LOW));
            bcm2835_gpio_write(SLAVE_SCL, HIGH);
            bcm2835_gpio_fsel(SLAVE_SCL, BCM2835_GPIO_FSEL_INPT);
            bcm2835_gpio_fen(SLAVE_SCL);
            cnt--;
        }
        if (cnt < 0) break;
    }
    return 0;
}

static int i2c_inb(void){ // TODO: 从机样式，用结构体链表挂载
    uint8_t indata = 0;
    uint8_t cnt = 0;
    while(1) {
        // TODO: 超时恢复机制
        if (bcm2835_gpio_eds(SLAVE_SCL) && bcm2835_gpio_lev(SLAVE_SCL))
        {
            bcm2835_gpio_set_eds(SLAVE_SCL);
            indata *= 2;
            if(bcm2835_gpio_lev(SLAVE_SDA))
                indata |= 0x01;
            cnt++;
        }
        if (cnt == 8) break;
    }
    return indata;
}

static int i2c_inb_with_sr(void){ // TODO: 从机样式，用结构体链表挂载
    uint8_t indata = 0;
    uint8_t cnt = 0;
    while(1) {
        // TODO: 超时恢复机制
        // 捕获 SR 信号
        bcm2835_gpio_fen(SLAVE_SDA);
        if (bcm2835_gpio_eds(SLAVE_SDA) && bcm2835_gpio_lev(SLAVE_SCL)) {
            bcm2835_gpio_set_eds(SLAVE_SDA);
            status = ADDRLISTENING;
            printf("SR triggered.\n");
            return -1;
        }
        // 捕获 STOP 信号
        bcm2835_gpio_ren(SLAVE_SDA);
        if (bcm2835_gpio_eds(SLAVE_SDA) && bcm2835_gpio_lev(SLAVE_SCL)) {
            bcm2835_gpio_set_eds(SLAVE_SDA);
            status = START;
            printf("P triggered.\n");
            return -1;
        }
        // 解析字节数据
        if (bcm2835_gpio_eds(SLAVE_SCL) && bcm2835_gpio_lev(SLAVE_SCL))
        {
            bcm2835_gpio_set_eds(SLAVE_SCL);
            indata *= 2;
            if(bcm2835_gpio_lev(SLAVE_SDA))
                indata |= 0x01;
            cnt++;
        }
        if (cnt == 8) break;
    }
    return indata;
}

void configIO_SM(uint8_t st)
{
    switch (st)
    {
    case START:
        bcm2835_gpio_fsel(SLAVE_SCL, BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_set_pud(SLAVE_SCL, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_fsel(SLAVE_SDA, BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_set_pud(SLAVE_SDA, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_fen(SLAVE_SDA);
        break;

    case ADDRLISTENING:
    case REGLISTENING:
    case DATALISTENING:
        bcm2835_gpio_fsel(SLAVE_SCL, BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_set_pud(SLAVE_SCL, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_ren(SLAVE_SCL);
        bcm2835_gpio_fsel(SLAVE_SDA, BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_set_pud(SLAVE_SDA, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_fen(SLAVE_SDA);
        break;

    case ACKNAK:
    case SENDING:
        bcm2835_gpio_fsel(SLAVE_SCL, BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_set_pud(SLAVE_SCL, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_fen(SLAVE_SCL);
        bcm2835_gpio_fsel(SLAVE_SDA, BCM2835_GPIO_FSEL_OUTP);
        break;

    case STOP:
        bcm2835_gpio_fsel(SLAVE_SCL, BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_set_pud(SLAVE_SCL, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_fsel(SLAVE_SDA, BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_set_pud(SLAVE_SCL, BCM2835_GPIO_PUD_UP);
        break;

    default:
        break;
    }
}

void switchIO_SM(uint8_t st)
{
    uint8_t indata = 0;
    switch (st)
    {
    case START:
        while(1){
            if (bcm2835_gpio_eds(SLAVE_SDA) && bcm2835_gpio_lev(SLAVE_SCL))
	        {
	            bcm2835_gpio_set_eds(SLAVE_SDA);
	            printf("I2C START.\n");
                status = ADDRLISTENING;
                break;
	        }
        }
        configIO_SM(status);
        break;
    
    case ADDRLISTENING:
        printf("I2C ADDRESS DETECTING.\n");
        indata = i2c_inb();
        printf("Recv data: %d\n", indata);
        adderessAnalyse(indata);
        configIO_SM(status);
        break;
    
    case REGLISTENING:
        printf("I2C REG DETECTING.\n");
        // TODO: 大小端
        indata = i2c_inb();
        printf("Recv reg: %d\n", indata);
        registerAnalyse(indata);
        configIO_SM(status);
        break;

    case DATALISTENING:
        printf("I2C DATA DETECTING.\n");
        // TODO: 大小端
        indata = i2c_inb_with_sr();
        printf("Recv data: %d\n", indata);
        dataAnalyse(indata);
        configIO_SM(status);
        break;

    case ACKNAK:
        while (1) {
            if (bcm2835_gpio_eds(SLAVE_SCL) && !bcm2835_gpio_lev(SLAVE_SCL))
            {
                bcm2835_gpio_set_eds(SLAVE_SCL);
                bcm2835_gpio_fsel(SLAVE_SCL, BCM2835_GPIO_FSEL_OUTP);
                bcm2835_gpio_write(SLAVE_SCL, LOW);
                bcm2835_gpio_write(SLAVE_SDA, LOW);
                bcm2835_gpio_write(SLAVE_SCL, HIGH);
                bcm2835_gpio_fsel(SLAVE_SCL, BCM2835_GPIO_FSEL_INPT);
                bcm2835_gpio_set_pud(SLAVE_SCL, BCM2835_GPIO_PUD_UP);
                bcm2835_gpio_fen(SLAVE_SCL);
                break;
            }
        }
        break;

    case SENDING:
        printf("I2C DATA SENDING.\n");
        waitReg = 1;
        status = STOP;
        configIO_SM(status);
        break;

    case STOP:
        printf("I2C DATA END.\n");
        // 恢复 I2C 状态
        waitReg = 1;
        // 捕获 SR 信号
        bcm2835_gpio_fen(SLAVE_SDA);
        if (bcm2835_gpio_eds(SLAVE_SDA) && bcm2835_gpio_lev(SLAVE_SCL)) {
            bcm2835_gpio_set_eds(SLAVE_SDA);
            status = ADDRLISTENING;
        }
        // 捕获 STOP 信号
        bcm2835_gpio_ren(SLAVE_SDA);
        if (bcm2835_gpio_eds(SLAVE_SDA) && bcm2835_gpio_lev(SLAVE_SCL)) {
            bcm2835_gpio_set_eds(SLAVE_SDA);
            status = START;
        }
        configIO_SM(status);
        break;

    default:
        break;
    }
}

int main(int *argv, char **argc)
{
    static int cnt = 0;

    if(!bcm2835_init())
        return 0;

    status = START;
    // TODO: 根据传入，配置信号线空闲电平
    configIO_SM(status);
    printf("Start from %d\n", status);
    while(cnt < 10)
    {
        switchIO_SM(status);
        if ( status == STOP )
            cnt++;
        bcm2835_delayMicroseconds(1);
    }

    bcm2835_close();
    return 0;
}
