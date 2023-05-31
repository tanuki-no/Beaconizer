/*!
 *	\file		ibeacon.c
 *	\brief		IBeacon logic
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		25/06/2022
 *	\version	1.10
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/random.h>
#include <sys/socket.h>

#include "beaconizer/btest.h"
#include "beaconizer/hci_cmd.h"
#include "beaconizer/ibeacon.h"


/*! Command line args */
static const struct option ibeacon_long_options[] = {
    { "advert",     required_argument,  NULL, 'a' },
    { "mode",       required_argument,  NULL, 'c' },
    { "index",      required_argument,  NULL, 'i' },
    { "major",      required_argument,  NULL, 'M' },
    { "minor",      required_argument,  NULL, 'm' },
    { "name",       required_argument,  NULL, 'n' },
    { "password",   required_argument,  NULL, 'p' },
    { "serial",     required_argument,  NULL, 's' },
    { "tx",         required_argument,  NULL, 't' },
    { "uuid",       required_argument,  NULL, 'u' },
    { "version",    no_argument,        NULL, 'v' },
    { "help",       no_argument,        NULL, 'h' },
    { 0,            0,                  NULL, 0 }
};

static const char* ibeacon_short_options = "a:c:i:M:m:n:p:s:t:u:vh";

/* Settings */
ibeacon_t   ibeacon_settings;    /*! Beacon settings */
static int hci_desc = -1;

/*! Help */
static void ib_help();

/*! Load defaults */
static void ib_set_defaults();

/*! Handle command line args */
static int ib_process_command_line(
    int             argc,
    char * const    argv[]
);

/* Printers */
static void ib_print_dev_common(
    struct hci_dev_info *di,
    struct hci_version  *ver);

static void ib_print_flags(
    struct hci_dev_info *di);

static void ib_print_dev_features(
    struct hci_dev_info *di);

/* HCI init/open */
static int ib_open_hci();

/* Clean up on exit */
static void ib_clean_up();

/*! Main loop */
int
main(int argc, char * const argv[], char * const env[]) {

    /* Set up deafults */
    int exit_status = EXIT_SUCCESS;

    /* Set up deafults */
    ib_set_defaults();

    /* Process command line */
    if (EXIT_SUCCESS != ib_process_command_line(argc, argv)) {
        return EXIT_FAILURE;
    }

    /* Check HCI */
    if (0 == ib_open_hci()) {
        /* Start advertising */
        printf("hci%u (mode: %d): \"%s\" %.2X%.2X%.2X%.2X-%.2X%.2X-%.2X%.2X-%.2X%.2X-%.2X%.2X%.2X%.2X%.2X%.2X/%u:%u (S/N: %c%c%c%c%c, TX %f dBm), adv %u ms ...\n",
            ibeacon_settings.hci,
            (int) ibeacon_settings.mode,
            ibeacon_settings.name,
            ibeacon_settings.uuid[0],  ibeacon_settings.uuid[1],  ibeacon_settings.uuid[2],  ibeacon_settings.uuid[3],
            ibeacon_settings.uuid[4],  ibeacon_settings.uuid[5],  ibeacon_settings.uuid[6],  ibeacon_settings.uuid[7],
            ibeacon_settings.uuid[8],  ibeacon_settings.uuid[9],  ibeacon_settings.uuid[10], ibeacon_settings.uuid[11],
            ibeacon_settings.uuid[12], ibeacon_settings.uuid[13], ibeacon_settings.uuid[14], ibeacon_settings.uuid[15],
            ibeacon_settings.major,
            ibeacon_settings.minor,
            ibeacon_settings.serial[0],ibeacon_settings.serial[1],ibeacon_settings.serial[2],ibeacon_settings.serial[3],
            ibeacon_settings.serial[4],
            ibeacon_settings.tx_power,
            ibeacon_settings.advertize);

        /* */

        /* Stop */
        printf("Done!\n");
    }

    /* Clean up */
    printf("Exiting ...\n");
    ib_clean_up();

    return exit_status;
}

/*! Description of utility
 */
 static void ib_help() {

    printf(
        "Beacon test suite %s\n"
        "---------------------------------------------------------------------------------------------------\n", __BTEST_VERSION_STRING);
    printf(
        "%s - Low Energy iBeacon testing tool\n"
        "---------------------------------------------------------------------------------------------------\n", __IBEACON_NAME);
    printf(
        "Usage: %s [options]\n"
        "---------------------------------------------------------------------------------------------------\n", __IBEACON_NAME);
    printf(
        "Options:\n");
    printf(
        "\t-a, --advert <num>     Advertising interval in ms (optional, default is %d ms)\n", __IBEACON_DEFAULT_ADVERTISE);
    printf(
        "\t-c, --mode <num>       Connection mode (optional, default is %d)\n", __IBEACON_DEFAULT_CONN_MODE);
    printf(
        "\t-i, --index <num>      Use specified controller (optional, default is %d)\n", __IBEACON_DEFAULT_HCI_CTRL);
    printf(
        "\t-M, --major <num>      Major (required, default is %d)\n", __IBEACON_DEFAULT_MAJOR);
    printf(
        "\t-m, --minor <num>      Minor (required, default is %d)\n", __IBEACON_DEFAULT_MINOR);
    printf(
        "\t-n, --name <str>       Device name (optional, default is %s)\n", __IBEACON_DEFAULT_NAME);
    printf(
        "\t-p, --password <str>   Password (optional, default is %s)\n", __IBEACON_DEFAULT_PASSWORD);
    printf(
        "\t-s, --serial <str>     Serial ID (optional value in range 00000-99999, default is %s)\n", __IBEACON_DEFAULT_SERIAL);
    printf(
        "\t-t, --tx <num>         TX Power (optional value in dBm, default is %f)\n", __IBEACON_DEFAULT_TX_POWER);
    printf(
        "\t-u, --uuid <str>       UUID (optional, autogenerated by default, must be 32 digits, separated by \':\' or \'-\')\n");
    printf(
        "\t-h, --help             Show help options\n"
        "\t-v, --version          Show version\n"
        "---------------------------------------------------------------------------------------------------\n");
 }

/*! Set defaults */
static void ib_set_defaults() {
    ibeacon_settings.hci         = __IBEACON_DEFAULT_HCI_CTRL;
    ibeacon_settings.mode        = __IBEACON_DEFAULT_CONN_MODE;
    ibeacon_settings.advertize   = __IBEACON_DEFAULT_ADVERTISE;
    ibeacon_settings.major       = __IBEACON_DEFAULT_MAJOR;
    ibeacon_settings.minor       = __IBEACON_DEFAULT_MINOR;
    ibeacon_settings.measured_power = __IBEACON_DEFAULT_MEASURED_POWER;
    ibeacon_settings.tx_power    = __IBEACON_DEFAULT_TX_POWER;
    strncpy(
        ibeacon_settings.name,
        __IBEACON_DEFAULT_NAME,
        sizeof(__IBEACON_DEFAULT_NAME));
    strncpy(
        ibeacon_settings.password,
        __IBEACON_DEFAULT_PASSWORD,
        __IBEACON_PASSWORD_LENGTH);
    strncpy(
        ibeacon_settings.serial,
        __IBEACON_DEFAULT_SERIAL,
        __IBEACON_SERIAL_LENGTH);
    getrandom(
        ibeacon_settings.uuid,
        16,
        GRND_RANDOM);
}

/*! Handle command line args */
static int ib_process_command_line(
    int             argc,
    char * const    argv[]
) {
    int8_t minor_is_set = 0, major_is_set = 0;

    /* Process options */
    for (int key = getopt_long(argc, argv, ibeacon_short_options, ibeacon_long_options, NULL), index = 0;
             key != -1;
             key = getopt_long(argc, argv, ibeacon_short_options, ibeacon_long_options, &index)) {
        
        char* ep = NULL;
        long c = 0;
        double d = 0.0;
        size_t l = 0;

        switch (key) {
            /* Advertizing interval */
            case 'a':   {

                errno = 0;
                c = strtol(optarg, &ep, 0);
                if ((ERANGE == errno && (LONG_MAX == c || LONG_MIN == c)) || (0 != errno && NULL != ep)) {
                    perror("Bad advertize value: ");
                    return EXIT_FAILURE;
                }
                
                if (0 > c) {
                    printf("Advertizing value must be positive! Exiting ...\n");
                    return EXIT_FAILURE;
                }

                ibeacon_settings.advertize = (uint32_t) c;

            } break;

            /* Connection mode */
            case 'c': {

                errno = 0;
                c = strtol(optarg, &ep, 0);
                if ((ERANGE == errno && (LONG_MAX == c || LONG_MIN == c)) || (0 != errno && NULL != ep)) {
                    perror("Bad connection mode value: ");
                    return EXIT_FAILURE;
                }

                if (0 > c) {
                    printf("Connection mode value must be positive! Exiting ...\n");
                    return EXIT_FAILURE;
                }

                if (UINT8_MAX < c) {
                    printf("Connection mode must be less than %u! Exiting ...\n", UINT8_MAX);
                    return EXIT_FAILURE;
                }

                ibeacon_settings.mode = (uint8_t) c;

                } break;

            /* HCI index */
            case 'i': {

                errno = 0;
                c = strtol(optarg, &ep, 0);
                if ((ERANGE == errno && (LONG_MAX == c || LONG_MIN == c)) || (0 != errno && NULL != ep)) {
                    perror("Bad bluetooth HCI controller index value: ");
                    return EXIT_FAILURE;
                }

                if (0 > c) {
                    printf("Bluetooth HCI controller index value must be positive! Exiting ...\n");
                    return EXIT_FAILURE;
                }

                if (UINT16_MAX < c) {
                    printf("Bluetooth HCI controller index must be less than %u! Exiting ...\n", UINT16_MAX);
                    return EXIT_FAILURE;
                }

                ibeacon_settings.hci = (uint16_t) c;

                } break;

            /* Major */
            case 'M': { 

                errno = 0;
                c = strtol(optarg, &ep, 0);
                if ((ERANGE == errno && (LONG_MAX == c || LONG_MIN == c)) || (0 != errno && NULL != ep)) {
                    perror("Bad major value: ");
                    return EXIT_FAILURE;
                }

                if (0 > c) {
                    printf("Major value must be positive! Exiting ...\n");
                    return EXIT_FAILURE;
                }

                if (UINT16_MAX < c) {
                    printf("Major value must be less than %u! Exiting ...\n", UINT16_MAX);
                    return EXIT_FAILURE;
                }

                ibeacon_settings.major = (uint16_t) c;
                major_is_set = 1;

                } break;

            /* Minor */
            case 'm': { 

                errno = 0;
                c = strtol(optarg, &ep, 0);
                if ((ERANGE == errno && (LONG_MAX == c || LONG_MIN == c)) || (0 != errno && NULL != ep)) {
                    perror("Bad minor value: ");
                    return EXIT_FAILURE;
                }

                if (0 > c) {
                    printf("Minor value must be positive! Exiting ...\n");
                    return EXIT_FAILURE;
                }

                if (UINT16_MAX < c) {
                    printf("Minor value must be less than %u! Exiting ...\n", UINT16_MAX);
                    return EXIT_FAILURE;
                }
                
                ibeacon_settings.minor = (uint16_t) c;
                minor_is_set = 1;

                } break;

            /* Beacon name */
            case 'n': {

                l = strlen(optarg);
                if (__IBEACON_DEFAULT_NAME_LENGTH <= l) {
                    printf("%s is to big for iBeacon name, expecting %u characters! Exiting ...\n",
                        optarg,
                        __IBEACON_DEFAULT_NAME_LENGTH - 1);
                    return EXIT_FAILURE;
                }

                strncpy(ibeacon_settings.name, optarg, __IBEACON_DEFAULT_NAME_LENGTH - 1);

                } break;

            /* Password */
            case 'p': {

                l = strlen(optarg);
                if (__IBEACON_PASSWORD_LENGTH < l) {
                    printf("%s is to big for iBeacon password, expecting %u characters! Exiting ...\n",
                        optarg,
                        __IBEACON_PASSWORD_LENGTH);
                    return EXIT_FAILURE;
                }

                bcopy(optarg, ibeacon_settings.password, __IBEACON_PASSWORD_LENGTH);

                } break;

            /* Serial */
            case 's': {

                l = strlen(optarg);
                if (__IBEACON_SERIAL_LENGTH < l) {
                    printf("%s is to big for iBeacon serial, expecting %u characters! Exiting ...\n",
                        optarg,
                        __IBEACON_SERIAL_LENGTH);
                    return EXIT_FAILURE;
                }

                bcopy(optarg, ibeacon_settings.serial, __IBEACON_SERIAL_LENGTH);

                } break;

            /* TX Power */
            case 't': {

                errno = 0;
                d = strtod(optarg, &ep);
                if (ERANGE == errno) {
                    perror("Bad TX power value: ");
                    return EXIT_FAILURE;
                }

                ibeacon_settings.tx_power = d;

                } break;

            /* UUID */
            case 'u': {
                /* Parse UUID */
                l = strlen(optarg);
                for (size_t i = 0, j = 0, k = 0; (i < l) && (j < 16); ++i) {
                    if (('0' <= optarg[i]) && ('9' >= optarg[i])) {
                        if (k) {
                            ibeacon_settings.uuid[j] <<= 4;
                            ibeacon_settings.uuid[j] |=  (optarg[i] - '0');
                            j++;
                        } else {
                            ibeacon_settings.uuid[j] =  (optarg[i] - '0');
                        }
                        k = !k;
                    } else if (('A' <= optarg[i]) && ('F' >= optarg[i])) {
                        if (k) {
                            ibeacon_settings.uuid[j] <<= 4;
                            ibeacon_settings.uuid[j] |=  10 + (optarg[i] - 'A');
                            j++;
                        } else {
                            ibeacon_settings.uuid[j] =  10 + (optarg[i] - 'A');
                        }
                        k = !k;
                    } else if (('a' <= optarg[i]) && ('f' >= optarg[i])) {
                        if (k) {
                            ibeacon_settings.uuid[j] <<= 4;
                            ibeacon_settings.uuid[j] |=  10 + (optarg[i] - 'a');
                            j++;
                        } else {
                            ibeacon_settings.uuid[j] =  10 + (optarg[i] - 'a');
                        }
                        k = !k;
                    } else if ((':' == optarg[i]) || ('-' == optarg[i])) {
                        continue;
                    } else {
                        printf("Wrong UUID format: %s. Please, use XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX or XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX/ Exiting ...\n", optarg);
                        return EXIT_FAILURE;
                    }
                }
                } break;

            /* Help */
            case 'h': {

                ib_help();
                return EXIT_FAILURE;

                } break;

            /* Version */
            case 'v': {

                printf("%s\n", __BTEST_VERSION_STRING);
                return EXIT_FAILURE;

                } break;

            /* Unknown switch */
            default: {

                ib_help();
                return EXIT_FAILURE;
            }
        }
    }

    /* Check wrong args */
    if (optind < argc) {
        printf("Bad parameters:");
        while (optind < argc)
            printf(" \"%s\"", argv[optind++]);
        printf("! Exiting ...\n");
        return EXIT_FAILURE;
    }

    /* Check major/minor */
    if (!major_is_set) {
        printf("Please, set iBeacon major value! Exiting ...\n\n");
        ib_help();
        return EXIT_FAILURE;
    }

    if (!minor_is_set) {
        printf("Please, set iBeacon minor value! Exiting ...\n\n");
        ib_help();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* Print device info */
static void ib_print_dev_common(
    struct hci_dev_info *di,
    struct hci_version  *ver)
{
    char addr[18];
    char    *_hci_ver_str = NULL;
    char    *_hci_lmp_str = NULL;


    if (0 > di->dev_id)  
        return;
    
    ba2str(&di->bdaddr, addr);
    _hci_ver_str = hci_vertostr(ver->hci_ver);
    if (((di->type & 0x30) >> 4) == HCI_PRIMARY)
        _hci_lmp_str = lmp_vertostr(ver->lmp_ver);
    else
        _hci_lmp_str = pal_vertostr(ver->lmp_ver);

    printf("%s:\tType: %s  Bus: %s\n", di->name,
        hci_typetostr((di->type & 0x30) >> 4),
        hci_bustostr(di->type & 0x0f));
    
    printf("\tBD Address: %s  ACL MTU: %d:%d  SCO MTU: %d:%d\n",
        addr, di->acl_mtu, di->acl_pkts,
        di->sco_mtu, di->sco_pkts);

    printf( "\tHCI Version: %s (0x%x)  Revision: 0x%x\n"
            "\t%s Version: %s (0x%x)  Subversion: 0x%x\n"
            "\tManufacturer: %s (%d)\n",
        NULL == _hci_ver_str ? "n/a" : _hci_ver_str,
        ver->hci_ver,
        ver->hci_rev,
        (((di->type & 0x30) >> 4) == HCI_PRIMARY) ? "LMP" : "PAL",
        NULL == _hci_lmp_str ? "n/a" : _hci_lmp_str,
        ver->lmp_ver,
        ver->lmp_subver,
        bt_compidtostr(ver->manufacturer),
        ver->manufacturer);

    /* Clean up */
    if (_hci_lmp_str)
        bt_free(_hci_lmp_str);

    if (_hci_ver_str)
        bt_free(_hci_ver_str);   
}

static void ib_print_dev_flags(
    struct hci_dev_info *di) {
    char *s = hci_dflagstostr(di->flags);
    printf("\tFlags: %s\n", s);
    bt_free(s);

    printf("\tRX bytes:%d acl:%d sco:%d events:%d errors:%d\n",
        di->stat.byte_rx,
        di->stat.acl_rx,
        di->stat.sco_rx,
        di->stat.evt_rx,
        di->stat.err_rx);

    printf("\tTX bytes:%d acl:%d sco:%d commands:%d errors:%d\n",
        di->stat.byte_tx,
        di->stat.acl_tx,
        di->stat.sco_tx,
        di->stat.cmd_tx,
        di->stat.err_tx);
}

static void ib_print_dev_features(
    struct hci_dev_info *di) {
    printf( "\tFeatures: 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x "
            "0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x\n",
            di->features[0],
            di->features[1],
            di->features[2],
            di->features[3],
            di->features[4],
            di->features[5],
            di->features[6],
            di->features[7]);

    char *s = lmp_featurestostr(di->features, "\t\t", 63);
    printf("%s\n", s);
    bt_free(s);
}


/* Open/init HCI */
static int ib_open_hci(
    ) {

    int e = EXIT_SUCCESS;
    struct hci_dev_info     _hci_dev_info;
    struct hci_version      _hci_version;

    /* Getting infomration about HCI */
    if (0 > hci_devinfo(ibeacon_settings.hci, &_hci_dev_info)) {
        return EXIT_FAILURE;
    }

    printf("Opening HCI %d ... ", ibeacon_settings.hci);

    /* Check index */
    hci_desc = hci_open_dev(ibeacon_settings.hci);
    if (0 > hci_desc) {
        printf("Failed!\n");
        return EXIT_FAILURE;
    }
    printf("OK!\n");

    /* Get version */
    if (0 > hci_read_local_version(hci_desc, &_hci_version, 1000)) {
        return EXIT_FAILURE;
    }

    /* Dump controller info */
    printf("\n");
    ib_print_dev_common(&_hci_dev_info, &_hci_version);
    ib_print_dev_flags(&_hci_dev_info);
    if (!hci_test_bit(HCI_RAW, &_hci_dev_info.flags)) {
        ib_print_dev_features(&_hci_dev_info);
    }
    printf("\n");

    return EXIT_SUCCESS;
}

static void ib_clean_up() {

    int e;

    /* Close HCI anyway */ 
    printf("Closing HCI %d ... ", ibeacon_settings.hci);
    e = hci_close_dev(hci_desc);
    if (0 != e && EBADF == e) {
        printf("%s. Failed!\n", strerror(e));
    }
    printf("OK!\n");
}

/* End of file*/