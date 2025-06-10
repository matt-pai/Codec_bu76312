#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/stat.h>
#include <linux/spi/spi.h>	


struct bu76312_dev {
	unsigned gpio_power;
};

struct bu76312_reg {
	u8 addr;
	u8 value;
};

static struct bu76312_reg bu76312_regs[] = {
	{0x00, 0x30},
	{0x01, 0x0E},
	{0x03, 0x01},
	{0x04, 0x16},
	{0x05, 0x25},
	{0x06, 0x18},
	{0x07, 0x00},
	{0x08, 0x51},
	{0x09, 0xFF},
	{0x0A, 0x48},
	{0x0B, 0x89},
	{0x0C, 0x00},
	{0x0D, 0x00},
	{0x0E, 0x1C},
	{0x0F, 0x27},
	{0x10, 0x10},
	{0x12, 0x00},
	{0x13, 0x00},
	{0x17, 0x07},
	{0x18, 0x70},
	{0x19, 0xB8},
	{0x1A, 0x0F},
	{0x1B, 0x1A},
	{0x1C, 0x35},
	{0x1D, 0x50},
	{0x1E, 0x02},
	{0x1F, 0xA8},
	{0x20, 0x35},
	{0x21, 0x50},
	{0x22, 0x02},
	{0x23, 0xA8},
	{0x24, 0xC4},
	{0x25, 0x36},
	{0x26, 0x33},
	{0x27, 0x32},
	{0x28, 0xC4},
	{0x29, 0x36},
	{0x2A, 0x36},
	{0x2B, 0xF3},
	{0x2C, 0x77},
	{0x2D, 0x77},
	{0x2E, 0x70},
	{0x2F, 0x00},
};

static int bu76312_write_reg(struct spi_device *spi_dev, u8 reg, u8 val)
{
    u8 tx[2];
    tx[0] = reg | 0x80;
    tx[1] = val;
    return spi_write(spi_dev, tx, 2);
}

static int bu76312_init(struct spi_device *spi)
{
	struct bu76312_reg *regs;
	u32 i, num_regs;
	
	regs = bu76312_regs;
	num_regs = ARRAY_SIZE(bu76312_regs);
	
	for (i = 0; i < num_regs; i++) {
		if (bu76312_write_reg(spi, regs[i].addr, regs[i].value) < 0) {
			pr_err("bu76312 write reg fail\n");
			return -1;
		}
	}
	return 0;
}

static int bu76312_probe(struct spi_device *spi)
{
	struct bu76312_dev *bu76312;
	unsigned gpio_power = -EINVAL;
	 
	bu76312 = devm_kzalloc(&spi->dev, sizeof(*bu76312), GFP_KERNEL);
	if (!bu76312) {
		pr_err("allocate memory fail\n");
		return -ENOMEM;
	}
	
    pr_info("bu76312 using spi%d\n", spi->controller->bus_num);

#ifdef CONFIG_OF
    if (of_property_read_u32(spi->dev.of_node, "gpio-power", &gpio_power)) {
        pr_err("get gpio-power fail\n");
        return -EINVAL;
    }
#endif

	//if ( !gpio_is_valid(gpio_power) ) {
	//	pr_err("invalid gpio: %d\n", gpio_power);
	//	return -EINVAL;
	//}
	gpio_request(gpio_power, "gpio_out");
	gpio_direction_output(gpio_power, 1);

    spi->bits_per_word = 16;
    spi->mode = SPI_MODE_0;
	spi_setup(spi);

    pr_info("mode %d, bits %d, speed %d\n", spi->mode, spi->bits_per_word, spi->max_speed_hz);

	bu76312->gpio_power = gpio_power;
	spi_set_drvdata(spi, bu76312);
	
	bu76312_init(spi);
	
    return 0;
}

static int bu76312_remove(struct spi_device *spi)
{
	struct bu76312_dev *bu76312;
	
	bu76312 = spi_get_drvdata(spi);
	
	gpio_free(bu76312->gpio_power);
	
    return 0;
}

static const struct of_device_id bu76312_of_match[] = {
    { .compatible = "codec,bu76312" },
    {},
};
MODULE_DEVICE_TABLE(of, bu76312_of_match);

static struct spi_driver bu76312_driver = {
    .probe = bu76312_probe,
    .remove = bu76312_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "bu76312",
        .of_match_table = bu76312_of_match,
    },
};

module_spi_driver(bu76312_driver);

MODULE_AUTHOR("Matt");
MODULE_DESCRIPTION("Rohm bu76312 audio codec driver");
MODULE_LICENSE("GPL");
