#include <rtthread.h>
#include <rtdevice.h>

#define ARG_CMD_POS             1
#define ARG_DEV_NAME_POS        2
#define ARG_BUS_NAME_POS        3
#define ARG_CS_PIN_POS          4
#define ARG_CONFIG_PARA_POS     3
#define ARG_TRANS_PARA_POS      3

static inline int str2hex_d(char c)
{
    if(c < '0' || c > '9')
    {
        return -1;
    }
    return (c - '0');
}
static inline int str2hex_x(char c)
{
    if(c < 'a' || c > 'f')
    {
        return str2hex_d(c);
    }
    return (c - 'W'); //'W' = 'a' - 10
}
static rt_uint32_t str2hex(const char *str)
{
    volatile rt_uint32_t value = 0;
    volatile rt_uint8_t scale = 0;
    volatile int i;
    int (*volatile func)(char) = RT_NULL;
    if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        i = 2;
        func = str2hex_x;
        scale = 16;
    }
    else
    {
        i = 0;
        func = str2hex_d;
        scale = 10;
    }
    for(;str[i];i++)
    {
        int v = func(str[i]|0x20);
        if(v == -1)
        {
            break;
        }
        value *= scale;
        value += v;
    }
    return value;
}
static rt_bool_t device_type_check(rt_device_t dev, enum rt_device_class_type type)
{
    if(dev->type == type)
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}
static void spi_help(void)
{
    rt_kprintf("---------------\n");
    rt_kprintf("spi tools help:\n");
    rt_kprintf("---------------\n");

    rt_kprintf("spitools <command> device_name [options]\n");
    rt_kprintf("spitools config device_name [options]\n");
    rt_kprintf("\t-s=[speed] spi speed, default=1000000\n");
    rt_kprintf("\t-m=[0-3] spi mode, default=0\n");
    rt_kprintf("\t-l=1:lsb, 0:msb, default=0\n");
    rt_kprintf("\t-b=[value] bits per word, default=8\n");
    rt_kprintf("spitools trans device_name [options]\n");
    rt_kprintf("\t-v=[value], default=0xFF\n");
    rt_kprintf("\t-l=[value], default=1, max=512\n");
    rt_kprintf("spitools trans device_name data0 [data1...]\n");
    rt_kprintf("spitools init device_name bus_name cs_pin\n");
    rt_kprintf("spitools deinit device_name\n\n");
}
static struct rt_spi_device *spi_device = RT_NULL;
static rt_uint8_t trans_buf[512];
static void spitools(int argc,char *argv[])
{
    if(argc > 3)
    {
        const char *cmd_str = argv[ARG_CMD_POS];
        const char *dev_name = argv[ARG_DEV_NAME_POS];

        if(!rt_strcmp(cmd_str, "config"))
        {
            struct rt_spi_device *dev = (struct rt_spi_device *)rt_device_find(dev_name);
            if(dev == RT_NULL)
            {
                rt_kprintf("[spitools] cant't find device:%s\n", dev_name);
                return ;
            }
            if(!device_type_check((void *)dev, RT_Device_Class_SPIDevice))
            {
                rt_kprintf("[spitools] %s is not a spi device\n", dev_name);
                return ;
            }
            
            struct rt_spi_configuration cfg = {
                .mode       = RT_SPI_MODE_0|RT_SPI_MASTER|RT_SPI_MSB,
                .data_width = 8,
                .max_hz     = 1000000,
            };
            
            for(int i=ARG_CONFIG_PARA_POS; i<argc; i++)
            {
                const char *para = argv[i];
                if(para[0] == '-' && para[2] == '=')
                {
                    switch(para[1])
                    {
                        case 's':
                            cfg.max_hz = str2hex(&para[3]);
                            break;
                        case 'm':
                            if(para[3] == '1')
                            {
                                cfg.mode |= RT_SPI_MODE_1;
                            }
                            else if(para[3] == '2')
                            {
                                cfg.mode |= RT_SPI_MODE_2;
                            }
                            else if(para[3] == '3')
                            {
                                cfg.mode |= RT_SPI_MODE_3;
                            }
                            break;
                        case 'l':
                            if(para[3] == '1')
                            {
                                cfg.mode &= ~RT_SPI_LSB;
                            }
                            break;
                        case 'b':
                            cfg.data_width = (rt_uint8_t)str2hex(&para[3]);
                            break;
                    }
                }
            }
            
            if(RT_EOK != rt_spi_configure(dev, &cfg))
            {
                rt_kputs("[spitools] config failed\n");
                return ;
            }
        }

        else if(!rt_strcmp(cmd_str, "trans"))
        {
            int i;
            int trans_len = 1;
            rt_uint8_t value = 0xFF;
            struct rt_spi_device *dev = (struct rt_spi_device *)rt_device_find(dev_name);
            if(dev == RT_NULL)
            {
                rt_kprintf("[spitools] cant't find device:%s\n", dev_name);
                return ;
            }
            if(!device_type_check((void *)dev, RT_Device_Class_SPIDevice))
            {
                rt_kprintf("[spitools] %s is not a spi device\n", dev_name);
                return ;
            }
            
            for(i=ARG_TRANS_PARA_POS; i<argc; i++)
            {
                const char *para = argv[i];
                if(para[0] == '-' && para[2] == '=')
                {
                    switch(para[1])
                    {
                        case 'v':
                            value = (rt_uint8_t)str2hex(&para[3]);
                            break;
                        case 'l':
                            trans_len = (int)str2hex(&para[3]);
                            break;
                    }
                }
                else
                {
                    break;
                }
            }
            
            if((i == ARG_TRANS_PARA_POS) && (i < argc))
            {
                trans_len = 0;
                for(; i<argc; i++)
                {
                    trans_buf[trans_len++] = (rt_uint8_t)str2hex(argv[i]);
                }
            }
            else
            {
                for(i=0; i<trans_len; i++)
                {
                    trans_buf[i] = value;
                }
            }
            
            rt_size_t ret = rt_spi_transfer(dev, trans_buf, trans_buf, (rt_uint32_t)trans_len);
            if(ret < 0)
            {
                rt_kputs("[spitools] trans failed\n");
                return ;
            }
            rt_kprintf("recv:%d,[", ret);
            for(i=0; i<trans_len; i++)
            {
                if(i)
                    rt_kprintf(" %02X", trans_buf[i]);
                else
                    rt_kprintf("%02X", trans_buf[i]);
            }
            rt_kputs("]\n");
        }

        else if(!rt_strcmp(cmd_str, "init"))
        {
            const char *bus_name = argv[ARG_BUS_NAME_POS];
            int cs_pin = (int)str2hex(argv[ARG_CS_PIN_POS]);
            if(spi_device == RT_NULL)
            {
                spi_device = rt_malloc(sizeof(struct rt_spi_device));
                if(spi_device == RT_NULL)
                {
                    rt_kputs("[spitools] create failed\n");
                    return ;
                }
            }
            if(RT_EOK != rt_spi_bus_attach_device(spi_device, dev_name, bus_name, &cs_pin))
            {
                rt_kputs("[spitools] attach failed!\n");
                return ;
            }
        }
    }
    else
    {
        const char *cmd_str = argv[ARG_CMD_POS];
        const char *dev_name = argv[ARG_DEV_NAME_POS];
        if(!rt_strcmp(cmd_str, "deinit"))
        {
            if(argc == 3)
            {
                rt_device_t dev = rt_device_find(dev_name);
                if(dev != RT_NULL)
                {
                    if(!device_type_check((void *)dev, RT_Device_Class_SPIDevice))
                    {
                        rt_kprintf("[spitools] %s is not a spi device\n", dev_name);
                        return ;
                    }
                    if(RT_EOK != rt_device_unregister(dev))
                    {
                        rt_kputs("[spitools] unregister failed\n");
                    }
                    return ;
                }
                rt_kprintf("[spitools] cant't find device:%s\n", dev_name);
                return ;
            }
        }
        spi_help();
    }
}
MSH_CMD_EXPORT(spitools, spi tools);
