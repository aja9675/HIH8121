#include <linux/init.h>  
#include <linux/kernel.h> 
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/i2c.h>
#include <linux/i2c-smbus.h>
#include <linux/delay.h>

MODULE_AUTHOR("Alex Avery <aja9675@rit.edu>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HIH8121 Temp/Humidity Sensor Driver");
MODULE_VERSION("0.1");

#define HIH8121_I2C_DEVICE_NAME "hih8121_i2c"

#define DEFAULT_I2C_ADDRESS 0x27
#define READ_DATA_ADDRESS 0x0


static const struct i2c_device_id hih8121_i2c_id[] = {
    { HIH8121_I2C_DEVICE_NAME, 0 }, {}
};
MODULE_DEVICE_TABLE(i2c, hih8121_i2c_id);


static int hih8121_read_raw_data(struct device *dev, u8 *data) {
    struct i2c_client *client = to_i2c_client(dev);
    int ret;

        /* Hacked 0 byte write to trigger the conversion since our smbus implementation
     * doesn't support smbus_quick_write() */
	ret = i2c_smbus_write_i2c_block_data(client, READ_DATA_ADDRESS, 0, NULL);
    if (ret) {
	    dev_err(&client->dev,"%s: i2c_smbus_write_i2c_block_data error: %i\n", __FUNCTION__, ret);
	    return ret;
    }

	/* Delay for conversion. Datasheet says 36.65 ms
	 * Could alternately poll on the status bits, but this seems to work reliably */
	mdelay(40);

	// Returns # bytes read
    ret = i2c_smbus_read_i2c_block_data(client, READ_DATA_ADDRESS, 4, data);
    if (ret < 4) {
	    dev_err(&client->dev,"%s: i2c_smbus_read_i2c_block_data error\n", __FUNCTION__);
	    ret = -EIO;
    }

	return ret;
}

static ssize_t read_temperature_f(struct device *dev, 
    struct device_attribute *dev_attr,
    char *buf)
{
    int ret, temp_raw, temp_c, temp_f;
    u8 data[4];

    ret = hih8121_read_raw_data(dev, data);
    if (ret < 0)
    	return ret;
   	
   	temp_raw = (((data[2] & 0xFF) * 256) + (data[3] & 0xFC)) / 4;
   	// Scaling up to report decidegrees without floating point math
    temp_c = ((temp_raw * 1650) >> 14) - 400; // (temp_raw / 16384.0) * 165.0 - 40.0;
    temp_f = temp_c * 18 / 10 + 320; // cTemp * 1.8 + 32

    return sprintf(buf, "%d\n", temp_f);
}

static ssize_t read_humidity(struct device *dev, 
    struct device_attribute *dev_attr,
    char *buf)
{
    int ret, humidity;
    u8 data[4];

    ret = hih8121_read_raw_data(dev, data);
    if (ret < 0)
    	return ret;
   	
    //humidity = ((((data[0] & 0x3F) * 256) + data[1]) * 100.0) / 16383.0
    humidity = ((((data[0] & 0x3F) * 256) + data[1]) * 1000) >> 14;

    return sprintf(buf, "%d\n", humidity);
}


static DEVICE_ATTR(read_temp_f, S_IRUGO, read_temperature_f, NULL);
static DEVICE_ATTR(read_humid, S_IRUGO, read_humidity, NULL);

static struct attribute *hih8121_attrs[] = {
	&dev_attr_read_temp_f.attr,
	&dev_attr_read_humid.attr,
	NULL
};

static const struct attribute_group hih8121_attr_group = {
	.attrs = hih8121_attrs,
};

static int hih8121_i2c_probe(struct i2c_client *client,
    const struct i2c_device_id *id)
{
    struct device *dev = &client->dev;
    int ret;

    dev_info(dev, "hih8121_i2c: %s\n", __FUNCTION__);

	ret = sysfs_create_group(&client->dev.kobj, &hih8121_attr_group);
	if (ret < 0) {
		dev_err(dev, "failed to create sysfs attributes\n");
		return ret;
	}

    return 0;
}

static int hih8121_i2c_remove(struct i2c_client * client)
{
    struct device *dev = &client->dev;

    dev_info(dev, "hih8121_i2c: %s\n", __FUNCTION__);

	sysfs_remove_group(&client->dev.kobj, &hih8121_attr_group);

    return 0;
}

static struct i2c_driver hih8121_i2c_driver = {
    .class      = I2C_CLASS_HWMON,
    .driver = {
            .name = HIH8121_I2C_DEVICE_NAME,
            .owner = THIS_MODULE,
    },
    .probe          = hih8121_i2c_probe,
    .remove         = hih8121_i2c_remove,
    .id_table       = hih8121_i2c_id,
};

module_i2c_driver(hih8121_i2c_driver);
