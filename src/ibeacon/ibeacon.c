#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/random.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>

#include "btest/btest.h"
#include "btest/ibeacon.h"

/*! Help */
void help();

/*! Command line args */
static const struct option ibeacon_long_options[] = {
    { "advert",     required_argument, NULL, 'a' },
    { "mode",       required_argument, NULL, 'c' },
    { "index",      required_argument, NULL, 'i' },
    { "major",      required_argument, NULL, 'M' },
    { "minor",      required_argument, NULL, 'm' },
    { "name",       required_argument, NULL, 'n' },
    { "password",   required_argument, NULL, 'p' },
    { "serial",     required_argument, NULL, 's' },
    { "tx",         required_argument, NULL, 't' },
    { "uuid",       required_argument, NULL, 'u' },
    { "version",    no_argument,       NULL, 'v' },
    { "help",       no_argument,       NULL, 'h' },
    { }
};

const char* ibeacon_short_options = "a:c:i:M:m:n:p:s:t:u:vh";

/*! Main loop */
int
main(int argc, char * const argv[], char * const env[]) {

    /* Set up deafults */
    int exit_status = EXIT_SUCCESS;
    uint16_t hci_index = __IBEACON_DEFAULT_HCI_CTRL;
    uint8_t mode = __IBEACON_DEFAULT_CONN_MODE;

    ibeacon_t beacon_settings = {
        .advertize      = __IBEACON_DEFAULT_ADVERTISE,
        .major          = __IBEACON_DEFAULT_MAJOR,
        .minor          = __IBEACON_DEFAULT_MINOR,
        .measured_power = __IBEACON_DEFAULT_MEASURED_POWER,
        .tx_power       = __IBEACON_DEFAULT_TX_POWER
    };

    {
        strncpy(beacon_settings.name,       __IBEACON_DEFAULT_NAME,     sizeof(__IBEACON_DEFAULT_NAME));
        strncpy(beacon_settings.password,   __IBEACON_DEFAULT_PASSWORD, __IBEACON_PASSWORD_LENGTH);
        strncpy(beacon_settings.serial,     __IBEACON_DEFAULT_SERIAL,   __IBEACON_SERIAL_LENGTH);
        getrandom(beacon_settings.uuid, 16, GRND_RANDOM);
    }

    int8_t minor_is_set = 0, major_is_set = 0;

    /* Process options */
    for (int key = getopt_long(argc, argv, ibeacon_short_options, ibeacon_long_options, NULL), index = 0;
             key != -1;
             key = getopt_long(argc, argv, ibeacon_short_options, ibeacon_long_options, &index)) {
        
        char* ep = NULL;
        long c = 0;
        size_t l = 0;

        switch (key) {
            case 'a':   /* Advertizing interval */
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

                beacon_settings.advertize = (uint32_t) c;
                break;
            case 'c':   /* Connection mode */
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

                mode = (uint8_t) c;
                break;
            case 'i':   /* HCI index */
                errno = 0;
                c = strtol(optarg, &ep, 0);
                if ((ERANGE == errno && (LONG_MAX == c || LONG_MIN == c)) || (0 != errno && NULL != ep)) {
                    perror("Bad bluetooth controller index value: ");
                    return EXIT_FAILURE;
                }

                if (0 > c) {
                    printf("Bluetooth controller index value must be positive! Exiting ...\n");
                    return EXIT_FAILURE;
                }

                if (UINT16_MAX < c) {
                    printf("Bluetooth controller index must be less than %u! Exiting ...\n", UINT16_MAX);
                    return EXIT_FAILURE;
                }

                hci_index = (uint16_t) c;
                break;
            case 'M':   /* Major */
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

                beacon_settings.major = (uint16_t) c;
                major_is_set = 1;
                break;
            case 'm':   /* Minor */
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
                
                beacon_settings.minor = (uint16_t) c;
                minor_is_set = 1;
                break;
            case 'n':   /* Beacon name */
                l = strlen(optarg);
                if (__IBEACON_DEFAULT_NAME_LENGTH <= l) {
                    printf("%s is to big for iBeacon name, expecting %u characters! Exiting ...\n",
                        optarg,
                        __IBEACON_DEFAULT_NAME_LENGTH - 1);
                    return EXIT_FAILURE;
                }

                strncpy(beacon_settings.name, optarg, __IBEACON_DEFAULT_NAME_LENGTH - 1);

                break;
            case 'p':   /* Password */
                l = strlen(optarg);
                if (__IBEACON_PASSWORD_LENGTH < l) {
                    printf("%s is to big for iBeacon password, expecting %u characters! Exiting ...\n",
                        optarg,
                        __IBEACON_PASSWORD_LENGTH);
                    return EXIT_FAILURE;
                }

                bcopy(optarg, beacon_settings.password, __IBEACON_PASSWORD_LENGTH);

                break;
            case 's':   /* Serial */
                l = strlen(optarg);
                if (__IBEACON_SERIAL_LENGTH < l) {
                    printf("%s is to big for iBeacon serial, expecting %u characters! Exiting ...\n",
                        optarg,
                        __IBEACON_SERIAL_LENGTH);
                    return EXIT_FAILURE;
                }

                bcopy(optarg, beacon_settings.serial, __IBEACON_SERIAL_LENGTH);

                break;
            case 't':   /* TX Power */
                break;
            case 'u':   /* UUID */
                break;
            case 'h':   /* Help */
                help();
                return exit_status;
                break;
            case 'v':   /* Version */
                printf("%s\n", __BTEST_VERSION_STRING);
                return exit_status;
                break;
            default:
                help();
                return EXIT_FAILURE;
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
        help();
        return exit_status;
    }

    if (!minor_is_set) {
        printf("Please, set iBeacon minor value! Exiting ...\n\n");
        help();
        return exit_status;
    }

    /* Start advertising */
    printf("hci%u: \"%s\" %.2X%.2X%.2X%.2X-%.2X%.2X-%.2X%.2X-%.2X%.2X-%.2X%.2X%.2X%.2X%.2X%.2X/%u:%u, adv %u ms ...\n",
        hci_index,
        beacon_settings.name,
        beacon_settings.uuid[0],  beacon_settings.uuid[1],  beacon_settings.uuid[2],  beacon_settings.uuid[3],
        beacon_settings.uuid[4],  beacon_settings.uuid[5],  beacon_settings.uuid[6],  beacon_settings.uuid[7],
        beacon_settings.uuid[8],  beacon_settings.uuid[9],  beacon_settings.uuid[10], beacon_settings.uuid[11],
        beacon_settings.uuid[12], beacon_settings.uuid[13], beacon_settings.uuid[14], beacon_settings.uuid[15],
        beacon_settings.major,
        beacon_settings.minor,
        beacon_settings.advertize);

    /* Done!*/
    printf("Exiting ...\n");

    return exit_status;
}

/*! Description of utility
 */
 void help() {

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
        "\t-u, --uuid <str>       UUID (optional, autogenerated by default)\n");
    printf(
        "\t-h, --help             Show help options\n"
        "\t-v, --version          Show version\n"
        "---------------------------------------------------------------------------------------------------\n");
 }

/* End of file*/