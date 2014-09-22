#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/tlv.h>
#include <sound/initval.h>

#include <sound/pt2348.h>

MODULE_AUTHOR("Alexey Hohlov <root@amper.me>");
MODULE_DESCRIPTION("pt2348 driver");
MODULE_LICENSE("GPL");

static int index[SNDRV_CARDS] = SNDRV_DEFAULT_IDX;	/* Index 0-MAX */
static char *id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;	/* ID for this card */
static int enable[SNDRV_CARDS] = SNDRV_DEFAULT_ENABLE_PNP;

static int debug;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Debug level (0-1)");






static int __devinit snd_pt2348_reset(struct snd_pt2348 *pt)
{

	unsigned char bytes[2];
//	int i;
	int status;
	printk(KERN_INFO "PT2348 reset\n");

	// reset chip
	bytes[0] = PT2348_ADDR;
	bytes[1] = PT2348_CMD_RESET;
	bytes[2] = PT2348_CMD_RESET;
	status = i2c_master_send(pt->client, bytes, 3);
	printk(KERN_INFO "reset status: %i\n", status);
	if (status != 3)
		goto __error;


	// set input to 1, 0db gain all
	pt->input = 1;

	pt->gain[0] = 0b00011000;
	pt->gain[1] = 0b00011000;
	pt->gain[2] = 0b00011000;
	pt->gain[3] = 0b00011000;

	bytes[0] = PT2348_ADDR;
	bytes[1] = PT2348_INPUT_CMD;
	bytes[2] = PT2348_INPUT_MASK | pt->gain[pt->input] | pt->input;
	status = i2c_master_send(pt->client, bytes, 3);
	printk(KERN_INFO "input status: %i\n", status);
	if (status != 3)
		goto __error;


	// set LOUD to 0db
	pt->loudness = 0b11110000;
	bytes[0] = PT2348_ADDR;
	bytes[1] = PT2348_LOUDNESS_CMD;
	bytes[2] = pt->loudness;
	status = i2c_master_send(pt->client, bytes, 3);
	if (status != 3)
		goto __error;


	// volume
	pt->volume = 0b01000000;
	bytes[0] = PT2348_ADDR;
	bytes[1] = PT2348_VOLUME_CMD;
	bytes[2] = pt->volume;
	status = i2c_master_send(pt->client, bytes, 3);
	if (status != 3)
		goto __error;


	// bass, treble
	pt->tone[0] = 0b00001111;
	pt->tone[1] = 0b00001111;
	bytes[0] = PT2348_ADDR;
	bytes[1] = PT2348_TONE_CMD;
	bytes[2] = pt->tone[0] << 4 | pt->tone[1];
	status = i2c_master_send(pt->client, bytes, 3);
	if (status != 3)
		goto __error;


	// attenuators
	pt->channels[0] = 0b11100000;
	pt->channels[1] = 0b11100000;
	pt->channels[2] = 0b11100000;
	pt->channels[3] = 0b11100000;

	bytes[0] = PT2348_ADDR;
	bytes[1] = PT2348_ATT_LF_CMD;
	bytes[2] = pt->channels[0];
//	snd_i2c_lock(pt->i2c_bus);
//	if (snd_i2c_sendbytes(pt->i2c_dev, bytes, 2) != 1)
//		goto __error;
//	snd_i2c_unlock(pt->i2c_bus);
	status = i2c_master_send(pt->client, bytes, 3);
	if (status != 3)
		goto __error;


	bytes[0] = PT2348_ATT_LR_CMD;
	bytes[1] = pt->channels[1];
//	snd_i2c_lock(pt->i2c_bus);
//	if (snd_i2c_sendbytes(pt->i2c_dev, bytes, 2) != 1)
//		goto __error;
//	snd_i2c_unlock(pt->i2c_bus);
	status = i2c_master_send(pt->client, bytes, 2);
	if (status != 2)
		goto __error;


	bytes[0] = PT2348_ATT_RF_CMD;
	bytes[1] = pt->channels[2];
//	snd_i2c_lock(pt->i2c_bus);
//	if (snd_i2c_sendbytes(pt->i2c_dev, bytes, 2) != 1)
//		goto __error;
//	snd_i2c_unlock(pt->i2c_bus);
	status = i2c_master_send(pt->client, bytes, 2);
	if (status != 2)
		goto __error;


	bytes[0] = PT2348_ATT_RR_CMD;
	bytes[1] = pt->channels[3];
//	snd_i2c_lock(pt->i2c_bus);
//	if (snd_i2c_sendbytes(pt->i2c_dev, bytes, 2) != 1)
//		goto __error;
//	snd_i2c_unlock(pt->i2c_bus);
	status = i2c_master_send(pt->client, bytes, 2);
	if (status != 2)
		goto __error;



	// mute
	pt->mute = 0b00000000;
	bytes[0] = PT2348_MUTE_CMD;
	bytes[1] = pt->mute;
//	snd_i2c_lock(pt->i2c_bus);
//	if (snd_i2c_sendbytes(pt->i2c_dev, bytes, 2) != 1)
//		goto __error;
//	snd_i2c_unlock(pt->i2c_bus);
	status = i2c_master_send(pt->client, bytes, 2);
	if (status != 2)
		goto __error;


	return 0;

      __error:
//	snd_i2c_unlock(pt->i2c_bus);
	snd_printk(KERN_ERR "PT2348 reset failed\n");
	return -EIO;
}













/*	INFO CALLBACKS		*/

static int pt2348_input_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 3;
	return 0;
}

static int pt2348_gain_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 3;
	return 0;
}

static int pt2348_loudness_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 15;
	return 0;
}

static int pt2348_volume_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 255;
	return 0;
}

static int pt2348_tone_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 15;
	return 0;
}

static int pt2348_attenuation_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 31;
	return 0;
}

static int pt2348_mute_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}


/*	GET CALLBACKS		*/

static int pt2348_input_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_pt2348 *pt = kcontrol->private_data;

	ucontrol->value.integer.value[0] = pt->input;;
	return 0;
}

static int pt2348_gain_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_pt2348 *pt = kcontrol->private_data;
	int base = kcontrol->private_value;

	ucontrol->value.integer.value[0] = 4 - pt->gain[base];
	return 0;
}

static int pt2348_loudness_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_pt2348 *pt = kcontrol->private_data;

	ucontrol->value.integer.value[0] = 16 - pt->loudness;
	return 0;
}

static int pt2348_volume_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_pt2348 *pt = kcontrol->private_data;

	ucontrol->value.integer.value[0] = 255 - pt->loudness;
	return 0;
}

static int pt2348_tone_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_pt2348 *pt = kcontrol->private_data;
	int base = kcontrol->private_value;

	ucontrol->value.integer.value[0] = 16 - pt->tone[base];
	return 0;
}

static int pt2348_attenuation_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_pt2348 *pt = kcontrol->private_data;
	int base = kcontrol->private_value;

	ucontrol->value.integer.value[0] = 32 - pt->channels[base * 2];
	ucontrol->value.integer.value[1] = 32 - pt->channels[base * 2 + 1];
	return 0;
}

static int pt2348_mute_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_pt2348 *pt = kcontrol->private_data;

	ucontrol->value.integer.value[0] = pt->mute;
	return 0;
}

/*	PUT CALLBACKS		*/

static int pt2348_input_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_pt2348 *pt = kcontrol->private_data;
	unsigned char bytes[2];
	int val;
	int status;

	val = ucontrol->value.integer.value[0];
	if (pt->input == val)
		return 0;

	pt->input = val;
	bytes[0] = PT2348_INPUT_CMD;
	bytes[1] = PT2348_INPUT_MASK | pt->gain[pt->input] | pt->input;

	status = i2c_master_send(pt->client, bytes, 2);
	if (status != 2)
		goto __error;

	return 1;

      __error:
//	snd_i2c_unlock(pt->i2c_bus);
	snd_printk(KERN_ERR "PT2348 access failed 2\n");
	return -EIO;
}

static int pt2348_gain_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_pt2348 *pt = kcontrol->private_data;
	int base = kcontrol->private_value;
	unsigned char bytes[2];
	int val;
	int status;

	val = 16 - ucontrol->value.integer.value[0];
	if (pt->gain[base] == val)
		return 0;

	pt->gain[base] = val;

	if (pt->input != base)
		return 0;

	bytes[0] = PT2348_INPUT_CMD;
	bytes[1] = PT2348_INPUT_MASK | pt->gain[pt->input] | pt->input;

	status = i2c_master_send(pt->client, bytes, 2);
	if (status != 2)
		goto __error;

	return 1;

      __error:
//	snd_i2c_unlock(pt->i2c_bus);
	snd_printk(KERN_ERR "PT2348 access failed 2\n");
	return -EIO;
}

static int pt2348_loudness_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_pt2348 *pt = kcontrol->private_data;
	unsigned char bytes[2];
	int val;
	int status;

	val = ucontrol->value.integer.value[0];
	if (pt->loudness == val)
		return 0;

	pt->loudness = val;
	bytes[0] = PT2348_LOUDNESS_CMD;
	bytes[1] = val;

	status = i2c_master_send(pt->client, bytes, 2);
	if (status != 2)
		goto __error;

	return 1;

      __error:
//	snd_i2c_unlock(pt->i2c_bus);
	snd_printk(KERN_ERR "PT2348 access failed 2\n");
	return -EIO;

}

static int pt2348_volume_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_pt2348 *pt = kcontrol->private_data;
	unsigned char bytes[2];
	int val;
	int status;

	val = 255 - ucontrol->value.integer.value[0];
	if (pt->volume == val)
		return 0;

	pt->volume = val;
	bytes[0] = PT2348_VOLUME_CMD;
	bytes[1] = val;

	status = i2c_master_send(pt->client, bytes, 2);
	if (status != 2)
		goto __error;

	return 1;

      __error:
//	snd_i2c_unlock(pt->i2c_bus);
	snd_printk(KERN_ERR "PT2348 access failed 2\n");
	return -EIO;
}

static int pt2348_tone_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_pt2348 *pt = kcontrol->private_data;
	int base = kcontrol->private_value;
	unsigned char bytes[2];
	int val;
	int status;

	val = ucontrol->value.integer.value[0];
	if (pt->tone[base] == val)
		return 0;

	pt->tone[base] = val;
	bytes[0] = PT2348_LOUDNESS_CMD;
	bytes[1] = pt->tone[0] << 4 | pt->tone[1];

	status = i2c_master_send(pt->client, bytes, 2);
	if (status != 2)
		goto __error;

	return 1;

      __error:
//	snd_i2c_unlock(pt->i2c_bus);
	snd_printk(KERN_ERR "PT2348 access failed 2\n");
	return -EIO;
}

static int pt2348_attenuation_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_pt2348 *pt = kcontrol->private_data;
	int base = kcontrol->private_value;
	unsigned char bytes[2];
	int val0, val1;
	int status;

	val0 = 79 - ucontrol->value.integer.value[0];
	val1 = 79 - ucontrol->value.integer.value[1];
	if (val0 < 0 || val0 > 79 || val1 < 0 || val1 > 79)
		return -EINVAL;
	if (val0 == pt->channels[base] && val1 == pt->channels[base + 1])
		return 0;

	pt->channels[base] = val0;
	bytes[0] = PT2348_ATT_LF_CMD + base;
	bytes[1] = pt->channels[base];

	pt->channels[base + 1] = val1;
	bytes[0] = PT2348_ATT_LF_CMD + base + 2;
	bytes[1] = pt->channels[base + 1];

	status = i2c_master_send(pt->client, bytes, 2);
	if (status != 2)
		goto __error;

	return 1;

      __error:
//	snd_i2c_unlock(pt->i2c_bus);
	snd_printk(KERN_ERR "PT2348 access failed\n");
	return -EIO;
}

static int pt2348_mute_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_pt2348 *pt = kcontrol->private_data;
	unsigned char bytes[2];
	int val;
	int status;

	val = !ucontrol->value.integer.value[0];
	if (pt->mute == val)
		return 0;

	pt->mute = val;
	bytes[0] = PT2348_MUTE_CMD;
	bytes[1] = val ? 0b00000000 : 0b00000001;

	status = i2c_master_send(pt->client, bytes, 2);
	if (status != 2)
		goto __error;

	return 1;

      __error:
//	snd_i2c_unlock(pt->i2c_bus);
	snd_printk(KERN_ERR "PT2348 access failed 2\n");
	return -EIO;
}


static const DECLARE_TLV_DB_SCALE(pt2348_gain_scale, 0, 375, 0);
static const DECLARE_TLV_DB_SCALE(pt2348_loudness_scale, -1875, 125, 0);
static const DECLARE_TLV_DB_SCALE(pt2348_volume_scale, -3875, 125, 1);
static const DECLARE_TLV_DB_SCALE(pt2348_tone_scale, -1400, 200, 0);
static const DECLARE_TLV_DB_SCALE(pt2348_attenuation_scale, -5969, 31, 0);

static int __devinit snd_pt2348_build_controls(struct snd_pt2348 *pt)
{
	struct snd_kcontrol_new knew;
	char *names[12] = {
		"Input Select",
		"Gain 1",
		"Gain 2",
		"Gain 3",
		"Gain 4",
		"Loudness",
		"Volume",
		"Tone Control - Bass",
		"Tone Control - Treble",
		"Front",
		"Rear",
		"Mute"
	};
	int i;

	//struct snd_pt2348 *pt;
	int err = -ENOMEM;

	printk(KERN_INFO "PT2348 build_controls\n");

	// Input Select
	memset(&knew, 0, sizeof(knew));
	knew.name = names[0];
	knew.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	knew.count = 1;
	knew.access = SNDRV_CTL_ELEM_ACCESS_READWRITE |
		SNDRV_CTL_ELEM_ACCESS_TLV_READ;
	knew.private_value = 0;
	knew.info = pt2348_input_info;
	knew.get = pt2348_input_get;
	knew.put = pt2348_input_put;

	err = snd_ctl_add(pt->card, snd_ctl_new1(&knew, pt));
	if (err < 0)
		return err;

	// Input Gain
	for (i = 0; i < 4; i++)
	{
		memset(&knew, 0, sizeof(knew));
		knew.name = names[i+1];
		knew.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
		knew.count = 1;
		knew.access = SNDRV_CTL_ELEM_ACCESS_READWRITE |
			SNDRV_CTL_ELEM_ACCESS_TLV_READ;
		knew.private_value = i;
		knew.info = pt2348_gain_info;
		knew.get = pt2348_gain_get;
		knew.put = pt2348_gain_put;
		knew.tlv.p = pt2348_gain_scale;

		err = snd_ctl_add(pt->card, snd_ctl_new1(&knew, pt));
		if (err < 0)
			return err;
	}

	// Loudness
	memset(&knew, 0, sizeof(knew));
	knew.name = names[4];
	knew.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	knew.count = 1;
	knew.access = SNDRV_CTL_ELEM_ACCESS_READWRITE |
		SNDRV_CTL_ELEM_ACCESS_TLV_READ;
	knew.private_value = 0;
	knew.info = pt2348_loudness_info;
	knew.get = pt2348_loudness_get;
	knew.put = pt2348_loudness_put;
	knew.tlv.p = pt2348_loudness_scale;

	err = snd_ctl_add(pt->card, snd_ctl_new1(&knew, pt));
	if (err < 0)
		return err;

	// Volume
	memset(&knew, 0, sizeof(knew));
	knew.name = names[5];
	knew.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	knew.count = 1;
	knew.access = SNDRV_CTL_ELEM_ACCESS_READWRITE |
		SNDRV_CTL_ELEM_ACCESS_TLV_READ;
	knew.private_value = 0;
	knew.info = pt2348_volume_info;
	knew.get = pt2348_volume_get;
	knew.put = pt2348_volume_put;
	knew.tlv.p = pt2348_volume_scale;

	err = snd_ctl_add(pt->card, snd_ctl_new1(&knew, pt));
	if (err < 0)
		return err;


	// Bass, Treble
	for (i = 0; i < 2; i++)
	{
		memset(&knew, 0, sizeof(knew));
		knew.name = names[i+6];
		knew.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
		knew.count = 1;
		knew.access = SNDRV_CTL_ELEM_ACCESS_READWRITE |
			SNDRV_CTL_ELEM_ACCESS_TLV_READ;
		knew.private_value = 0;
		knew.info = pt2348_tone_info;
		knew.get = pt2348_tone_get;
		knew.put = pt2348_tone_put;
		knew.tlv.p = pt2348_tone_scale;

		err = snd_ctl_add(pt->card, snd_ctl_new1(&knew, pt));
		if (err < 0)
			return err;
	}



	// Attenuators

	memset(&knew, 0, sizeof(knew));
	knew.name = names[8];
	knew.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	knew.count = 1;
	knew.access = SNDRV_CTL_ELEM_ACCESS_READWRITE |
		SNDRV_CTL_ELEM_ACCESS_TLV_READ;
	knew.private_value = 0;
	knew.info = pt2348_attenuation_info;
	knew.get = pt2348_attenuation_get;
	knew.put = pt2348_attenuation_put;
	knew.tlv.p = pt2348_attenuation_scale;

	err = snd_ctl_add(pt->card, snd_ctl_new1(&knew, pt));
	if (err < 0)
		return err;

	memset(&knew, 0, sizeof(knew));
	knew.name = names[9];
	knew.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	knew.count = 1;
	knew.access = SNDRV_CTL_ELEM_ACCESS_READWRITE |
		SNDRV_CTL_ELEM_ACCESS_TLV_READ;
	knew.private_value = 2;
	knew.info = pt2348_attenuation_info;
	knew.get = pt2348_attenuation_get;
	knew.put = pt2348_attenuation_put;
	knew.tlv.p = pt2348_attenuation_scale;

	err = snd_ctl_add(pt->card, snd_ctl_new1(&knew, pt));
	if (err < 0)
		return err;


	// Mute
	memset(&knew, 0, sizeof(knew));
	knew.name = names[10];
	knew.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	knew.count = 1;
	knew.access = SNDRV_CTL_ELEM_ACCESS_READWRITE |
		SNDRV_CTL_ELEM_ACCESS_TLV_READ;
	knew.private_value = 0;
	knew.info = pt2348_mute_info;
	knew.get = pt2348_mute_get;
	knew.put = pt2348_mute_put;

	err = snd_ctl_add(pt->card, snd_ctl_new1(&knew, pt));
	if (err < 0)
		return err;

	return 0;
}















static int snd_pt2348_dev_free(struct snd_device *device)
{
//	struct snd_pt2348 * pt = device->device_data;
//	return snd_pt2348_free(pt);
	return 0;	//FIXME!!!
}

static int snd_pt2348_free(struct snd_pt2348 * pt)
{
	printk(KERN_DEBUG "PT2348 free\n");
//	snd_card_free(pt->card);
	kfree(pt);
	printk(KERN_DEBUG "PT2348 freed\n");
	return 0;
}

static int __devinit snd_pt2348_create(struct snd_card *card, struct snd_pt2348 ** rpt)
{
	static struct snd_device_ops ops = {
		.dev_free = snd_pt2348_dev_free,
	};

	struct snd_pt2348 * pt;
//	struct snd_i2c_device *device;
//	struct snd_i2c_bus * bus;
	int err;

	if ((pt = kzalloc(sizeof(*pt), GFP_KERNEL)) == NULL)
		return -ENOMEM;

	pt->card = card;

	/* Register device */
	if ((err = snd_device_new(card, SNDRV_DEV_LOWLEVEL, pt, &ops)) < 0) {
		snd_pt2348_free(pt);
		return err;
	}

	if (rpt)
		*rpt = pt;

	return 0;
}

static int pt2348_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	//struct snd_pt2348 *pt;
	struct snd_card * card;
	struct snd_pt2348 *pt2348;
	int err, i;

	int dev = 2;

//	if (debug > 0)
		printk(KERN_DEBUG "pt2348_probe\n");

	printk(KERN_DEBUG "chip found @ 0x%x (%s)\n", client->addr << 1, client->adapter->name);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_WRITE_BYTE))
		return -EIO;

////////////////////////////


	printk(KERN_INFO "PT2348 probe\n");

	err  = snd_card_create(dev, "PT2348", THIS_MODULE, 0, &card);
	if (err < 0)
		return err;

	strcpy(card->driver, "PT2348");
	strcpy(card->shortname, "4.1 Audio Processor PT2348");

	printk(KERN_INFO "PT2348 snd_card created\n");

	if ((err = snd_pt2348_create(card, &pt2348)) < 0 )
		goto _err;

	pt2348->client = client;

	printk(KERN_INFO "PT2348 snd_pt2348 created\n");

	snd_pt2348_reset(pt2348);

	snd_pt2348_build_controls(pt2348);

	snd_card_set_dev(card, &client->dev);
	if ((err = snd_card_register(card)) < 0)
		goto _err;

	 printk(KERN_INFO "PT2348 snd_card registered\n");

/////////////////////////////

	if (err) {
		printk(KERN_ERR "could not initialize pt2348\n");
		return -ENODEV;
	}

	return 0;

       _err:
	snd_card_free(card);
	return err;
}

static int pt2348_remove(struct i2c_client *client)
{
	struct snd_pt2348 *pt = i2c_get_clientdata(client);

	//snd_card_free(pt->card);

	// TODO remove
//	if (debug > 0)
		printk(KERN_DEBUG "pt2348_remove\n");

	return 0;
}

static const struct i2c_device_id pt2348_id[] = {
	{ "pt2348", 0 },
	{ }
};

static struct i2c_driver pt2348_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "pt2348",
	},
	.probe		= pt2348_probe,
	.remove		= pt2348_remove,
	.id_table	= pt2348_id,
};

module_i2c_driver(pt2348_driver);
