#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char* hostname;
    int port;
    char* username;
    char* password;
} Config;

Config init_config() {
    Config config;
    config.hostname = "127.0.0.1";
    config.port = 8080;
    config.username = NULL;
    config.password = NULL;
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
	"\n  -c  --config=CONFIG_FILE   Define path to config file containing options"
	"\n  -s, --server=HOSTNAME     	Set connection hostname"
	"\n  -p, --port=PORT           	Set connection port"
	"\n  -w, --password=PASSWORD   	Set IRC password"
	"\n  -u, --username=USERNAME   	Set IRC username";

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

    	while((opt = getopt(argc, argv, "hs:p:u:w:")) > 0) {
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
                		break;
            		case 'w':
                		config.password = strdup(optarg);
                		break;
            		default:
                		fprintf(stderr, "Usage: %s [-h] [-s hostname] [-p port] [-u username] [-w password]\n", argv[0]);
                		exit(EXIT_FAILURE);
        	}
    	}

	return config;
}

int main(int argc, char *argv[]) {
	Config config = parse_args(argc, argv);
	// TODO: Initialization of other components, IO, State, etc.
	// TODO: logic to start/connect to the server/client with the parsed config
	
	printf("Config parameters supplied\n");
	printf("hostname: %s\n", config.hostname);
	printf("port: %d\n", config.port);
	printf("username: %s\n", config.username);
	// below, safest code ever
	printf("password!!: %s\n", config.password);
	free(config.hostname);
	free(config.username);
	free(config.password);
}
