#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "../include/server.h"

typedef struct {
    char* hostname;
    int port;
    char* username;
    char* password;
    char* nick;
    int username_flag;
    int password_flag;
    int nick_flag;
} Config;

Config init_config() {
    Config config;
    config.hostname = "127.0.0.1";
    config.port = 8080;
    config.username = NULL;
    config.password = NULL;
    config.nick = NULL;
    config.username_flag = 0;
    config.password_flag = 0;
    config.nick_flag = 0;
    return config;
}

static const char *const circb_help =
	"circb"
	"\n"
	"Usage:"
	"\n  circb [-h] [[options] ...]"
	"\n"
	"\nInfo:"
	"\n  -h, --help      		Print help message and exit"
	"\n"
	"\nOptions:"
	"\n  -c  --config=CONFIG_FILE      Define path to config file containing options"
	"\n  -s, --server=HOSTNAME     	Set connection hostname"
	"\n  -p, --port=PORT           	Set connection port"
	"\n  -u, --username=USERNAME   	Set IRC username"
	"\n  -w, --password=PASSWORD   	Set IRC password"
	"\n  -n, --nick=NICKNAME   	Set IRC nickname";


static const char *const usage_help = "Usage: circb [-h] [-s hostname] [-p port] [-u username] [-w password] [-n nick]";

Config parse_config_file(const char *filename) {
	FILE* file = fopen(filename, "r");
	Config config = init_config();

	// If there is issue opening file
	if (file == NULL) {
        	perror("Error opening config file");
        	exit(EXIT_FAILURE);
	}

	// init a buffer variable for reading lines
	char line[256];

	// loop over lines in config file
	while (fgets(line, sizeof(line), file)) {
		// Grabbing key = value pairs
        	char* key = strtok(line, "=");
        	char* value = strtok(NULL, "\n");

		// If either key or value is null, throw error
        	if (key == NULL || value == NULL) {
            		fprintf(stderr, "Error parsing line: %s\n", line);
            		fclose(file);
            		exit(EXIT_FAILURE);
        	}

		// Grab values for each key of interest, one-by-one
        	if (strcmp(key, "hostname") == 0) {
            		config.hostname = strdup(value);
        	} else if (strcmp(key, "port") == 0) {
            		config.port = atoi(value);
        	} else if (strcmp(key, "username") == 0) {
			config.username = strdup(value);
		} else if (strcmp(key, "password") == 0) {
			config.password = strdup(value);
		} else if (strcmp(key, "nick") == 0) {
			config.nick = strdup(value);
		}
	}

    	if (ferror(file)) {
        	perror("Error reading from config file");
        	fclose(file);
        	exit(EXIT_FAILURE);
    	}

    	fclose(file);
    	return config;
}

Config parse_args (int argc, char *argv[]) {
	Config config = init_config();
	int opt;

	struct option long_opts[] = {
		{"help", no_argument, 0, 'h'},
		{"server", required_argument, 0, 's'},
		{"port", required_argument, 0, 'p'},
		{"username", required_argument, 0, 'u'},
		{"password", required_argument, 0, 'w'},
		{"nick", required_argument, 0, 'n'}
	};

    	while((opt = getopt_long(argc, argv, "hs:p:u:w:n:", long_opts, 0)) > 0) {
        	switch(opt) {
            		case 'h':
				puts(circb_help);
                		exit(0);
                		break;
			case 'c':
				config = parse_config_file(optarg);
				break;
        		case 's':
                		config.hostname = strdup(optarg);
                		break;
            		case 'p':
                		config.port = atoi(optarg);
                		break;
            		case 'u':
                		config.username = strdup(optarg);
				config.username_flag = 1;
                		break;
            		case 'w':
                		config.password = strdup(optarg);
				config.password_flag = 1;
                		break;
			case 'n':
				config.nick = strdup(optarg);
				config.nick_flag = 1;
				break;
            		default:
                		fprintf(stderr, usage_help);
                		exit(EXIT_FAILURE);
        	}
    	}
	if (!config.username_flag) {
		fprintf(stderr, "Error. Missing required argument: username\n");
	}
	if (!config.password_flag) {
		fprintf(stderr, "Error. Missing required argument: password\n");
	}
	if (!config.nick_flag) {
		fprintf(stderr, "Error. Missing required argument: nick\n");
	}
	if (!config.username_flag || !config.password_flag || !config.nick_flag) {
		puts(usage_help);
		exit(EXIT_FAILURE);
	}

	return config;
}

int main(int argc, char *argv[]) {
	Config config = parse_args(argc, argv);
	
	printf("Config parameters supplied:\n");
	printf("hostname: %s\n", config.hostname);
	printf("port: %d\n", config.port);
	printf("username: %s\n", config.username);
	// below, safest code ever
	printf("password!!: %s\n", config.password);
	printf("nick: %s\n", config.nick);
	
	// TODO: Initialization of other components, IO, State, etc.


	// TODO: logic to start/connect to the server/client with the parsed config
	
	
	//free(config.hostname);
	//free(config.username);
	//free(config.password);
}
