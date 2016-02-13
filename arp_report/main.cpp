#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h>
#include "sendrecv.h"
#include "log.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <json/json.h>
#include "libconfig.h++"
#include "safe/safe.h"
#include "./socket_lib/Socket.hpp"

using namespace std;
using namespace libconfig;

static int do_fork = 1;
static int signup_flag = 1;

void signup (int signum) {
    switch (signum) {
    case SIGUSR1:
        break;
    case SIGUSR2:
        break;
    case SIGPIPE:
        printf ("Broken PIPE\n");
    case SIGHUP:
    case SIGTERM:
    case SIGABRT:
    case SIGINT: {
        if (signup_flag == 1) {
            /* Prepare Leave, free malloc*/
            signup_flag = 0;
        } else {
            printf ("Signup_flag is already 0\n");
        }
    }
    break;
    case SIGCHLD: {
        wait ( (int*) 0);
    }
    break;
    default:
        printf ("Do nothing, %d\n", signum);
        break;
    }
}

void init_signals (void) {
    signal (SIGTERM, signup);
    signal (SIGABRT, signup);
    signal (SIGUSR1, signup);
    signal (SIGUSR2, signup);
    signal (SIGPIPE, signup);
    signal (SIGCHLD, signup);
    signal (SIGINT, signup);
}

int get_hosts (char *ip, int len) {
    FILE *pp;
    pp = popen ("wget http://ipecho.net/plain -O - -q", "r");
    if (!pp) {
        perror ("popen");
        return -1;
    }
    memset (ip, 0, len);
    fgets (ip, len, pp);
    pclose (pp);
    return 0;
}

static void work_loop() {
    char *cmd;
    char ip[50];
    get_hosts (ip, sizeof (ip));
    while (signup_flag) {
        try {
            // send
            safe_asprintf (&cmd,
                           "curl -H \"Content-Type: application/json\" -H \"api-key:nJCjW8bhvx=aWcIJcogptgbNo0I=\" -X POST --data '{\"mac\":\"%s\"}' http://api.heclouds.com/devices/634648/datapoints?type=3",
                           ip);
            printf ("cmd:%s\n", cmd);
            system (cmd);
            free (cmd);
        } catch (Socket::SocketException &e) {
            cout << e << endl;
        }
        sleep (10);
    }
    printf ("work_loop is closed.\n");
}

void print_usage (const char * prog) {
    system ("clear");
    printf ("Usage: %s [-d]\n", prog);
    puts ("  -d  --set debug mode\n");
}

int parse_opts (int argc, char * argv[]) {
    int ch;

    while ( (ch = getopt (argc, argv, "d")) != EOF) {
        switch (ch) {
        case 'd':
            do_fork = 0;
            break;
        case 'h':
        case '?':
            print_usage (argv[0]);
            return -1;
        default:
            break;
        }
    }

    return 0;
}

int read_conf () {
    Config cfg;
    static const char *output_file = "conf.cfg";
    // Read the file. If there is an error, report it and exit.
    try {
        cfg.readFile (output_file);
    } catch (const FileIOException &fioex) {
        std::cerr << "I/O error while reading file." << std::endl;
        return (EXIT_FAILURE);
    } catch (const ParseException &pex) {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
                  << " - " << pex.getError() << std::endl;
        return (EXIT_FAILURE);
    }
    // Get the store name.
    try {
        int name = cfg.lookup ("int");
        cout << "int: " << name << endl << endl;
    } catch (const SettingNotFoundException &nfex) {
        cerr << "No 'name' setting in configuration file." << endl;
    }
    try {
        string name = cfg.lookup ("str");
        cout << "str: " << name << endl << endl;
    } catch (const SettingNotFoundException &nfex) {
        cerr << "No 'name' setting in configuration file." << endl;
    }

    // Find the 'movies' setting. Add intermediate settings if they don't yet
    // exist.
    Setting &root = cfg.getRoot();
    //root.remove ("int");
    if (! root.exists ("int")) {
        root.add ("int", Setting::TypeInt) = 17;
    }
    if (! root.exists ("str")) {
        root.add ("str", Setting::TypeString) = "hello";
    }
    //if (! root.exists ("inventory")) {
    //root.add ("inventory", Setting::TypeGroup);
    //}
    //Setting &inventory = root["inventory"];
    //if (! inventory.exists ("movies")) {
    //inventory.add ("movies", Setting::TypeList);
    //}
    //Setting &movies = inventory["movies"];

    //// Create the new movie entry.
    //Setting &movie = movies.add (Setting::TypeGroup);

    //movie.add ("title", Setting::TypeString) = "Buckaroo Banzai";
    //movie.add ("media", Setting::TypeString) = "DVD";
    //movie.add ("price", Setting::TypeFloat) = 12.99;
    //movie.add ("qty", Setting::TypeInt) = 20;

    // Write out the updated configuration.
    try {
        cfg.writeFile (output_file);
        cerr << "Updated configuration successfully written to: " << output_file
             << endl;
    } catch (const FileIOException &fioex) {
        cerr << "I/O error while writing file: " << output_file << endl;
        return (EXIT_FAILURE);
    }

}

int main (int argc, char * argv[]) {

    int ret;
    char* cp;

    /* open log */
    cp = strrchr (argv[0], '/');
    if (cp != (char*) 0) {
        ++cp;
    } else {
        cp = argv[0];
    }
    openlog (cp, LOG_NDELAY | LOG_PID, LOG_DAEMON);

    INIT_LOG ("log");
    DEBUG_LOG ("int: %d, str: %s, double: %lf", 1, "hello world", 1.5);

    /* parse opts */
    ret = parse_opts (argc, argv);
    if (ret < 0) {
        exit (0);
    }

    /* read conf file */
    read_conf ();

    /* init_signals */
    init_signals();

    /* background ourself */
    if (do_fork) {
        /* Make ourselves a daemon. */
        if (daemon (0, 0) < 0) {
            syslog (LOG_CRIT, "daemon - %m");
            perror ("daemon");
            exit (1);
        }
    } else {
        /* Even if we don't daemonize, we still want to disown our parent
        ** process.
        */
        (void) setsid();
    }

    /* While loop util program finish */
    work_loop();

    /* exit */
    printf ("Program is finished!\n");
    exit (0);

}
