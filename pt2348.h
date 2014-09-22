/*
 *   ALSA Driver for the PT2348 volume controller.
 *
 *	Copyright (c) 2006  Jochen Voss <voss@seehuhn.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#ifndef __SOUND_PT2348_H
#define __SOUND_PT2348_H


#define PT2348_ADDR		0x88

#define PT2348_CMD_RESET	0b11111110
#define PT2348_CMD_MUTE		0b11111000

#define PT2348_INPUT_CMD	0b11100000
#define PT2348_LOUDNESS_CMD	0b11100001
#define PT2348_VOLUME_CMD	0b11100010
#define PT2348_TONE_CMD		0b11100011
#define PT2348_ATT_LF_CMD	0b11100100
#define PT2348_ATT_LR_CMD	0b11100101
#define PT2348_ATT_RF_CMD	0b11100110
#define PT2348_ATT_RR_CMD	0b11100111
#define PT2348_MUTE_CMD		0b11101000

#define PT2348_INPUT1		0b00000010
#define PT2348_INPUT2		0b00000001
#define PT2348_INPUT3		0b00000101
#define PT2348_INPUT4		0b00000011	// AM MONO

#define PT2348_INPUT_MASK	0b11100000

static struct platform_device * device;

struct snd_pt2348 {
	struct snd_card * card;
//	struct snd_i2c_bus *i2c_bus;
//	struct snd_i2c_device *i2c_dev;
//	struct resource *i2c_res;
	struct i2c_client * client;

	unsigned char	input;
	unsigned char	gain[4];
	unsigned char	loudness;
	unsigned char	volume;
	unsigned char	tone[2];
	unsigned char	channels[4];
	unsigned char	mute;
};

static int pt2348_input_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo);
static int pt2348_gain_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo);
static int pt2348_loudness_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo);
static int pt2348_volume_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo);
static int pt2348_tone_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo);
static int pt2348_attenuation_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo);
static int pt2348_mute_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo);

static int pt2348_input_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);
static int pt2348_gain_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);
static int pt2348_loudness_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);
static int pt2348_volume_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);
static int pt2348_tone_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);
static int pt2348_attenuation_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);
static int pt2348_mute_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);

static int pt2348_input_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);
static int pt2348_gain_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);
static int pt2348_loudness_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);
static int pt2348_volume_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);
static int pt2348_tone_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);
static int pt2348_attenuation_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);
static int pt2348_mute_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);

static int __devinit snd_pt2348_reset(struct snd_pt2348 *pt);
static int __devinit snd_pt2348_build_controls(struct snd_pt2348 *pt);

#endif /* __SOUND_PT2348_H */
