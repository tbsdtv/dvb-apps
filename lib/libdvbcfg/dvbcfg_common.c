/**
 * dvbcfg_common utilities.
 *
 * Copyright (c) 2005 by Andrew de Quincey <adq_dvb@lidskialf.net>
 *
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include "dvbcfg_common.h"
#include "dvbcfg_util.h"

static const struct dvbcfg_setting bandwidth_list [] = {
        { "BANDWIDTH_6_MHZ", DVBFE_DVBT_BANDWIDTH_6_MHZ },
	{ "BANDWIDTH_7_MHZ", DVBFE_DVBT_BANDWIDTH_7_MHZ },
	{ "BANDWIDTH_8_MHZ", DVBFE_DVBT_BANDWIDTH_8_MHZ },
	{ "BANDWIDTH_AUTO",  DVBFE_DVBT_BANDWIDTH_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting guard_interval_list [] = {
	{"GUARD_INTERVAL_1_16", DVBFE_DVBT_GUARD_INTERVAL_1_16},
	{"GUARD_INTERVAL_1_32", DVBFE_DVBT_GUARD_INTERVAL_1_32},
	{"GUARD_INTERVAL_1_4",  DVBFE_DVBT_GUARD_INTERVAL_1_4},
	{"GUARD_INTERVAL_1_8",  DVBFE_DVBT_GUARD_INTERVAL_1_8},
	{"GUARD_INTERVAL_AUTO", DVBFE_DVBT_GUARD_INTERVAL_AUTO},
        { NULL, -1 },
};

static const struct dvbcfg_setting hierarchy_information_list [] = {
	{ "HIERARCHY_NONE", DVBFE_DVBT_HIERARCHY_NONE },
	{ "HIERARCHY_1",    DVBFE_DVBT_HIERARCHY_1 },
	{ "HIERARCHY_2",    DVBFE_DVBT_HIERARCHY_2 },
	{ "HIERARCHY_4",    DVBFE_DVBT_HIERARCHY_4 },
	{ "HIERARCHY_AUTO", DVBFE_DVBT_HIERARCHY_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting constellation_list [] = {
	{ "QPSK",     DVBFE_DVBT_CONST_QPSK },
	{ "QAM_16",   DVBFE_DVBT_CONST_QAM_16 },
	{ "QAM_32",   DVBFE_DVBT_CONST_QAM_32 },
	{ "QAM_64",   DVBFE_DVBT_CONST_QAM_64 },
	{ "QAM_128",  DVBFE_DVBT_CONST_QAM_128 },
	{ "QAM_256",  DVBFE_DVBT_CONST_QAM_256 },
	{ "AUTO",     DVBFE_DVBT_CONST_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting transmission_mode_list [] = {
	{ "TRANSMISSION_MODE_2K",   DVBFE_DVBT_TRANSMISSION_MODE_2K },
	{ "TRANSMISSION_MODE_8K",   DVBFE_DVBT_TRANSMISSION_MODE_8K },
	{ "TRANSMISSION_MODE_AUTO", DVBFE_DVBT_TRANSMISSION_MODE_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting inversion_list[] = {
	{ "INVERSION_OFF",  DVBFE_INVERSION_OFF },
	{ "INVERSION_ON",   DVBFE_INVERSION_ON },
	{ "INVERSION_AUTO", DVBFE_INVERSION_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting fec_list[] = {
	{ "FEC_NONE", DVBFE_FEC_NONE },
	{ "FEC_1_2",  DVBFE_FEC_1_2 },
	{ "FEC_2_3",  DVBFE_FEC_2_3 },
	{ "FEC_3_4",  DVBFE_FEC_3_4 },
	{ "FEC_4_5",  DVBFE_FEC_4_5 },
	{ "FEC_5_6",  DVBFE_FEC_5_6 },
	{ "FEC_6_7",  DVBFE_FEC_6_7 },
	{ "FEC_7_8",  DVBFE_FEC_7_8 },
	{ "FEC_8_9",  DVBFE_FEC_8_9 },
	{ "FEC_AUTO", DVBFE_FEC_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting dvbc_modulation_list[] = {
	{ "QAM_16",   DVBFE_DVBC_MOD_QAM_16 },
	{ "QAM_32",   DVBFE_DVBC_MOD_QAM_32 },
	{ "QAM_64",   DVBFE_DVBC_MOD_QAM_64 },
	{ "QAM_128",  DVBFE_DVBC_MOD_QAM_128 },
	{ "QAM_256",  DVBFE_DVBC_MOD_QAM_256 },
	{ "AUTO",     DVBFE_DVBC_MOD_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting atsc_modulation_list[] = {
	{ "VSB_8",    DVBFE_ATSC_MOD_VSB_8 },
	{ "VSB_16",   DVBFE_ATSC_MOD_VSB_16 },
	{ "QAM_64",   DVBFE_ATSC_MOD_QAM_64 },
	{ "QAM_256",  DVBFE_ATSC_MOD_QAM_256 },
	{ "AUTO",     DVBFE_ATSC_MOD_AUTO },
        { NULL, -1 },
};

int dvbcfg_delivery_from_string(char * delivery_str,
				dvbfe_type_t source_type,
                                struct dvbcfg_delivery *delivery)
{
        int numtokens;
        int val;
        char *linepos;
        int long_delivery = 0;

        switch(source_type) {
        case DVBFE_TYPE_DVBS:
                numtokens = dvbcfg_tokenise(delivery_str, " \t", -1, 1);
                if (numtokens != 5)
                        return -EINVAL;
                linepos = delivery_str;

                /* frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.frequency = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* determine delivery type */
                if (!strncmp(linepos, "INVERSION_", 10))
                        long_delivery = 1;

                /* inversion */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, inversion_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.inversion = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* polarization */
                if (toupper(linepos[0]) == 'H')
			delivery->u.dvb.u.dvbs.polarization = DVBFE_POLARIZATION_H;
                else if (toupper(linepos[0]) == 'V')
			delivery->u.dvb.u.dvbs.polarization = DVBFE_POLARIZATION_V;
                else if (toupper(linepos[0]) == 'L')
			delivery->u.dvb.u.dvbs.polarization = DVBFE_POLARIZATION_L;
                else if (toupper(linepos[0]) == 'R')
                        delivery->u.dvb.u.dvbs.polarization = DVBFE_POLARIZATION_R;
                else
                        return -EINVAL;
                linepos = dvbcfg_nexttoken(linepos);

                /* symbol_rate */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.u.dvbs.symbol_rate = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* fec_inner */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbs.fec_inner = val;
                break;

        case DVBFE_TYPE_DVBC:
                numtokens = dvbcfg_tokenise(delivery_str, " \t", -1, 1);
                if (numtokens != 5)
                        return -EINVAL;
                linepos = delivery_str;

                /* frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.frequency = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* determine delivery type */
                if (!strncmp(linepos, "INVERSION_", 10))
                        long_delivery = 1;

                /* inversion */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, inversion_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.inversion = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* symbol_rate */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.u.dvbc.symbol_rate = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* fec_inner */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbc.fec_inner = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* modulation */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, dvbc_modulation_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbc.modulation = val;
                break;

        case DVBFE_TYPE_DVBT:
                numtokens = dvbcfg_tokenise(delivery_str, " \t", -1, 1);
                if (numtokens != 9)
                        return -EINVAL;
                linepos = delivery_str;

                /* frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.frequency = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* determine delivery type */
                if (!strncmp(linepos, "INVERSION_", 10))
                        long_delivery = 1;

                /* inversion */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, inversion_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.inversion = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* bandwidth */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, bandwidth_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbt.bandwidth = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* code_rate_HP */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbt.code_rate_HP = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* code_rate_LP */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbt.code_rate_LP = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* constellation */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, constellation_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbt.constellation = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* transmission_mode */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, transmission_mode_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbt.transmission_mode = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* guard_interval */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, guard_interval_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbt.guard_interval = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* hierarchy_information */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, hierarchy_information_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbt.hierarchy_information = val;
                break;

        case DVBFE_TYPE_ATSC:
                numtokens = dvbcfg_tokenise(delivery_str, " \t", -1, 1);
                if (numtokens != 3)
                        return -EINVAL;
                linepos = delivery_str;

                /* frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.frequency = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* determine delivery type */
                if (!strncmp(linepos, "INVERSION_", 10))
                        long_delivery = 1;

                /* inversion */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, inversion_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.inversion = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* modulation */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, atsc_modulation_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.atsc.modulation = val;
                break;

        default:
                return -EINVAL;
        }

        /* success! */
        return 0;
}

int dvbcfg_delivery_to_string(dvbfe_type_t source_type,
                              int long_delivery,
                              struct dvbcfg_delivery *delivery,
                              char* dest,
                              int destsz)
{
        char polarization = 'H';

        switch(source_type) {
        case DVBFE_TYPE_DVBS:
                switch(delivery->u.dvb.u.dvbs.polarization) {
                case DVBFE_POLARIZATION_H:
                        polarization = 'H';
                        break;

                case DVBFE_POLARIZATION_V:
                        polarization = 'V';
                        break;

                case DVBFE_POLARIZATION_L:
                        polarization = 'L';
                        break;

                case DVBFE_POLARIZATION_R:
                        polarization = 'R';
                        break;
                }

                if (long_delivery) {
                        if (snprintf(dest, destsz, "%i %s %c %i %s",
                                    delivery->u.dvb.frequency,
                                    dvbcfg_lookupsetting(delivery->u.dvb.inversion,
                                                          inversion_list),
                                    polarization,
                                    delivery->u.dvb.u.dvbs.symbol_rate,
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbs.fec_inner,
                                                          fec_list)) >= destsz)
                                return -ENOMEM;
                } else {
                        if (snprintf(dest, destsz, "%i %i %c %i %i",
                                    delivery->u.dvb.frequency,
                                    delivery->u.dvb.inversion,
                                    polarization,
                                    delivery->u.dvb.u.dvbs.symbol_rate,
                                    delivery->u.dvb.u.dvbs.fec_inner) >= destsz)
                                return -ENOMEM;
                }
                break;

        case DVBFE_TYPE_DVBC:
                if (long_delivery) {
                        if (snprintf(dest, destsz, "%i %s %i %s %s",
                                    delivery->u.dvb.frequency,
                                    dvbcfg_lookupsetting(delivery->u.dvb.inversion,
                                                          inversion_list),
                                    delivery->u.dvb.u.dvbs.symbol_rate,
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbc.fec_inner,
                                                          fec_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbc.modulation,
                                                          dvbc_modulation_list)) >= destsz)
                                return -ENOMEM;
                } else {
                        if (snprintf(dest, destsz, "%i %i %i %i %i",
                                     delivery->u.dvb.frequency,
                                     delivery->u.dvb.inversion,
                                     delivery->u.dvb.u.dvbs.symbol_rate,
                                     delivery->u.dvb.u.dvbc.fec_inner,
                                     delivery->u.dvb.u.dvbc.modulation) >= destsz)
                                return -ENOMEM;
                }
                break;

        case DVBFE_TYPE_DVBT:
                if (long_delivery) {
                        if (snprintf(dest, destsz, "%i %s %s %s %s %s %s %s %s",
                                    delivery->u.dvb.frequency,
                                    dvbcfg_lookupsetting(delivery->u.dvb.inversion,
                                                          inversion_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbt.bandwidth,
                                                          bandwidth_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbt.code_rate_HP,
                                                          fec_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbt.code_rate_LP,
                                                          fec_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbt.constellation,
                                                          constellation_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbt.transmission_mode,
                                                          transmission_mode_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbt.guard_interval,
                                                          guard_interval_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbt.hierarchy_information,
                                                          hierarchy_information_list)) >= destsz)
                                return -ENOMEM;
                } else {
                        if (snprintf(dest, destsz, "%i %i %i %i %i %i %i %i %i",
                                    delivery->u.dvb.frequency,
                                    delivery->u.dvb.inversion,
                                    delivery->u.dvb.u.dvbt.bandwidth,
                                    delivery->u.dvb.u.dvbt.code_rate_HP,
                                    delivery->u.dvb.u.dvbt.code_rate_LP,
                                    delivery->u.dvb.u.dvbt.constellation,
                                    delivery->u.dvb.u.dvbt.transmission_mode,
                                    delivery->u.dvb.u.dvbt.guard_interval,
                                    delivery->u.dvb.u.dvbt.hierarchy_information) >= destsz)
                                return -ENOMEM;
                }
                break;

        case DVBFE_TYPE_ATSC:
                if (long_delivery) {
                        if (snprintf(dest, destsz, "%i %s %s",
                                    delivery->u.dvb.frequency,
                                    dvbcfg_lookupsetting(delivery->u.dvb.inversion,
                                                         inversion_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.atsc.modulation,
                                                         atsc_modulation_list)) >= destsz)
                                return -ENOMEM;
                } else {
                        if (snprintf(dest, destsz, "%i %i %i",
                                     delivery->u.dvb.frequency,
                                     delivery->u.dvb.inversion,
                                     delivery->u.dvb.u.atsc.modulation) >= destsz)
                                return -ENOMEM;
                }
                break;
        }

        return 0;
}
