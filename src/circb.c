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
	"\n  rirc [-h] [[options] ...]"
	"\n"
	"\nInfo:"
	"\n  -h, --help      		Print help message and exit"
	"\n"
	"\nOptions:"
	"\n  -s, --server=HOSTNAME     	Set connection hostname"
	"\n  -p, --port=PORT           	Set connection port"
	"\n  -w, --password=PASSWORD   	Set IRC password"
	"\n  -u, --username=USERNAME   	Set IRC username";

Config parse_args (int argc, char *argv[]) {
	Config config = init_config();
	int opt;

    	while((opt = getopt(argc, argv, "h:s:p:u:w:")) > 0) {
        	switch(opt) {
            		case 'h':
				puts(circb_help);
                		exit(0);
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
	
	free(config.hostname);
	free(config.username);
	free(config.password);
}
