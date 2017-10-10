#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

bool getSettings(int argc, char **argv,
                int& port_listen,
                char *&ip_listen,
                char *&path_root){

  opterr = 0;
  int c;
  while ((c = getopt (argc, argv, "h:p:d:")) != -1)
    switch (c){
      case 'h':
        ip_listen = optarg;
        break;
      case 'p':
        port_listen = atoi(optarg);
        break;
      case 'd':
        path_root = optarg;
        break;
      case '?':
        if (optopt == 'h' || optopt == 'p' || optopt == 'd'   )
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return false;
      default:
        printf ("help: -h <ip> -p <port> -d <path>\n");
        abort ();
    }


  printf ("ip = %s, port = %d, path_root = %s\n",
          ip_listen, port_listen, path_root);
  
  int index;
  bool result=true;
  for (index = optind; index < argc; index++){
      printf ("Non-option argument %s\n", argv[index]);
      result = false;
  }
      
  return result;
}
