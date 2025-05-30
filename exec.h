void runWithOutFile(char* argv[]);
void performCmd(char buffer[], char cmd[], char prevBuffer[], char *argv[]);
void runBuildInCmd(char buffer[], char cmd[], char prevBuffer[], char *argv[]);
void jobCmd();
void backToForeground(int job_id);