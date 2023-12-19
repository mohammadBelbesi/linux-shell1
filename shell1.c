//mohammad belbesi

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <wait.h>


#define SENTENCE_LENGTH 511 //sentence length is 511 with \0 that's mean the sentence length is 510
#define MAX_ARGS 10
#define REGULAR 0
#define DOLLAR 1
#define EQUAL 2
#define EMPTY 3
#define CD 4
#define ECHO 5
#define  QUOTATION_MARKS 6
#define  SEMI_COLON  7


int sentenceType(char *s);
void stringParser(char* s, int* wordNum, int* charNum);
void command(char *s,char **commandL,int *argsNum);
void removeSpaces(char* str);
int countAndRemoveQuotes(char* str);
char* get_env_var(char* name);
void set_env_var(char* name, char* value);
void parseString(char* input, char** name, char** value);
void deleteFirstDollar(char* str);
int echoQuotationChecker(char *s);
void addSpacesToSemiColon(char* str);
int semiColonChecker(char *s);
int count_words_without_semicolon(const char* str);
int quotationFlag=0;
int num_env_vars = 0;

typedef struct {
    char name[SENTENCE_LENGTH];
    char value[SENTENCE_LENGTH];
} env_var_t;
env_var_t env_vars[SENTENCE_LENGTH];

int main() {
    char sentence[SENTENCE_LENGTH]={},cwd[SENTENCE_LENGTH]={},**commandLine={NULL},**cmd_args={NULL},*name={NULL},*value={NULL};
    memset(sentence,0,SENTENCE_LENGTH);//clear the char array and fill it with zero's
    memset(cwd,0,SENTENCE_LENGTH);//clear the char array and fill it with zero's
    /*
    memset(name,0,SENTENCE_LENGTH);//clear the char array and fill it with zero's
    memset(value,0,SENTENCE_LENGTH);//clear the char array and fill it with zero's
    memset(commandLine,0,SENTENCE_LENGTH);//clear the char array and fill it with zero's
    memset(cmd_args,0,SENTENCE_LENGTH);//clear the char array and fill it with zero's
    */
    int enterCounter =0,commandsNum=0,argsNum=0,wordNum = 0,charNum=0,status=0;
    getcwd(cwd,sizeof (cwd));
    while(1){//the shell loop
        if(enterCounter==3){
            break;
        }
        printf("#cmd:%d|args:%d@%s>",commandsNum,argsNum,cwd);//the prompt
        if(fgets(sentence,SENTENCE_LENGTH,stdin)!=NULL){//to scan the command that we enter
            sentence[strlen(sentence)-1]='\0';//solve the problem of \n and replace it with \0
            if(sentenceType(sentence)==EMPTY){
                enterCounter++;
            }
            else if(sentenceType(sentence)==CD){
                printf("cd not supported\n");
            }
            else if(sentenceType(sentence)==EQUAL){
                parseString(sentence,&name,&value);
                set_env_var(name,value);
            }
            else{//if it's a command
                if(echoQuotationChecker(sentence) == QUOTATION_MARKS){//check if the command contain echo and quotation and deal with it
                    countAndRemoveQuotes(sentence);
                    quotationFlag=1;
                    wordNum +=2;//to deal with num world in the case that we have quotation
                }
                if(sentenceType(sentence)==DOLLAR) {//check if the command contain dollar and deal with it
                    int dollarCounter=1;//for the loop that deleting the $'s
                    char *ptr = strchr(sentence, '$');

                    while (ptr != NULL) {//the main loop for dealing with $'s
                        char word[SENTENCE_LENGTH];//to save the key
                        //memset(word,0,SENTENCE_LENGTH);//clear the char array and fill it with zero's
                        ptr++;//push the pointer one bit forward
                        int i = 0;
                        while (*ptr != '\0' && *ptr != ' ' && *ptr != ';' &&i < SENTENCE_LENGTH) {//loop to copy the key in word and to know the length of the key by using i
                            word[i] = *ptr;
                            ptr++;
                            i++;
                        }
                        word[i] = '\0';
                        ptr -= i;//return the pointer to point on $ in the sentence

                        char *val = get_env_var(word);//get the value of the key word from the map

                        if (ptr != NULL) {//this if to replace the value instead of the key in the command sentence
                            int oldWordLength = i;
                            unsigned int newWordLength = strlen(val);

                            if (oldWordLength != newWordLength) {//moves the memory block of characters to accommodate the new length of the word.
                                memmove(ptr + newWordLength, ptr + oldWordLength, strlen(ptr + oldWordLength) + 1);
                            }

                            memcpy(ptr, val, newWordLength);//replace the $var to $value
                            removeSpaces(ptr);
                            ptr[strlen(sentence)-1]='\0';//add \0 to the end of the sentence
                            //printf("The word after \"$\" is \"%s\"\n", word);
                            //printf("The new sentence after is \"%s\"\n", sentence);

                            if (sentence[0] == '$') {
                                memmove(sentence, sentence+1, strlen(sentence+1)+1);
                            }

                            ptr = strchr(ptr, '$');
                            if(ptr!=NULL){//counter to delete the $'s
                                dollarCounter++;
                            }
                        }
                    }
                    while(dollarCounter!=0){//to delete the first dollar that the loop see in every iteration
                        deleteFirstDollar(sentence);
                        dollarCounter--;
                    }


                }
                if(quotationFlag == 0){
                    stringParser(sentence, &wordNum, &charNum); //our main func for count words and chars
                }
                if(semiColonChecker(sentence)==SEMI_COLON && sentenceType(sentence)==DOLLAR){
                    argsNum+=count_words_without_semicolon(sentence);
                }
                //wordNumInAllCommands+=wordNum;

                commandLine = (char**)malloc(sizeof(char*)*(MAX_ARGS+1));//the array of the pointers on the words array
                if(commandLine==NULL){
                    fprintf(stderr,"malloc failed!\n");
                    exit(EXIT_FAILURE);
                }
                //commandLine[wordNum]=NULL;
                if(semiColonChecker(sentence)==SEMI_COLON){
                    addSpacesToSemiColon(sentence);
                }
                int i=0;
                commandLine[i] = strtok(sentence, ";");//toke every command that occur between ; separately
                while (commandLine[i] != NULL) {//loop to split the command
                    i++;
                    commandLine[i] = strtok(NULL, ";");
                }

                for(int j=0;j<i;j++) {//main loop to run the commands
                    pid_t pid;

                    pid = fork();

                    if (pid < 0) {//fork failed
                        perror("fork failed!\n");
                        exit(EXIT_FAILURE);
                    }

                    if (pid == 0) {//child
                        //command(sentence, commandLine);
                        cmd_args = (char**) malloc(MAX_ARGS * sizeof(char*));
                        command(commandLine[j], cmd_args,&argsNum);


                    }

                    if (pid > 0) {//parent
                        //wait(NULL);
                        waitpid(pid, &status, 0);
                        if (WIFEXITED(status)) {//to check if the command is legal then modify the command and the args number
                            commandsNum++;
                            argsNum+=wordNum;
                        }
                        wordNum = 0, charNum = 0;//initialize the variable for the next iterations

                    }

                }

            }

        }

    }

    return 0;
}

void deleteFirstDollar(char* str) {
    char* pos = strchr(str, '$'); // Find the first occurrence of '$' in the string
    if (pos != NULL) {
        memmove(pos, pos + 1, strlen(pos)); // Shift the rest of the string left to overwrite '$'
    }
}

char* get_env_var(char* name) {
    for (int i = 0; i < num_env_vars; i++) {
        if (strcmp(name, env_vars[i].name) == 0) {
            return env_vars[i].value;
        }
    }
    return NULL;
}

void set_env_var(char* name, char* value) {
    for (int i = 0; i < num_env_vars; i++) {
        if (strcmp(name, env_vars[i].name) == 0) {
            strcpy(env_vars[i].value, value);
            return;
        }
    }
    strcpy(env_vars[num_env_vars].name, name);
    strcpy(env_vars[num_env_vars].value, value);
    num_env_vars++;
}

void parseString(char* input, char** name, char** value) {
    // Find the position of the '=' character in the input string
    char* equalsPos = strchr(input, '=');
    if (!equalsPos) {
        // '=' not found in input string
        *name = NULL;
        *value = NULL;
        return;
    }

    // Allocate memory for the name and value strings
    *name = (char*) malloc(sizeof(char) * (equalsPos - input + 1));
    size_t len = strlen(equalsPos+1);
    *value = (char*) malloc(sizeof(char) * (int)(len + 1));
    //*value = (char*) malloc(sizeof(char) * (strlen(equalsPos+1) + 1));
    //*value = (char*) malloc(sizeof(char) * (strlen(equalsPos) + 1 + 1));


    // Copy the name and value strings into their respective memory locations
    strncpy(*name, input, equalsPos - input);
    strncpy(*value, equalsPos + 1, strlen(equalsPos + 1));

    // Null-terminate the strings
    (*name)[equalsPos - input] = '\0';
    (*value)[strlen(equalsPos + 1)] = '\0';
}

int sentenceType(char *s){ //the func will take the sentence str and return if we had entered command, variable, cd or just press enter
    long unsigned int length = strlen(s)+1;
    char copySentence[length]; //this string to copy the original string without spaces to solve the spaces or enter case
    memset(copySentence, 0, length);
    strcpy(copySentence,s); //copy func in c
    copySentence[length]='\0';
    int i,j;
    for(i=0,j=0; j<length; j++){//copy the sentence without spaces "its good for checking if we enter only spaces, or if we press enter in the command line"
        if(s[j]!=' '){
            copySentence[i]=s[j];
            i++;
        }
    }
    copySentence[i]='\0';
    char *equal = strchr(s, '=');
    char *dollar = strchr(s, '$');
    char *cd = strstr(s, "cd");

    //for the sentence type flags
    if(dollar){
        return DOLLAR;
    }

    else if(equal){
        return EQUAL;
    }

    else if(copySentence[0]=='\0'){
        return EMPTY;
    }

    else if(cd){
        return CD;
    }

    else{
        return REGULAR;
    }

}

int echoQuotationChecker(char *s){
    char *quotationMarks = strchr(s, '"');
    char *echo = strstr(s, "echo");
    if(echo){
        if(quotationMarks){
            return QUOTATION_MARKS;
        }
        return ECHO;
    }
    return 0;

}

int semiColonChecker(char *s){
    char *semiColon = strchr(s, ';');
    if(semiColon){
        return SEMI_COLON;
    }
    return 0;

}

void stringParser(char* s, int* wordNum, int* charNum){ // return how many words and chars in the sentence that we entered, and it takes the sentence str and two pointers that points on word and char number
    long unsigned int len = strlen(s)+1; // the sentence length that we entered

    for(int i=0; i<len;){ // loop to count the words and the chars by o(n), only one time we check the char
        if(s[i]==' '||s[i]==';'){
            i++;
        }
        else{
            (*wordNum)++;
            (*charNum)++;
            int j;
            for(j=i+1; j<len; j++){
                if(s[j]== ' '){
                    i=j;
                    break;
                }
                else{
                    (*charNum)++;
                }
            }
            if(s[j]=='\0') {
                i = j;
            }
        }
    }

}

void command(char *s, char **commandL,int *argsNum) {
    unsigned int length = strlen(s) + 1;
    char copyS[length];
    strcpy(copyS, s); // we copy the array because the strtok is not safe, and it makes changes on the original one
    copyS[length] = '\0';

    char *word = strtok(copyS, " ");
    int i = 0;
    while (word != NULL) {
        unsigned int wordLength = strlen(word) + 1;
        commandL[i] = (char*) malloc(sizeof(char) * (wordLength + 1)); // take every word of the sentence and enter it in the 2d commandL string
        if (commandL[i] == NULL) {
            fprintf(stderr, "malloc failed!\n");
            exit(EXIT_FAILURE); // exit the code
        }
        strcpy(commandL[i], word);
        i++;
        word = strtok(NULL, " ");
    }
    (*argsNum)+=i;
    free(word);
    if (execvp(commandL[0], commandL) == -1) {
        perror("the execvp() failed, the generalCommand not found! or there are to many commands!");
        sleep(1);
    }
    exit(EXIT_FAILURE);
}

void removeSpaces(char* str) {
    unsigned int len = strlen(str);

    // Remove leading spaces
    while (isspace(str[0])) {
        for (int i = 0; i < len; i++) {
            str[i] = str[i+1];
        }
        len--;
    }

    // Remove trailing spaces
    while (isspace(str[len-1])) {
        str[len-1] = '\0';
        len--;
    }

    // Replace multiple spaces with a single space
    int i,j;
    for (i = 0, j = 0; i < len; i++) {
        if (!isspace(str[i]) || (i > 0 && !isspace(str[i-1]))) {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

int countAndRemoveQuotes(char* str) {
    int count = 0;
    unsigned int len = strlen(str);

    for (int i = 0; i < len; i++) {
        if (str[i] == '\"') {
            count++;
        } else {
            str[i - count] = str[i];
        }
    }

    str[len - count] = '\0';

    return count;
}

void addSpacesToSemiColon(char* str) {
    unsigned int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (str[i] == ';') {
            if (i > 0 && str[i-1] != ' ') {
                memmove(str+i+1, str+i, len-i+1);
                str[i] = ' ';
                len++;
            }
            if (i < len-1 && str[i+1] != ' ') {
                memmove(str+i+2, str+i+1, len-i);
                str[i+1] = ' ';
                len++;
                i++; // Skip over newly added space
            }
        }
    }
    str[len] = '\0'; // add \0

}

int count_words_without_semicolon(const char* str) {
    char* str_copy = strdup(str); // create a copy of the original string
    int count = 0;
    char* token = strtok(str_copy, " ;"); // strtok function splits string into tokens using delimiters

    while (token != NULL) {
        count++;
        token = strtok(NULL, " ;");
    }

    free(str_copy); // free the memory allocated for the copy of the string
    return count;
}
